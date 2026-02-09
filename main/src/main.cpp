

#include "engine.hpp"
#include "interpreter.hpp"
#include "bindings.hpp"
#include "camera.hpp"
#include "platform.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <raylib.h>

extern Scene gScene;
extern ParticleSystem gParticleSystem;
extern CameraManager gCamera;


struct FileLoaderContext
{
    const char *searchPaths[8];
    int pathCount;
    char fullPath[512];
    char buffer[1024 * 1024];
};

const char *multiPathFileLoader(const char *filename, size_t *outSize, void *userdata)
{
    FileLoaderContext *ctx = (FileLoaderContext *)userdata;

    for (int i = 0; i < ctx->pathCount; i++)
    {
        snprintf(ctx->fullPath, sizeof(ctx->fullPath), "%s/%s", ctx->searchPaths[i], filename);

        FILE *f = fopen(ctx->fullPath, "rb");
        if (!f)
            continue;

        fseek(f, 0, SEEK_END);
        long size = ftell(f);

        if (size <= 0)
        {
            fclose(f);
            continue;
        }

        if (size >= (long)sizeof(ctx->buffer))
        {
            fprintf(stderr, "File too large: %s (%ld bytes)\n", ctx->fullPath, size);
            fclose(f);
            *outSize = 0;
            return nullptr;
        }

        fseek(f, 0, SEEK_SET);
        size_t bytesRead = fread(ctx->buffer, 1, size, f);
        fclose(f);

        if (bytesRead != (size_t)size)
            continue;

        ctx->buffer[bytesRead] = '\0';
        *outSize = bytesRead;
        return ctx->buffer;
    }

    *outSize = 0;
    return nullptr;
}

// Helper: load file contents into a string
static std::string loadFile(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        fprintf(stderr, "Could not open file: %s\n", path);
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

// ============================================================
// Global Configuration Variables (set by script)
// ============================================================

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;
std::string WINDOW_TITLE = "BuGameEngine";

bool FULLSCREEN = false;
bool CAN_RESIZE = true;
bool CAN_CLOSE = false;
Color BACKGROUND_COLOR = BLACK;

// ============================================================
// Native Functions for Script Configuration
// ============================================================

int native_set_window_size(Interpreter *vm, int argCount, Value *args)
{
    if (argCount != 2)
    {
        Error("set_window_size expects 2 integer arguments (width, height)");
        return 0;
    }
    if (!args[0].isNumber() || !args[1].isNumber())
    {
        Error("set_window_size expects integer arguments (width, height)");
        return 0;
    }

    WINDOW_WIDTH = int(args[0].asNumber());
    WINDOW_HEIGHT = int(args[1].asNumber());

    return 0;
}

int native_set_window_title(Interpreter *vm, int argCount, Value *args)
{
    if (argCount != 1)
    {
        Error("set_window_title expects 1 string argument (title)");
        return 0;
    }
    if (!args[0].isString())
    {
        Error("set_window_title expects a string argument (title)");
        return 0;
    }

    WINDOW_TITLE = args[0].asStringChars();

    return 0;
}

int native_set_fullscreen(Interpreter *vm, int argCount, Value *args)
{
    if (argCount != 1)
    {
        Error("set_fullscreen expects 1 boolean argument");
        return 0;
    }

    FULLSCREEN = args[0].asBool();

    return 0;
}

int native_close_window(Interpreter *vm, int argCount, Value *args)
{
    (void)vm;
    (void)argCount;
    (void)args;
    CAN_CLOSE = true;
    return 0;
}

int native_set_window_resizable(Interpreter *vm, int argCount, Value *args)
{
    if (argCount != 1)
    {
        Error("set_window_resizable expects 1 boolean argument");
        return 0;
    }

    CAN_RESIZE = args[0].asBool();

    return 0;
}

int native_set_log_level(Interpreter *vm, int argCount, Value *args)
{
    if (argCount != 1)
    {
        Error("set_log_level expects 1 integer argument");
        return 0;
    }
    if (!args[0].isNumber())
    {
        Error("set_log_level expects an integer argument");
        return 0;
    }

    int level = int(args[0].asNumber());
    SetTraceLogLevel(level);

    return 0;
}


void FreeResources()
{
}

void onCreate(Interpreter *vm, Process *proc)
{
    Entity *entity = gScene.addEntity(-1, 0, 0, 0);
    proc->userData = entity;
    entity->userData = proc;
    entity->procID = proc->id;
    entity->blueprint = proc->blueprint;
    entity->ready = false;
    entity->layer = 0;
    entity->flags = B_VISIBLE | B_COLLISION;
}

void onStart(Interpreter *vm, Process *proc)
{

    double x = proc->privates[0].asNumber();
    double y = proc->privates[1].asNumber();
    int z = (int)proc->privates[2].asNumber();
    int graph = proc->privates[3].asInt();
    int angle = proc->privates[4].asInt();
    int size = proc->privates[5].asInt();
    int flags = proc->privates[6].asInt();
    int id = proc->privates[7].asInt();
    int father = proc->privates[8].asInt();
    double red = 1.0;
    if (proc->privates[(int)PrivateIndex::iGREEN].isInt())
        red = proc->privates[9].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iGREEN].isNumber())
        red = proc->privates[9].asNumber();

    double green = 1.0;
    if (proc->privates[(int)PrivateIndex::iGREEN].isInt())
        green = proc->privates[10].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iGREEN].isNumber())
        green = proc->privates[10].asNumber();
    double blue = 1.0;
    if (proc->privates[(int)PrivateIndex::iBLUE].isInt())
        blue = proc->privates[11].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iBLUE].isNumber())
        blue = proc->privates[11].asNumber();
    double alpha = 1.0;
    if (proc->privates[(int)PrivateIndex::iALPHA].isInt())
        alpha = proc->privates[12].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iALPHA].isNumber())
        alpha = proc->privates[12].asNumber();

    // Info("Create process: ID:%d  Layer:%d  angle:%d  Size:%d   FLAGS: %d X:%f Y:%f  FATHER:%d  GRAPH:%d", id, z, angle, size,  flags, x, y, father, graph);

    Entity *entity = (Entity *)proc->userData;
    if (!entity)
    {
        // Warning("Process %d has no associated entity!", proc->id);
        return;
    }
    int safeLayer = z;
    if (safeLayer < 0 || safeLayer >= MAX_LAYERS)
        safeLayer = 0;

    if (entity->layer != safeLayer)
    {

        gScene.moveEntityToLayer(entity, safeLayer);
    }

    entity->graph = graph;
    entity->procID = proc->id;
    entity->setPosition(x, y);
    entity->setAngle(angle);
    entity->setSize(size);
    entity->color.r = (uint8)(red * 255.0);
    entity->color.g = (uint8)(green * 255.0);
    entity->color.b = (uint8)(blue * 255.0);
    entity->color.a = (uint8)(alpha * 255.0);
    entity->flags = B_VISIBLE | B_COLLISION;

    entity->ready = true;
}
void onUpdate(Interpreter *vm, Process *proc, float dt)
{

    Entity *entity = (Entity *)proc->userData;
    if (!entity)
    {
        // Warning("Process %d has no associated entity!", proc->id);
        return;
    }
    if (!entity->ready)
        return;

    double x = proc->privates[0].asNumber();
    double y = proc->privates[1].asNumber();
    int z = proc->privates[2].asInt();
    int graph = proc->privates[3].asInt();
    int angle = proc->privates[4].asInt();
    int size = proc->privates[5].asInt();
    int flags = proc->privates[6].asInt();
    int id = proc->privates[7].asInt();
    int father = proc->privates[8].asInt();
    double red = 1.0;
    if (proc->privates[(int)PrivateIndex::iGREEN].isInt())
        red = proc->privates[9].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iGREEN].isNumber())
        red = proc->privates[9].asNumber();

    double green = 1.0;
    if (proc->privates[(int)PrivateIndex::iGREEN].isInt())
        green = proc->privates[10].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iGREEN].isNumber())
        green = proc->privates[10].asNumber();
    double blue = 1.0;
    if (proc->privates[(int)PrivateIndex::iBLUE].isInt())
        blue = proc->privates[11].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iBLUE].isNumber())
        blue = proc->privates[11].asNumber();
    double alpha = 1.0;
    if (proc->privates[(int)PrivateIndex::iALPHA].isInt())
        alpha = proc->privates[12].asInt() / 255.0;
    else if (proc->privates[(int)PrivateIndex::iALPHA].isNumber())
        alpha = proc->privates[12].asNumber();

    int safeLayer = z;
    if (safeLayer < 0 || safeLayer >= MAX_LAYERS)
        safeLayer = 0;

    if (entity->layer != safeLayer)
    {
        gScene.moveEntityToLayer(entity, safeLayer);
    }
    entity->graph = graph;
    entity->setPosition(x, y);
    entity->setAngle(angle);
    entity->setSize(size);
    entity->color.r = (uint8)(red * 255.0);
    entity->color.g = (uint8)(green * 255.0);
    entity->color.b = (uint8)(blue * 255.0);
    entity->color.a = (uint8)(alpha * 255.0);

    // proc->privates[0] = vm->makeDouble(entity->x);
    // proc->privates[1] = vm->makeDouble(entity->y);
    // proc->privates[4] = vm->makeInt(entity->angle);
    // proc->privates[5] = vm->makeInt(entity->size);
    // proc->privates[6] = vm->makeInt(flags);
}
void onDestroy(Interpreter *vm, Process *proc, int exitCode)
{
    //   Info("Destroy process: %d with exit code %d", proc->id, exitCode);
    Entity *entity = (Entity *)proc->userData;
    if (entity && proc->userData)
    {
        gScene.removeEntity(entity);
        proc->userData = nullptr;
    }
}
void onRender(Interpreter *vm, Process *proc)
{
}

int main(int argc, char *argv[])
{
    Interpreter vm;

    VMHooks hooks;
    hooks.onStart = onStart;
    hooks.onUpdate = onUpdate;
    hooks.onDestroy = onDestroy;
    hooks.onRender = onRender;
    hooks.onCreate = onCreate;

    vm.registerAll();
    vm.setHooks(hooks);

    Bindings::registerAll(vm);
    BindingsEase::registerAll(vm);
    registerCameraNatives(vm);
    vm.registerNative("set_window_size", native_set_window_size, 2);
    vm.registerNative("set_window_title", native_set_window_title, 1);
    vm.registerNative("set_fullscreen", native_set_fullscreen, 1);
    vm.registerNative("set_window_resizable", native_set_window_resizable, 1);
    vm.registerNative("close_window", native_close_window, 0);
    vm.registerNative("set_log_level", native_set_log_level, 1);

    FileLoaderContext ctx;
    ctx.searchPaths[0] = "./bin";
    ctx.searchPaths[1] = "./scripts";
    ctx.searchPaths[2] = "../scripts";
    ctx.searchPaths[3] = ".";
    ctx.pathCount = 4;
    vm.setFileLoader(multiPathFileLoader, &ctx);

    const char *scriptFile = nullptr;

    if (argc > 1)
    {
        if (OsFileExists(argv[1]))
        {
            scriptFile = argv[1];
        }
        else
        {
            fprintf(stderr, "Specified script file does not exist: %s\n", argv[1]);

            return 1;
        }
    }

    if (!scriptFile)
    {
        if (OsFileExists("scripts/main.bu"))
        {
            scriptFile = "scripts/main.bu";
        }
        else if (OsFileExists("main.bu"))
        {
            scriptFile = "main.bu";
        }
        else if (OsFileExists("../scripts/main.bu"))
        {
            scriptFile = "../scripts/main.bu";
        }
        else
        {
            fprintf(stderr, "No script file specified and no default found.\n");

            return 1;
        }
    }

    std::ifstream file(scriptFile);
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    SetTraceLogLevel(LOG_NONE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE.c_str());
    SetExitKey(KEY_NULL); // Disable default ESC exit from Raylib.
    InitSound();
    InitScene();
    gCamera.init(WINDOW_WIDTH, WINDOW_HEIGHT);
    gCamera.setScreenScaleMode(SCALE_NONE);
    gCamera.setVirtualScreenEnabled(false); 

    if (!vm.run(code.c_str(), false))
    {
        Error("Failed to execute script: %s", scriptFile);
        CloseWindow();
        return 1;
    }

    uint32 flags = 0;
    if (FULLSCREEN)
        flags |= FLAG_FULLSCREEN_MODE;
    if (CAN_RESIZE)
        flags |= FLAG_WINDOW_RESIZABLE;

    SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetWindowTitle(WINDOW_TITLE.c_str());
    SetWindowState(flags);
    SetTargetFPS(60);


    // gCamera.setZoom(1.0f);
    
    
    // gCamera.setVirtualScreenEnabled(true);
    // gCamera.setScreenScaleMode(SCALE_FIT);   

 

     //gCamera.setDesignResolution(30 * 24, 20 * 24); // Configurar resolução de design para 720x480 (30x20 tiles de 24px)
    // gCamera.setScreenScaleMode(SCALE_STRETCH);  // Usar modo FIT para manter aspecto ratio e mostrar barras pretas se necessário
    // gCamera.setVirtualScreenEnabled(true); // Ativar virtual screen para usar a resolução de design


    while (!CAN_CLOSE && vm.getTotalAliveProcesses() > 0)
    {
        if (WindowShouldClose())
        {
            CAN_CLOSE = true;
        }

        if ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_X))
        {
            CAN_CLOSE = true;
        }
 
        //   // Mudar modo de escala
        // if (IsKeyPressed(KEY_F1)) gCamera.setScreenScaleMode(SCALE_NONE);
        // if (IsKeyPressed(KEY_F2)) gCamera.setScreenScaleMode(SCALE_FIT);
        // if (IsKeyPressed(KEY_F3)) gCamera.setScreenScaleMode(SCALE_STRETCH);
        // if (IsKeyPressed(KEY_F4)) gCamera.setScreenScaleMode(SCALE_FILL);


       
        
        float dt = GetFrameTime();
        gCamera.update(dt);
        UpdateFade(dt);
        gScene.updateCollision();

         

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        gCamera.begin();
        gParticleSystem.update(dt);
        BindingsDraw::resetDrawCommands();
        vm.update(dt);
        RenderScene();
        gParticleSystem.cleanup();
        gParticleSystem.draw();
        gCamera.end();

        BindingsDraw::RenderScreenCommands();


        DrawFade();

       // DrawText(TextFormat("FPS: %d Processes: %d", GetFPS(), vm.getTotalAliveProcesses()), 10, 10, 20, WHITE);
        EndDrawing();
    }
    BindingsMessage::clearAllMessages();
    gParticleSystem.clear();
    BindingsDraw::unloadFonts();
    DestroySound();
    DestroyScene();

    CloseWindow();
    return 0;
}
