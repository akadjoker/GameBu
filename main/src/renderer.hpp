#pragma once

#include <vector>
#include <raylib.h>

class Interpreter;

// A single pass in the post-processing pipeline
struct RenderPass
{
    int shaderId = -1;
    bool shouldClear = false;
    Color clearColor = { 0, 0, 0, 0 };
    int width = 0;
    int height = 0;
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void init(int width, int height);
    void shutdown();
    void onWindowResize();

    void renderFrame(Interpreter* vm, float dt);

    // API
    void enablePostProcessing(bool enabled);
    bool isPostProcessingEnabled() const;
    void addPostProcessingPass(const RenderPass& pass);
    void clearPostProcessingPasses();

private:
    void drawSceneContent(Interpreter* vm, float dt);
    void drawPostProcessingPath();

    bool m_postProcessingEnabled;
    std::vector<RenderPass> m_passes;

    RenderTexture2D m_sceneTexture;
    RenderTexture2D m_pingPongTexture;
};