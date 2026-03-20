#pragma once
// Viewport — renders the scene into a RenderTexture displayed in an ImGui panel.
// Provides camera pan (middle-click drag), zoom (scroll wheel), and coordinate conversion.

#include "SceneDocument.h"
#include "Gizmo2D.h"
#include <raylib.h>
#include <imgui.h>
#include <string>
#include <functional>

namespace le
{

// Forward
struct EditorContext;

// Hit-test result
struct HitResult
{
    uint32_t uid   = 0;
    int      layer = -1;
    bool     hit   = false;
};

class Viewport
{
public:
    Viewport();
    ~Viewport();

    // Call once after raylib InitWindow
    void Init(int width, int height);
    void Resize(int width, int height);
    void Shutdown(); // Call before CloseWindow to release GPU resources safely

    // Render the viewport as an ImGui image inside the current window.
    // Returns the ImGui content region used.
    // If gizmo is non-null and selectedObj is valid, draws the gizmo on top.
    ImVec2 Render(SceneDocument& doc, int activeLayer, uint32_t selectedUID,
                  const std::function<void(int graphId, float x, float y, float angle,
                                           float sx, float sy, Color tint, bool flipX, bool flipY)>& drawGraphFn,
                  Gizmo2D* gizmo = nullptr, SceneObject* selectedObj = nullptr);

    // Camera
    Vector2 GetCameraTarget() const { return m_camera.target; }
    float   GetCameraZoom()   const { return m_camera.zoom; }
    void    SetCameraTarget(float x, float y) { m_camera.target = {x, y}; }
    void    SetCameraZoom(float z) { m_camera.zoom = z; }

    // Coordinate conversion
    Vector2 ScreenToWorld(ImVec2 screenPos) const;
    ImVec2  WorldToScreen(Vector2 worldPos) const;

    // Hit-test: find topmost object at screen position
    HitResult HitTest(const SceneDocument& doc, ImVec2 screenPos, int activeLayer) const;

    // State
    bool IsHovered() const { return m_hovered; }
    ImVec2 GetOrigin() const { return m_origin; }
    ImVec2 GetSize()   const { return m_size; }

private:
    void HandleInput(SceneDocument& doc);
    void DrawGrid();
    void DrawObjects(SceneDocument& doc, int activeLayer, uint32_t selectedUID,
                     const std::function<void(int, float, float, float, float, float, Color, bool, bool)>& drawGraphFn);
    void DrawSelection(const SceneObject& obj);
    void DrawSolids(const SceneDocument& doc);

    RenderTexture2D m_rt;
    Camera2D        m_camera;
    int             m_rtWidth, m_rtHeight;
    bool            m_initialized = false;
    bool            m_hovered     = false;
    bool            m_panning     = false;
    Vector2         m_panStart    = {};
    ImVec2          m_origin      = {};   // top-left of the ImGui image in screen coords
    ImVec2          m_size        = {};

    // Grid
    bool  m_showGrid    = true;
    float m_gridSize    = 32.0f;
    Color m_gridColor   = { 60, 60, 60, 100 };
    Color m_originColor = { 100, 100, 100, 180 };
};

} // namespace le
