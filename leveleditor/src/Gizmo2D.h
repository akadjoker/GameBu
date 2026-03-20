#pragma once
// Gizmo2D — Interactive 2D transform gizmo for the level editor.
// Draws translate (arrows), rotate (ring), and scale (square handles) controls.
// All coordinates are in world-space; rendered inside BeginMode2D/EndMode2D.

#include "SceneDocument.h"
#include <raylib.h>
#include <raymath.h>
#include <imgui.h>
#include <cmath>
#include <algorithm>

namespace le
{

// ── Gizmo mode ──────────────────────────────────────────────────────────────
enum class GizmoMode { Translate, Rotate, Scale };

// Which part of the gizmo the mouse is hovering/dragging
enum class GizmoPart { None, CenterXY, AxisX, AxisY, Ring };

// ── Gizmo2D ─────────────────────────────────────────────────────────────────
class Gizmo2D
{
public:
    GizmoMode mode = GizmoMode::Translate;

    // Style (sizes in world units, but we scale by 1/zoom so they look constant on screen)
    float axisLength   = 70.0f;   // pixels on screen
    float axisThick    = 3.0f;
    float arrowSize    = 10.0f;
    float centerSize   = 10.0f;   // center square half-size
    float ringRadius   = 55.0f;   // rotate ring radius (screen pixels)
    float ringThick    = 3.0f;
    float scaleHandleSize = 7.0f; // half-size of scale square handles

    // Colors
    Color colorX       = { 230, 60,  60,  240 };
    Color colorY       = {  60, 200, 60,  240 };
    Color colorCenter  = { 255, 255, 100, 200 };
    Color colorRing    = { 100, 180, 255, 220 };
    Color colorHover   = { 255, 255,   0, 255 };

    // ── Update — call every frame, returns true if gizmo consumed the input ──
    // worldMouse: current mouse position in world coordinates
    // zoom: camera zoom (used to keep gizmo screen-size constant)
    // leftDown, leftClicked: mouse button state
    bool Update(SceneObject* obj, Vector2 worldMouse, float zoom,
                bool leftDown, bool leftClicked, bool leftReleased)
    {
        if (!obj) { m_hoveredPart = GizmoPart::None; m_dragging = false; return false; }

        float inv = 1.0f / std::max(zoom, 0.01f);
        Vector2 center = { (float)obj->x, (float)obj->y };

        if (!m_dragging)
            m_hoveredPart = HitTestGizmo(center, worldMouse, inv);

        // Start drag
        if (leftClicked && m_hoveredPart != GizmoPart::None)
        {
            m_dragging = true;
            m_dragPart = m_hoveredPart;
            m_dragStart = worldMouse;
            m_dragObjStart = center;
            m_dragAngleStart = (float)obj->angle;
            m_dragScaleStartX = (float)obj->size_x;
            m_dragScaleStartY = (float)obj->size_y;
            m_dragStartAngleToMouse = std::atan2(worldMouse.y - center.y, worldMouse.x - center.x);
        }

        // Dragging
        if (m_dragging && leftDown)
        {
            switch (mode)
            {
            case GizmoMode::Translate:
                HandleTranslate(obj, worldMouse);
                break;
            case GizmoMode::Rotate:
                HandleRotate(obj, worldMouse, center);
                break;
            case GizmoMode::Scale:
                HandleScale(obj, worldMouse, inv);
                break;
            }
            return true;
        }

        // Release
        if (m_dragging && leftReleased)
        {
            m_dragging = false;
            return true;
        }

        return m_hoveredPart != GizmoPart::None;
    }

    // ── Draw — call inside BeginMode2D ──
    void Draw(const SceneObject* obj, float zoom) const
    {
        if (!obj) return;

        float inv = 1.0f / std::max(zoom, 0.01f);
        Vector2 center = { (float)obj->x, (float)obj->y };

        switch (mode)
        {
        case GizmoMode::Translate: DrawTranslateGizmo(center, inv); break;
        case GizmoMode::Rotate:    DrawRotateGizmo(center, inv, (float)obj->angle); break;
        case GizmoMode::Scale:     DrawScaleGizmo(center, inv); break;
        }
    }

    bool IsDragging() const { return m_dragging; }
    GizmoPart HoveredPart() const { return m_hoveredPart; }

private:
    GizmoPart m_hoveredPart = GizmoPart::None;
    bool      m_dragging    = false;
    GizmoPart m_dragPart    = GizmoPart::None;
    Vector2   m_dragStart   = {};
    Vector2   m_dragObjStart = {};
    float     m_dragAngleStart = 0.0f;
    float     m_dragScaleStartX = 100.0f;
    float     m_dragScaleStartY = 100.0f;
    float     m_dragStartAngleToMouse = 0.0f;

    // ── Hit test against the gizmo parts ──
    GizmoPart HitTestGizmo(Vector2 center, Vector2 mouse, float inv) const
    {
        float mx = mouse.x - center.x;
        float my = mouse.y - center.y;

        if (mode == GizmoMode::Translate || mode == GizmoMode::Scale)
        {
            float len = axisLength * inv;
            float handleHalf = (mode == GizmoMode::Scale ? scaleHandleSize : arrowSize) * inv;
            float thick = 8.0f * inv; // click tolerance

            // Center handle
            float cs = centerSize * inv;
            if (mx >= -cs && mx <= cs && my >= -cs && my <= cs)
                return GizmoPart::CenterXY;

            // X-axis handle (tip area for translate arrow / scale square)
            if (mx >= len - handleHalf && mx <= len + handleHalf &&
                my >= -thick && my <= thick)
                return GizmoPart::AxisX;

            // X-axis shaft
            if (mx >= cs && mx <= len && my >= -thick && my <= thick)
                return GizmoPart::AxisX;

            // Y-axis handle
            if (my >= len - handleHalf && my <= len + handleHalf &&
                mx >= -thick && mx <= thick)
                return GizmoPart::AxisY;

            // Y-axis shaft
            if (my >= cs && my <= len && mx >= -thick && mx <= thick)
                return GizmoPart::AxisY;
        }

        if (mode == GizmoMode::Rotate)
        {
            float dist = std::sqrt(mx * mx + my * my);
            float r = ringRadius * inv;
            float tolerance = 10.0f * inv;
            if (dist >= r - tolerance && dist <= r + tolerance)
                return GizmoPart::Ring;

            // Center for free rotate
            float cs = centerSize * inv;
            if (mx >= -cs && mx <= cs && my >= -cs && my <= cs)
                return GizmoPart::Ring;
        }

        return GizmoPart::None;
    }

    // ── Transform handlers ──

    void HandleTranslate(SceneObject* obj, Vector2 worldMouse)
    {
        Vector2 delta = { worldMouse.x - m_dragStart.x, worldMouse.y - m_dragStart.y };

        switch (m_dragPart)
        {
        case GizmoPart::CenterXY:
            obj->x = m_dragObjStart.x + delta.x;
            obj->y = m_dragObjStart.y + delta.y;
            break;
        case GizmoPart::AxisX:
            obj->x = m_dragObjStart.x + delta.x;
            obj->y = m_dragObjStart.y; // lock Y
            break;
        case GizmoPart::AxisY:
            obj->x = m_dragObjStart.x; // lock X
            obj->y = m_dragObjStart.y + delta.y;
            break;
        default: break;
        }
    }

    void HandleRotate(SceneObject* obj, Vector2 worldMouse, Vector2 center)
    {
        float angleNow = std::atan2(worldMouse.y - center.y, worldMouse.x - center.x);
        float angleDelta = (angleNow - m_dragStartAngleToMouse) * (180.0f / 3.14159265f);
        obj->angle = m_dragAngleStart + angleDelta;
        // Normalize to [0, 360)
        while (obj->angle < 0)    obj->angle += 360;
        while (obj->angle >= 360) obj->angle -= 360;
    }

    void HandleScale(SceneObject* obj, Vector2 worldMouse, float inv)
    {
        Vector2 delta = { worldMouse.x - m_dragStart.x, worldMouse.y - m_dragStart.y };
        float sensitivity = 1.0f / std::max(axisLength * inv, 1.0f) * 100.0f;

        switch (m_dragPart)
        {
        case GizmoPart::CenterXY:
        {
            // Uniform scale: use average of X and Y movement
            float avg = (delta.x + delta.y) * 0.5f * sensitivity;
            obj->size_x = std::max(5.0, (double)(m_dragScaleStartX + avg));
            obj->size_y = std::max(5.0, (double)(m_dragScaleStartY + avg));
            break;
        }
        case GizmoPart::AxisX:
            obj->size_x = std::max(5.0, (double)(m_dragScaleStartX + delta.x * sensitivity));
            break;
        case GizmoPart::AxisY:
            obj->size_y = std::max(5.0, (double)(m_dragScaleStartY + delta.y * sensitivity));
            break;
        default: break;
        }
    }

    // ── Drawing ──

    void DrawTranslateGizmo(Vector2 c, float inv) const
    {
        float len = axisLength * inv;
        float arr = arrowSize * inv;
        float cs  = centerSize * inv;

        Color xCol = (m_hoveredPart == GizmoPart::AxisX) ? colorHover : colorX;
        Color yCol = (m_hoveredPart == GizmoPart::AxisY) ? colorHover : colorY;
        Color cCol = (m_hoveredPart == GizmoPart::CenterXY) ? colorHover : colorCenter;

        // X axis: line + arrow
        DrawLineEx({ c.x + cs, c.y }, { c.x + len, c.y }, axisThick * inv, xCol);
        // Arrow head (triangle pointing right)
        Vector2 tip = { c.x + len + arr, c.y };
        Vector2 t1  = { c.x + len - arr * 0.3f, c.y - arr * 0.5f };
        Vector2 t2  = { c.x + len - arr * 0.3f, c.y + arr * 0.5f };
        DrawTriangle(tip, t2, t1, xCol);

        // Y axis: line + arrow (pointing down, +Y is down in screen)
        DrawLineEx({ c.x, c.y + cs }, { c.x, c.y + len }, axisThick * inv, yCol);
        Vector2 tipY = { c.x, c.y + len + arr };
        Vector2 ty1  = { c.x - arr * 0.5f, c.y + len - arr * 0.3f };
        Vector2 ty2  = { c.x + arr * 0.5f, c.y + len - arr * 0.3f };
        DrawTriangle(tipY, ty1, ty2, yCol);

        // Center square
        DrawRectangle((int)(c.x - cs), (int)(c.y - cs), (int)(cs * 2), (int)(cs * 2), Fade(cCol, 0.4f));
        DrawRectangleLinesEx({ c.x - cs, c.y - cs, cs * 2, cs * 2 }, inv, cCol);

        // Axis labels
        float labelOff = 14.0f * inv;
        DrawText("X", (int)(c.x + len + arr + 2 * inv), (int)(c.y - 5 * inv), (int)(12 * inv), xCol);
        DrawText("Y", (int)(c.x - 5 * inv), (int)(c.y + len + arr + 2 * inv), (int)(12 * inv), yCol);
    }

    void DrawRotateGizmo(Vector2 c, float inv, float currentAngle) const
    {
        float r = ringRadius * inv;
        Color rCol = (m_hoveredPart == GizmoPart::Ring) ? colorHover : colorRing;

        // Draw ring as line segments
        int segments = 64;
        for (int i = 0; i < segments; i++)
        {
            float a1 = (float)i / segments * 2.0f * 3.14159265f;
            float a2 = (float)(i + 1) / segments * 2.0f * 3.14159265f;
            Vector2 p1 = { c.x + std::cos(a1) * r, c.y + std::sin(a1) * r };
            Vector2 p2 = { c.x + std::cos(a2) * r, c.y + std::sin(a2) * r };
            DrawLineEx(p1, p2, ringThick * inv, rCol);
        }

        // Current angle indicator line
        float rad = currentAngle * (3.14159265f / 180.0f);
        Vector2 angleEnd = { c.x + std::cos(rad) * r, c.y + std::sin(rad) * r };
        DrawLineEx(c, angleEnd, 2.0f * inv, { 255, 255, 255, 180 });

        // Small dot at angle end
        DrawCircleV(angleEnd, 4.0f * inv, { 255, 255, 255, 220 });

        // Center dot
        float cs = 5.0f * inv;
        DrawCircleV(c, cs, Fade(colorRing, 0.5f));

        // Angle text
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f°", currentAngle);
        DrawText(buf, (int)(c.x + r + 8 * inv), (int)(c.y - 6 * inv), (int)(12 * inv), rCol);
    }

    void DrawScaleGizmo(Vector2 c, float inv) const
    {
        float len = axisLength * inv;
        float hs  = scaleHandleSize * inv;
        float cs  = centerSize * inv;

        Color xCol = (m_hoveredPart == GizmoPart::AxisX) ? colorHover : colorX;
        Color yCol = (m_hoveredPart == GizmoPart::AxisY) ? colorHover : colorY;
        Color cCol = (m_hoveredPart == GizmoPart::CenterXY) ? colorHover : colorCenter;

        // X axis line + square handle
        DrawLineEx({ c.x + cs, c.y }, { c.x + len, c.y }, axisThick * inv, xCol);
        DrawRectangle((int)(c.x + len - hs), (int)(c.y - hs), (int)(hs * 2), (int)(hs * 2), xCol);

        // Y axis line + square handle
        DrawLineEx({ c.x, c.y + cs }, { c.x, c.y + len }, axisThick * inv, yCol);
        DrawRectangle((int)(c.x - hs), (int)(c.y + len - hs), (int)(hs * 2), (int)(hs * 2), yCol);

        // Center square (uniform scale)
        DrawRectangle((int)(c.x - cs), (int)(c.y - cs), (int)(cs * 2), (int)(cs * 2), Fade(cCol, 0.4f));
        DrawRectangleLinesEx({ c.x - cs, c.y - cs, cs * 2, cs * 2 }, inv, cCol);

        // Labels
        DrawText("Sx", (int)(c.x + len + hs + 2 * inv), (int)(c.y - 5 * inv), (int)(12 * inv), xCol);
        DrawText("Sy", (int)(c.x - 8 * inv), (int)(c.y + len + hs + 2 * inv), (int)(12 * inv), yCol);
    }
};

} // namespace le
