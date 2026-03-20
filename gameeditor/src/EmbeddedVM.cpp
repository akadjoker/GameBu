// EmbeddedVM.cpp — In-process BuLang VM for the game editor.
// When running, the game takes over the full window. Escape returns to editor.

#include "EmbeddedVM.h"

#include "interpreter.hpp"
#include "engine.hpp"
#include "filebuffer.hpp"
#include "bindings.hpp"
#include "camera.hpp"
#include "sound.hpp"
#include "renderer.hpp"

#include <raylib.h>
#include <cstring>
#include <cstdio>
#include <exception>

// --- Globals provided by the runtime library (camera.cpp, emitter.cpp, engine.cpp, renderer.cpp) ---
extern Scene gScene;
extern ParticleSystem gParticleSystem;
extern CameraManager gCamera;
extern Renderer gRenderer;
extern Color BACKGROUND_COLOR;

// ---------------------------------------------------------------------------
// Script-controlled window config (mirrored from main/src/main.cpp)
// These are set by native functions during vm.run() (top-level script code)
// and applied afterwards.
// ---------------------------------------------------------------------------
static int    s_requestedWidth  = 0;
static int    s_requestedHeight = 0;
static std::string s_requestedTitle;
static bool   s_wantClose       = false;

static int native_set_window_size(Interpreter* /*vm*/, int argCount, Value* args)
{
    if (argCount >= 2 && args[0].isNumber() && args[1].isNumber())
    {
        s_requestedWidth  = static_cast<int>(args[0].asNumber());
        s_requestedHeight = static_cast<int>(args[1].asNumber());
        SetWindowSize(s_requestedWidth, s_requestedHeight);
    }
    return 0;
}

static int native_set_window_title(Interpreter* /*vm*/, int argCount, Value* args)
{
    if (argCount >= 1 && args[0].isString())
    {
        s_requestedTitle = args[0].asStringChars();
        SetWindowTitle(s_requestedTitle.c_str());
    }
    return 0;
}

static int native_set_fullscreen(Interpreter* /*vm*/, int argCount, Value* args)
{
    // Ignored in editor mode — we don't want fullscreen to take over
    (void)argCount; (void)args;
    return 0;
}

static int native_set_window_resizable(Interpreter* /*vm*/, int argCount, Value* args)
{
    // Ignored in editor mode — we always keep the window resizable
    (void)argCount; (void)args;
    return 0;
}

static int native_close_window(Interpreter* /*vm*/, int /*argCount*/, Value* /*args*/)
{
    s_wantClose = true;
    return 0;
}

static int native_set_log_level(Interpreter* /*vm*/, int argCount, Value* args)
{
    if (argCount >= 1 && args[0].isNumber())
    {
        SetTraceLogLevel(static_cast<int>(args[0].asNumber()));
    }
    return 0;
}

// ---------------------------------------------------------------------------
// File loader (simplified version for the embedded VM)
// ---------------------------------------------------------------------------
struct EmbeddedFileLoaderCtx
{
    const char* searchPaths[8];
    int pathCount;
    char fullPath[512];
    FileBuffer fileBuffer;
};

static const char* embeddedFileLoader(const char* filename, size_t* outSize, void* userdata)
{
    if (!filename || !outSize || !userdata) return nullptr;
    auto* ctx = static_cast<EmbeddedFileLoaderCtx*>(userdata);
    *outSize = 0;

    auto tryLoad = [&](const char* path) -> const char* {
        if (!path || !*path) return nullptr;
        if (ctx->fileBuffer.load(path))
        {
            *outSize = ctx->fileBuffer.size();
            return ctx->fileBuffer.c_str();
        }
        return nullptr;
    };

    // Skip leading slashes for relative resolution
    const char* relName = filename;
    while (*relName == '/' || *relName == '\\') relName++;

    // Absolute path
    if (filename[0] == '/')
    {
        return tryLoad(filename);
    }

    // Search configured paths
    const char* name = (relName != filename) ? relName : filename;
    for (int i = 0; i < ctx->pathCount; i++)
    {
        std::snprintf(ctx->fullPath, sizeof(ctx->fullPath), "%s/%s", ctx->searchPaths[i], name);
        const char* r = tryLoad(ctx->fullPath);
        if (r) return r;
    }

    // Last resort: direct
    const char* r = tryLoad(filename);
    if (r) return r;
    if (relName != filename) return tryLoad(relName);
    return nullptr;
}

// ---------------------------------------------------------------------------
// VMHooks callbacks — link Process ↔ Entity (identical to main/src/main.cpp)
// ---------------------------------------------------------------------------
static void hookOnCreate(Interpreter* /*vm*/, Process* proc)
{
    Entity* entity = gScene.addEntity(-1, 0, 0, 0);
    proc->userData = entity;
    entity->userData = proc;
    entity->procID = proc->id;
    entity->blueprint = proc->blueprint;
    entity->ready = false;
    entity->layer = 0;
    entity->z = 0;
    entity->flags = B_VISIBLE | B_COLLISION;
}

static void hookOnStart(Interpreter* /*vm*/, Process* proc)
{
    Entity* entity = static_cast<Entity*>(proc->userData);
    if (!entity) return;

    double x = proc->privates[0].asNumber();
    double y = proc->privates[1].asNumber();
    int z = (int)proc->privates[2].asNumber();
    int graph = proc->privates[3].asInt();
    int angle = proc->privates[4].asInt();

    auto readColor = [&](int idx) -> double {
        if (proc->privates[idx].isInt()) return proc->privates[idx].asInt() / 255.0;
        if (proc->privates[idx].isNumber()) return proc->privates[idx].asNumber();
        return 1.0;
    };

    entity->graph = graph;
    entity->z = z;
    entity->procID = proc->id;
    entity->setPosition(x, y);
    entity->setAngle(angle);
    entity->size_x = proc->privates[(int)PrivateIndex::SIZEX].asNumber();
    entity->size_y = proc->privates[(int)PrivateIndex::SIZEY].asNumber();
    entity->color.r = (uint8)(readColor(9) * 255.0);
    entity->color.g = (uint8)(readColor(10) * 255.0);
    entity->color.b = (uint8)(readColor(11) * 255.0);
    entity->color.a = (uint8)(readColor(12) * 255.0);
    entity->flags = B_VISIBLE | B_COLLISION;
    entity->ready = true;
}

static void hookOnUpdate(Interpreter* /*vm*/, Process* proc, float /*dt*/)
{
    Entity* entity = static_cast<Entity*>(proc->userData);
    if (!entity || !entity->ready) return;

    double x = proc->privates[0].asNumber();
    double y = proc->privates[1].asNumber();
    int z = proc->privates[2].asInt();
    int graph = proc->privates[3].asInt();
    int angle = proc->privates[4].asInt();

    auto readColor = [&](int idx) -> double {
        if (proc->privates[idx].isInt()) return proc->privates[idx].asInt() / 255.0;
        if (proc->privates[idx].isNumber()) return proc->privates[idx].asNumber();
        return 1.0;
    };

    entity->graph = graph;
    entity->z = z;
    entity->setPosition(x, y);
    entity->setAngle(angle);
    entity->size_x = proc->privates[(int)PrivateIndex::SIZEX].asNumber();
    entity->size_y = proc->privates[(int)PrivateIndex::SIZEY].asNumber();
    entity->color.r = (uint8)(readColor(9) * 255.0);
    entity->color.g = (uint8)(readColor(10) * 255.0);
    entity->color.b = (uint8)(readColor(11) * 255.0);
    entity->color.a = (uint8)(readColor(12) * 255.0);
}

static void hookOnDestroy(Interpreter* /*vm*/, Process* proc, int /*exitCode*/)
{
    Entity* entity = static_cast<Entity*>(proc->userData);
    if (entity)
    {
        gScene.removeEntity(entity);
        proc->userData = nullptr;
    }
}

static void hookOnRender(Interpreter* /*vm*/, Process* /*proc*/) {}

// ---------------------------------------------------------------------------
// EmbeddedVM implementation
// ---------------------------------------------------------------------------

EmbeddedVM::EmbeddedVM() = default;

EmbeddedVM::~EmbeddedVM()
{
    stop();
}

void EmbeddedVM::initVM(const std::string& projectDir)
{
    m_vm = new Interpreter();

    VMHooks hooks{};
    hooks.onCreate  = hookOnCreate;
    hooks.onStart   = hookOnStart;
    hooks.onUpdate  = hookOnUpdate;
    hooks.onDestroy = hookOnDestroy;
    hooks.onRender  = hookOnRender;

    m_vm->registerAll();
    m_vm->setHooks(hooks);

    // Register all game bindings
    Bindings::registerAll(*m_vm);
    BindingsEase::registerAll(*m_vm);
    registerCameraNatives(*m_vm);

    // Register window-control natives (same API as standalone runner)
    m_vm->registerNative("set_window_size",      native_set_window_size, 2);
    m_vm->registerNative("set_window_title",     native_set_window_title, 1);
    m_vm->registerNative("set_fullscreen",       native_set_fullscreen, 1);
    m_vm->registerNative("set_window_resizable", native_set_window_resizable, 1);
    m_vm->registerNative("close_window",         native_close_window, 0);
    m_vm->registerNative("set_log_level",        native_set_log_level, 1);

    // Set up the file loader with include search paths
    static EmbeddedFileLoaderCtx loaderCtx{};
    loaderCtx.pathCount = 0;

    // Build search paths based on projectDir
    static std::string scriptsDir;
    static std::string projectDirCopy;
    projectDirCopy = projectDir;
    scriptsDir = projectDir + "/scripts";

    auto addPath = [&](const char* p) {
        if (loaderCtx.pathCount < 8) loaderCtx.searchPaths[loaderCtx.pathCount++] = p;
    };
    addPath(scriptsDir.c_str());
    addPath(projectDirCopy.c_str());

    m_vm->setFileLoader(embeddedFileLoader, &loaderCtx);
}

void EmbeddedVM::shutdownVM()
{
    if (!m_vm) return;

    m_vm->killAliveProcess();
    BindingsMessage::clearAllMessages();
    gParticleSystem.clear();
    BindingsSound::shutdown();
    BindingsBox2D::shutdownPhysics();
    BindingsDraw::unloadFonts();

    if (m_soundInited)
    {
        DestroySound();
        m_soundInited = false;
    }
    if (m_sceneInited)
    {
        DestroyScene();
        m_sceneInited = false;
    }

    gRenderer.shutdown();

    delete m_vm;
    m_vm = nullptr;
}

void EmbeddedVM::saveWindowState()
{
    m_savedWidth  = GetScreenWidth();
    m_savedHeight = GetScreenHeight();
    // GetWindowTitle() is not available in all raylib versions,
    // so we just store a known editor title.
    m_savedTitle = "BuEditor";
}

void EmbeddedVM::restoreWindowState()
{
    if (m_savedWidth > 0 && m_savedHeight > 0)
    {
        SetWindowSize(m_savedWidth, m_savedHeight);
    }
    if (!m_savedTitle.empty())
    {
        SetWindowTitle(m_savedTitle.c_str());
    }
}

bool EmbeddedVM::start(const std::string& sourceCode, const std::string& projectDir)
{
    // Stop any previous session
    stop();
    m_error.clear();

    // Save the current editor window state so we can restore it later
    saveWindowState();

    // Reset script-requested window config
    s_requestedWidth  = 0;
    s_requestedHeight = 0;
    s_requestedTitle.clear();
    s_wantClose = false;

    // Initialize the scene and sound subsystems
    InitScene();
    m_sceneInited = true;
    InitSound();
    m_soundInited = true;

    // Initialize camera to current window size
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    gCamera.init(w, h);
    gCamera.setScreenScaleMode(SCALE_NONE);
    gCamera.setVirtualScreenEnabled(false);

    // Initialize renderer
    gRenderer.init();

    // Reset background color
    BACKGROUND_COLOR = BLACK;

    // Create and configure the VM
    initVM(projectDir);

    // Compile & run the script
    bool ok = false;
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    try
    {
        ok = m_vm->run(sourceCode.c_str(), false);
    }
    catch (const std::exception& e)
    {
        m_error = std::string("Script exception: ") + e.what();
    }
    catch (...)
    {
        m_error = "Unknown C++ exception while loading script.";
    }
#else
    ok = m_vm->run(sourceCode.c_str(), false);
#endif

    if (!ok && m_error.empty())
    {
        m_error = "Failed to execute script.";
    }

    if (!ok)
    {
        shutdownVM();
        restoreWindowState();
        m_state = EmbeddedVMState::Error;
        return false;
    }

    m_state = EmbeddedVMState::Running;
    return true;
}

bool EmbeddedVM::frame()
{
    if (m_state != EmbeddedVMState::Running || !m_vm) return false;

    // Check if all processes have terminated or script called close_window()
    if (m_vm->getTotalAliveProcesses() <= 0 || s_wantClose)
    {
        stop();
        return false;
    }

    float dt = GetFrameTime();

    // Update game systems
    BindingsInput::update();
    gCamera.update(dt);
    BindingsSound::updateMusicStreams();
    UpdateFade(dt);
    gParticleSystem.update(dt);
    gScene.updateCollision();

    // Render directly to the window
    BeginDrawing();
    ClearBackground(BACKGROUND_COLOR);

    gCamera.begin();
    BindingsDraw::resetDrawCommands();
    m_vm->update(dt);
    RenderScene();
    gParticleSystem.cleanup();
    gParticleSystem.draw();
    BindingsBox2D::renderDebug();
    gCamera.end();

    BindingsDraw::RenderScreenCommands();
    BindingsInput::drawVirtualKeys();
    DrawFade();

    // Draw a small hint overlay
    DrawText("F5 = Back to Editor", 10, 10, 16, Fade(WHITE, 0.5f));

    EndDrawing();

    return true;
}

void EmbeddedVM::stop()
{
    if (m_state == EmbeddedVMState::Idle) return;

    shutdownVM();

    // Restore the editor window size and title
    restoreWindowState();

    m_state = EmbeddedVMState::Idle;
    m_error.clear();
}
