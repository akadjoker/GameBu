#include "Viewport.h"
#include <rlgl.h>
#include <cmath>
#include <algorithm>

namespace le
{

Viewport::Viewport() = default;

Viewport::~Viewport()
{
    // Shutdown() should be called explicitly before CloseWindow.
    // If it wasn't, don't touch GPU resources — the GL context may be gone.
}

void Viewport::Shutdown()
{
    if (m_initialized)
    {
        UnloadRenderTexture(m_rt);
        m_initialized = false;
    }
}

void Viewport::Init(int width, int height)
{
    m_rtWidth  = width;
    m_rtHeight = height;
    m_rt = LoadRenderTexture(width, height);
    m_camera.target   = { 0.0f, 0.0f };
    m_camera.offset   = { width / 2.0f, height / 2.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom     = 1.0f;
    m_initialized     = true;
}

void Viewport::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    if (width == m_rtWidth && height == m_rtHeight) return;
    if (m_initialized)
        UnloadRenderTexture(m_rt);
    m_rtWidth  = width;
    m_rtHeight = height;
    m_rt = LoadRenderTexture(width, height);
    m_camera.offset = { width / 2.0f, height / 2.0f };
    m_initialized = true;
}

// ── Render ──────────────────────────────────────────────────────────────────

ImVec2 Viewport::Render(SceneDocument& doc, int activeLayer, uint32_t selectedUID,
                         const std::function<void(int, float, float, float, float, float, Color, bool, bool)>& drawGraphFn,
                         Gizmo2D* gizmo, SceneObject* selectedObj)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int wantW = std::max(64, (int)avail.x);
    int wantH = std::max(64, (int)avail.y);
    Resize(wantW, wantH);

    m_origin = ImGui::GetCursorScreenPos();
    m_size   = { (float)m_rtWidth, (float)m_rtHeight };
    m_hovered = ImGui::IsWindowHovered() &&
                ImGui::IsMouseHoveringRect(m_origin, { m_origin.x + m_size.x, m_origin.y + m_size.y });

    HandleInput(doc);

    // Draw into render texture
    BeginTextureMode(m_rt);
    ClearBackground({ 40, 40, 45, 255 });
    BeginMode2D(m_camera);

    DrawGrid();
    DrawSolids(doc);
    DrawObjects(doc, activeLayer, selectedUID, drawGraphFn);

    // Draw gizmo on top of everything (still in world coords)
    if (gizmo && selectedObj)
        gizmo->Draw(selectedObj, m_camera.zoom);

    EndMode2D();
    EndTextureMode();

    // Display as ImGui image (flip Y for OpenGL)
    ImGui::Image(
        (ImTextureID)(intptr_t)m_rt.texture.id,
        m_size,
        ImVec2(0, 1), ImVec2(1, 0));

    return m_size;
}

// ── Input ───────────────────────────────────────────────────────────────────

void Viewport::HandleInput(SceneDocument& /*doc*/)
{
    if (!m_hovered) return;

    ImGuiIO& io = ImGui::GetIO();

    // Zoom with scroll wheel
    if (std::abs(io.MouseWheel) > 0.0f)
    {
        float zoomDelta = io.MouseWheel * 0.1f;
        m_camera.zoom = std::clamp(m_camera.zoom + zoomDelta * m_camera.zoom, 0.05f, 20.0f);
    }

    // Pan with middle mouse button
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        if (!m_panning)
        {
            m_panning  = true;
            m_panStart = { io.MousePos.x, io.MousePos.y };
        }
        else
        {
            float dx = io.MousePos.x - m_panStart.x;
            float dy = io.MousePos.y - m_panStart.y;
            m_camera.target.x -= dx / m_camera.zoom;
            m_camera.target.y -= dy / m_camera.zoom;
            m_panStart = { io.MousePos.x, io.MousePos.y };
        }
    }
    else
    {
        m_panning = false;
    }
}

// ── Coordinate Conversion ───────────────────────────────────────────────────

Vector2 Viewport::ScreenToWorld(ImVec2 screenPos) const
{
    // screenPos is in ImGui screen coordinates; convert to RT-local first
    float lx = screenPos.x - m_origin.x;
    float ly = screenPos.y - m_origin.y;
    // Then apply inverse camera transform
    float wx = (lx - m_camera.offset.x) / m_camera.zoom + m_camera.target.x;
    float wy = (ly - m_camera.offset.y) / m_camera.zoom + m_camera.target.y;
    return { wx, wy };
}

ImVec2 Viewport::WorldToScreen(Vector2 worldPos) const
{
    float lx = (worldPos.x - m_camera.target.x) * m_camera.zoom + m_camera.offset.x;
    float ly = (worldPos.y - m_camera.target.y) * m_camera.zoom + m_camera.offset.y;
    return { m_origin.x + lx, m_origin.y + ly };
}

// ── Hit Test ────────────────────────────────────────────────────────────────

HitResult Viewport::HitTest(const SceneDocument& doc, ImVec2 screenPos, int activeLayer) const
{
    Vector2 world = ScreenToWorld(screenPos);
    HitResult best;

    // Search all objects on the active layer (or all layers if activeLayer < 0)
    for (auto it = doc.layers.rbegin(); it != doc.layers.rend(); ++it)
    {
        const SceneLayer& layer = *it;
        if (activeLayer >= 0 && layer.index != activeLayer) continue;
        if (!layer.visible || layer.locked) continue;

        // Reverse iterate for top-most first (higher z)
        for (auto oit = layer.objects.rbegin(); oit != layer.objects.rend(); ++oit)
        {
            const SceneObject& obj = *oit;
            if (!obj.visible) continue;

            // Simple AABB test (use graph size or default 32x32)
            float hw = 16.0f * (float)(obj.size_x / 100.0);
            float hh = 16.0f * (float)(obj.size_y / 100.0);
            if (world.x >= obj.x - hw && world.x <= obj.x + hw &&
                world.y >= obj.y - hh && world.y <= obj.y + hh)
            {
                best.uid   = obj.uid;
                best.layer = layer.index;
                best.hit   = true;
                return best; // topmost
            }
        }
    }
    return best;
}

// ── Drawing Helpers ─────────────────────────────────────────────────────────

void Viewport::DrawGrid()
{
    if (!m_showGrid) return;

    float left   = m_camera.target.x - m_camera.offset.x / m_camera.zoom;
    float right  = m_camera.target.x + m_camera.offset.x / m_camera.zoom;
    float top    = m_camera.target.y - m_camera.offset.y / m_camera.zoom;
    float bottom = m_camera.target.y + m_camera.offset.y / m_camera.zoom;

    float startX = std::floor(left / m_gridSize) * m_gridSize;
    float startY = std::floor(top / m_gridSize) * m_gridSize;

    for (float x = startX; x <= right; x += m_gridSize)
        DrawLine((int)x, (int)top, (int)x, (int)bottom, m_gridColor);
    for (float y = startY; y <= bottom; y += m_gridSize)
        DrawLine((int)left, (int)y, (int)right, (int)y, m_gridColor);

    // Origin axes
    DrawLine(0, (int)top, 0, (int)bottom, m_originColor);
    DrawLine((int)left, 0, (int)right, 0, m_originColor);
}

void Viewport::DrawSolids(const SceneDocument& doc)
{
    for (auto& s : doc.solids)
    {
        DrawRectangleLines((int)s.x, (int)s.y, (int)s.w, (int)s.h, { 255, 100, 100, 120 });
    }
}

void Viewport::DrawObjects(SceneDocument& doc, int activeLayer, uint32_t selectedUID,
                            const std::function<void(int, float, float, float, float, float, Color, bool, bool)>& drawGraphFn)
{
    for (auto& layer : doc.layers)
    {
        if (!layer.visible) continue;

        // Dim non-active layers
        float alpha = (activeLayer >= 0 && layer.index != activeLayer) ? 0.3f : 1.0f;

        // Sort by z
        std::stable_sort(layer.objects.begin(), layer.objects.end(),
            [](const SceneObject& a, const SceneObject& b) { return a.z < b.z; });

        for (auto& obj : layer.objects)
        {
            if (!obj.visible) continue;

            Color tint = { obj.r, obj.g, obj.b, (unsigned char)(obj.a * alpha) };

            if (obj.graph >= 0 && drawGraphFn)
            {
                drawGraphFn(obj.graph, (float)obj.x, (float)obj.y, (float)obj.angle,
                            (float)obj.size_x, (float)obj.size_y, tint, obj.flip_x, obj.flip_y);
            }
            else
            {
                // No graph: draw a placeholder rectangle
                float hw = 16.0f * (float)(obj.size_x / 100.0);
                float hh = 16.0f * (float)(obj.size_y / 100.0);
                DrawRectangleLines((int)(obj.x - hw), (int)(obj.y - hh),
                                   (int)(hw * 2), (int)(hh * 2), tint);
                // Draw name label
                if (!obj.name.empty())
                {
                    DrawText(obj.name.c_str(), (int)(obj.x - hw + 2), (int)(obj.y - hh + 2), 10,
                             { 200, 200, 200, (unsigned char)(200 * alpha) });
                }
            }

            // Selection highlight
            if (obj.uid == selectedUID && selectedUID != 0)
            {
                DrawSelection(obj);
            }
        }
    }
}

void Viewport::DrawSelection(const SceneObject& obj)
{
    float hw = 18.0f * (float)(obj.size_x / 100.0);
    float hh = 18.0f * (float)(obj.size_y / 100.0);
    float x  = (float)obj.x;
    float y  = (float)obj.y;

    // Selection outline
    DrawRectangleLinesEx({ x - hw, y - hh, hw * 2, hh * 2 }, 2.0f, { 0, 180, 255, 220 });

    // Small handles at corners
    float hs = 4.0f;
    Color hc = { 255, 255, 255, 255 };
    DrawRectangle((int)(x - hw - hs/2), (int)(y - hh - hs/2), (int)hs, (int)hs, hc);
    DrawRectangle((int)(x + hw - hs/2), (int)(y - hh - hs/2), (int)hs, (int)hs, hc);
    DrawRectangle((int)(x - hw - hs/2), (int)(y + hh - hs/2), (int)hs, (int)hs, hc);
    DrawRectangle((int)(x + hw - hs/2), (int)(y + hh - hs/2), (int)hs, (int)hs, hc);
}

} // namespace le
