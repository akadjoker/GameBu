#include "renderer.hpp"
#include "engine.hpp"
#include "bindings.hpp"
#include "camera.hpp"
#include "interpreter.hpp"

extern Scene gScene;
extern ParticleSystem gParticleSystem;
extern CameraManager gCamera;
extern Color BACKGROUND_COLOR;

Renderer::Renderer() : m_postProcessingEnabled(false) {}

Renderer::~Renderer() {}

void Renderer::init(int width, int height)
{
    m_sceneTexture = LoadRenderTexture(width, height);
    m_pingPongTexture = LoadRenderTexture(width, height);
}

void Renderer::shutdown()
{
    UnloadRenderTexture(m_sceneTexture);
    UnloadRenderTexture(m_pingPongTexture);
}

void Renderer::onWindowResize()
{
    shutdown();
    init(GetScreenWidth(), GetScreenHeight());
}

void Renderer::enablePostProcessing(bool enabled)
{
    m_postProcessingEnabled = enabled;
}

bool Renderer::isPostProcessingEnabled() const
{
    return m_postProcessingEnabled;
}

void Renderer::addPostProcessingPass(const RenderPass& pass)
{
    m_passes.push_back(pass);
}

void Renderer::clearPostProcessingPasses()
{
    m_passes.clear();
}

void Renderer::drawSceneContent(Interpreter* vm, float dt)
{
    gCamera.begin();
    BindingsDraw::resetDrawCommands();
    vm->update(dt);
    RenderScene();
    gParticleSystem.cleanup();
    gParticleSystem.draw();
    BindingsBox2D::renderDebug();
    gCamera.end();
}

void Renderer::drawPostProcessingPath()
{
    // Render the scene texture to the screen, applying the chain of post-processing shaders
    BeginDrawing();
    ClearBackground(BLACK); // For letterboxing

    if (m_passes.empty())
    {
        // No passes, just draw the scene texture directly to the screen.
        Rectangle sourceRec = { 0, 0, (float)m_sceneTexture.texture.width, -(float)m_sceneTexture.texture.height };
        Rectangle destRec = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(m_sceneTexture.texture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
    }
    else
    {
        RenderTexture2D *source = &m_sceneTexture;
        RenderTexture2D *dest = &m_pingPongTexture;

        for (size_t i = 0; i < m_passes.size(); ++i)
        {
            const RenderPass& pass = m_passes[i];
            if (pass.width > 0 || pass.height > 0)
            {
                Warning("RenderPass custom size is not fully supported yet and will be ignored.");
            }

            bool isLastPass = (i == m_passes.size() - 1);

            if (!isLastPass) BeginTextureMode(*dest);
            
            if (pass.shouldClear) ClearBackground(pass.clearColor);

            Shader* shader = BindingsDraw::getLoadedShader(pass.shaderId);
            if (shader) BeginShaderMode(*shader);

            Rectangle sourceRec = { 0, 0, (float)source->texture.width, -(float)source->texture.height };
            Rectangle destRec = isLastPass ? Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() } : Rectangle{ 0, 0, (float)dest->texture.width, (float)dest->texture.height };
            DrawTexturePro(source->texture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);

            if (shader) EndShaderMode();
            if (!isLastPass) EndTextureMode();

            if (!isLastPass) std::swap(source, dest);
        }
    }

    // Draw UI elements on top of the final processed image
    BindingsDraw::RenderScreenCommands();
    BindingsInput::drawVirtualKeys();
    DrawFade();
    
    EndDrawing();
}

void Renderer::renderFrame(Interpreter* vm, float dt)
{
    bool usePostProcessing = m_postProcessingEnabled;

    if (usePostProcessing)
    {
        // 1. Render the entire game scene to an off-screen texture
        BeginTextureMode(m_sceneTexture);
            ClearBackground(BACKGROUND_COLOR);
            drawSceneContent(vm, dt);
        EndTextureMode();

        // 2. Apply post-processing passes
        drawPostProcessingPath();
    }
    else
    {
        // Render directly to the screen
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            drawSceneContent(vm, dt);
            BindingsDraw::RenderScreenCommands();
            BindingsInput::drawVirtualKeys();
            DrawFade();
        EndDrawing();
    }
}