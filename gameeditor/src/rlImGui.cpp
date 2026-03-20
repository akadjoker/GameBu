// rlImGui - Dear ImGui backend for raylib
// Renders ImGui draw data through rlgl (raylib's OpenGL abstraction).
// Handles input mapping from raylib → ImGui.

#include "rlImGui.h"

#include "imgui.h"
#include "raylib.h"
#include "rlgl.h"

#include <cmath>
#include <cstdint>
#include <cstring>

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------
static unsigned int g_FontTexture = 0;

// Previous key states for edge detection
static bool g_PrevKeyState[512] = {};
static bool g_PrevMouseState[3] = {};

// Cursor mapping
static MouseCursor g_LastImGuiCursor = MOUSE_CURSOR_DEFAULT;

// ---------------------------------------------------------------------------
// Key mapping: raylib KeyboardKey → ImGuiKey
// ---------------------------------------------------------------------------
struct KeyPair { int raylib; ImGuiKey imgui; };

static const KeyPair g_KeyMap[] = {
    { KEY_TAB,            ImGuiKey_Tab },
    { KEY_LEFT,           ImGuiKey_LeftArrow },
    { KEY_RIGHT,          ImGuiKey_RightArrow },
    { KEY_UP,             ImGuiKey_UpArrow },
    { KEY_DOWN,           ImGuiKey_DownArrow },
    { KEY_PAGE_UP,        ImGuiKey_PageUp },
    { KEY_PAGE_DOWN,      ImGuiKey_PageDown },
    { KEY_HOME,           ImGuiKey_Home },
    { KEY_END,            ImGuiKey_End },
    { KEY_INSERT,         ImGuiKey_Insert },
    { KEY_DELETE,         ImGuiKey_Delete },
    { KEY_BACKSPACE,      ImGuiKey_Backspace },
    { KEY_SPACE,          ImGuiKey_Space },
    { KEY_ENTER,          ImGuiKey_Enter },
    { KEY_ESCAPE,         ImGuiKey_Escape },
    { KEY_APOSTROPHE,     ImGuiKey_Apostrophe },
    { KEY_COMMA,          ImGuiKey_Comma },
    { KEY_MINUS,          ImGuiKey_Minus },
    { KEY_PERIOD,         ImGuiKey_Period },
    { KEY_SLASH,          ImGuiKey_Slash },
    { KEY_SEMICOLON,      ImGuiKey_Semicolon },
    { KEY_EQUAL,          ImGuiKey_Equal },
    { KEY_LEFT_BRACKET,   ImGuiKey_LeftBracket },
    { KEY_BACKSLASH,      ImGuiKey_Backslash },
    { KEY_RIGHT_BRACKET,  ImGuiKey_RightBracket },
    { KEY_GRAVE,          ImGuiKey_GraveAccent },
    { KEY_CAPS_LOCK,      ImGuiKey_CapsLock },
    { KEY_SCROLL_LOCK,    ImGuiKey_ScrollLock },
    { KEY_NUM_LOCK,       ImGuiKey_NumLock },
    { KEY_PRINT_SCREEN,   ImGuiKey_PrintScreen },
    { KEY_PAUSE,          ImGuiKey_Pause },
    { KEY_KP_0,           ImGuiKey_Keypad0 },
    { KEY_KP_1,           ImGuiKey_Keypad1 },
    { KEY_KP_2,           ImGuiKey_Keypad2 },
    { KEY_KP_3,           ImGuiKey_Keypad3 },
    { KEY_KP_4,           ImGuiKey_Keypad4 },
    { KEY_KP_5,           ImGuiKey_Keypad5 },
    { KEY_KP_6,           ImGuiKey_Keypad6 },
    { KEY_KP_7,           ImGuiKey_Keypad7 },
    { KEY_KP_8,           ImGuiKey_Keypad8 },
    { KEY_KP_9,           ImGuiKey_Keypad9 },
    { KEY_KP_DECIMAL,     ImGuiKey_KeypadDecimal },
    { KEY_KP_DIVIDE,      ImGuiKey_KeypadDivide },
    { KEY_KP_MULTIPLY,    ImGuiKey_KeypadMultiply },
    { KEY_KP_SUBTRACT,    ImGuiKey_KeypadSubtract },
    { KEY_KP_ADD,         ImGuiKey_KeypadAdd },
    { KEY_KP_ENTER,       ImGuiKey_KeypadEnter },
    { KEY_KP_EQUAL,       ImGuiKey_KeypadEqual },
    { KEY_LEFT_SHIFT,     ImGuiKey_LeftShift },
    { KEY_LEFT_CONTROL,   ImGuiKey_LeftCtrl },
    { KEY_LEFT_ALT,       ImGuiKey_LeftAlt },
    { KEY_LEFT_SUPER,     ImGuiKey_LeftSuper },
    { KEY_RIGHT_SHIFT,    ImGuiKey_RightShift },
    { KEY_RIGHT_CONTROL,  ImGuiKey_RightCtrl },
    { KEY_RIGHT_ALT,      ImGuiKey_RightAlt },
    { KEY_RIGHT_SUPER,    ImGuiKey_RightSuper },
    { KEY_KB_MENU,        ImGuiKey_Menu },
    { KEY_ZERO,           ImGuiKey_0 },
    { KEY_ONE,            ImGuiKey_1 },
    { KEY_TWO,            ImGuiKey_2 },
    { KEY_THREE,          ImGuiKey_3 },
    { KEY_FOUR,           ImGuiKey_4 },
    { KEY_FIVE,           ImGuiKey_5 },
    { KEY_SIX,            ImGuiKey_6 },
    { KEY_SEVEN,          ImGuiKey_7 },
    { KEY_EIGHT,          ImGuiKey_8 },
    { KEY_NINE,           ImGuiKey_9 },
    { KEY_A,              ImGuiKey_A },
    { KEY_B,              ImGuiKey_B },
    { KEY_C,              ImGuiKey_C },
    { KEY_D,              ImGuiKey_D },
    { KEY_E,              ImGuiKey_E },
    { KEY_F,              ImGuiKey_F },
    { KEY_G,              ImGuiKey_G },
    { KEY_H,              ImGuiKey_H },
    { KEY_I,              ImGuiKey_I },
    { KEY_J,              ImGuiKey_J },
    { KEY_K,              ImGuiKey_K },
    { KEY_L,              ImGuiKey_L },
    { KEY_M,              ImGuiKey_M },
    { KEY_N,              ImGuiKey_N },
    { KEY_O,              ImGuiKey_O },
    { KEY_P,              ImGuiKey_P },
    { KEY_Q,              ImGuiKey_Q },
    { KEY_R,              ImGuiKey_R },
    { KEY_S,              ImGuiKey_S },
    { KEY_T,              ImGuiKey_T },
    { KEY_U,              ImGuiKey_U },
    { KEY_V,              ImGuiKey_V },
    { KEY_W,              ImGuiKey_W },
    { KEY_X,              ImGuiKey_X },
    { KEY_Y,              ImGuiKey_Y },
    { KEY_Z,              ImGuiKey_Z },
    { KEY_F1,             ImGuiKey_F1 },
    { KEY_F2,             ImGuiKey_F2 },
    { KEY_F3,             ImGuiKey_F3 },
    { KEY_F4,             ImGuiKey_F4 },
    { KEY_F5,             ImGuiKey_F5 },
    { KEY_F6,             ImGuiKey_F6 },
    { KEY_F7,             ImGuiKey_F7 },
    { KEY_F8,             ImGuiKey_F8 },
    { KEY_F9,             ImGuiKey_F9 },
    { KEY_F10,            ImGuiKey_F10 },
    { KEY_F11,            ImGuiKey_F11 },
    { KEY_F12,            ImGuiKey_F12 },
};
static constexpr int g_KeyMapSize = sizeof(g_KeyMap) / sizeof(g_KeyMap[0]);

// ---------------------------------------------------------------------------
// Font texture management
// ---------------------------------------------------------------------------
static void BuildFontTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_FontTexture = rlLoadTexture(pixels, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    // Set bilinear filtering
    rlTextureParameters(g_FontTexture, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
    rlTextureParameters(g_FontTexture, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);

    io.Fonts->SetTexID((ImTextureID)(intptr_t)g_FontTexture);
}

static void CleanupFontTexture()
{
    if (g_FontTexture != 0)
    {
        rlUnloadTexture(g_FontTexture);
        g_FontTexture = 0;
        ImGui::GetIO().Fonts->SetTexID(0);
    }
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
static void RenderImDrawData(ImDrawData* drawData)
{
    // Avoid rendering when minimized
    int fbWidth  = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth <= 0 || fbHeight <= 0) return;

    // Flush any pending raylib geometry
    rlDrawRenderBatchActive();

    // ── Save and configure GPU state ──
    rlDisableBackfaceCulling();
    rlDisableDepthTest();
    rlDisableDepthMask();
    rlEnableColorBlend();
    rlEnableScissorTest();

    // Set up orthographic projection matching ImGui pixel coordinates
    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();

    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
    rlOrtho(L, R, B, T, -1.0, 1.0);

    rlMatrixMode(RL_MODELVIEW);
    rlPushMatrix();
    rlLoadIdentity();

    // Set viewport
    rlViewport(0, 0, fbWidth, fbHeight);

    ImVec2 clipOff   = drawData->DisplayPos;
    ImVec2 clipScale = drawData->FramebufferScale;

    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        const ImDrawVert* vtxBuffer = cmdList->VtxBuffer.Data;
        const ImDrawIdx*  idxBuffer = cmdList->IdxBuffer.Data;

        for (int cmdI = 0; cmdI < cmdList->CmdBuffer.Size; cmdI++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdI];

            if (pcmd->UserCallback != nullptr)
            {
                // Flush before the callback
                rlDrawRenderBatchActive();
                pcmd->UserCallback(cmdList, pcmd);
                continue;
            }

            // Project scissor/clipping rect onto framebuffer
            float clipMinX = (pcmd->ClipRect.x - clipOff.x) * clipScale.x;
            float clipMinY = (pcmd->ClipRect.y - clipOff.y) * clipScale.y;
            float clipMaxX = (pcmd->ClipRect.z - clipOff.x) * clipScale.x;
            float clipMaxY = (pcmd->ClipRect.w - clipOff.y) * clipScale.y;
            if (clipMaxX <= clipMinX || clipMaxY <= clipMinY) continue;

            // Apply scissor (GL-style: origin at bottom-left)
            rlScissor(
                (int)clipMinX,
                (int)(fbHeight - clipMaxY),
                (int)(clipMaxX - clipMinX),
                (int)(clipMaxY - clipMinY)
            );

            // Emit triangles
            // NOTE: rlBegin() must be called BEFORE rlSetTexture() because
            // rlBegin() resets the draw call's textureId to the default
            // white texture when the mode changes (e.g. from RL_QUADS after
            // a batch flush).  Setting the texture afterwards ensures it
            // sticks.
            unsigned int texId = (unsigned int)(intptr_t)pcmd->GetTexID();
            rlBegin(RL_TRIANGLES);
            rlSetTexture(texId);
            for (unsigned int i = 0; i < pcmd->ElemCount; i += 3)
            {
                // Process 3 vertices at a time (one triangle)
                for (int vi = 0; vi < 3; vi++)
                {
                    ImDrawIdx idx = idxBuffer[pcmd->IdxOffset + i + vi];
                    const ImDrawVert& v = vtxBuffer[pcmd->VtxOffset + idx];

                    rlColor4ub(
                        (unsigned char)((v.col >>  0) & 0xFF),
                        (unsigned char)((v.col >>  8) & 0xFF),
                        (unsigned char)((v.col >> 16) & 0xFF),
                        (unsigned char)((v.col >> 24) & 0xFF)
                    );
                    rlTexCoord2f(v.uv.x, v.uv.y);
                    rlVertex2f(v.pos.x, v.pos.y);
                }
            }
            rlEnd();

            // Flush after each draw command to ensure scissor/texture changes apply
            rlDrawRenderBatchActive();
        }
    }

    // ── Restore state ──
    rlSetTexture(0);
    rlDisableScissorTest();
    rlEnableBackfaceCulling();
    rlEnableDepthTest();
    rlEnableDepthMask();

    // Restore matrices
    rlMatrixMode(RL_MODELVIEW);
    rlPopMatrix();
    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();

    rlDrawRenderBatchActive();
}

// ---------------------------------------------------------------------------
// ImGui mouse cursor → raylib cursor
// ---------------------------------------------------------------------------
static void UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) return;

    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    if (cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        HideCursor();
    }
    else
    {
        MouseCursor raylibCursor = MOUSE_CURSOR_DEFAULT;
        switch (cursor)
        {
            case ImGuiMouseCursor_Arrow:      raylibCursor = MOUSE_CURSOR_ARROW;         break;
            case ImGuiMouseCursor_TextInput:   raylibCursor = MOUSE_CURSOR_IBEAM;         break;
            case ImGuiMouseCursor_ResizeAll:    raylibCursor = MOUSE_CURSOR_RESIZE_ALL;    break;
            case ImGuiMouseCursor_ResizeNS:     raylibCursor = MOUSE_CURSOR_RESIZE_NS;     break;
            case ImGuiMouseCursor_ResizeEW:     raylibCursor = MOUSE_CURSOR_RESIZE_EW;     break;
            case ImGuiMouseCursor_ResizeNESW:   raylibCursor = MOUSE_CURSOR_RESIZE_NESW;   break;
            case ImGuiMouseCursor_ResizeNWSE:   raylibCursor = MOUSE_CURSOR_RESIZE_NWSE;   break;
            case ImGuiMouseCursor_Hand:         raylibCursor = MOUSE_CURSOR_POINTING_HAND; break;
            case ImGuiMouseCursor_NotAllowed:   raylibCursor = MOUSE_CURSOR_NOT_ALLOWED;   break;
            default: break;
        }
        if (raylibCursor != g_LastImGuiCursor)
        {
            SetMouseCursor(raylibCursor);
            g_LastImGuiCursor = raylibCursor;
        }
        ShowCursor();
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool rlImGuiSetup(bool darkTheme)
{
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = "rlImGui_raylib";
    io.BackendRendererName = "rlImGui_rlgl";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    // Initialize key states
    std::memset(g_PrevKeyState, 0, sizeof(g_PrevKeyState));
    std::memset(g_PrevMouseState, 0, sizeof(g_PrevMouseState));

    // Build font atlas texture
    BuildFontTexture();

    if (darkTheme)
    {
        ImGui::StyleColorsDark();
    }
    else
    {
        ImGui::StyleColorsLight();
    }

    return true;
}

void rlImGuiBegin()
{
    ImGuiIO& io = ImGui::GetIO();

    // Display size
    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Handle HiDPI: check if render size differs from screen size
    int renderWidth = GetRenderWidth();
    int renderHeight = GetRenderHeight();
    if (renderWidth > 0 && renderHeight > 0)
    {
        io.DisplayFramebufferScale = ImVec2(
            (float)renderWidth / io.DisplaySize.x,
            (float)renderHeight / io.DisplaySize.y
        );
    }

    // Delta time
    float dt = GetFrameTime();
    io.DeltaTime = (dt > 0.0f) ? dt : (1.0f / 60.0f);

    // Focus
    if (IsWindowFocused())
    {
        io.AddFocusEvent(true);
    }
    else
    {
        io.AddFocusEvent(false);
    }

    // Mouse position
    if (io.WantSetMousePos)
    {
        SetMousePosition((int)io.MousePos.x, (int)io.MousePos.y);
    }
    else
    {
        io.AddMousePosEvent((float)GetMouseX(), (float)GetMouseY());
    }

    // Mouse buttons
    io.AddMouseButtonEvent(0, IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    io.AddMouseButtonEvent(1, IsMouseButtonDown(MOUSE_BUTTON_RIGHT));
    io.AddMouseButtonEvent(2, IsMouseButtonDown(MOUSE_BUTTON_MIDDLE));

    // Mouse wheel
    Vector2 mouseWheel = GetMouseWheelMoveV();
    io.AddMouseWheelEvent(mouseWheel.x, mouseWheel.y);

    // Keyboard modifiers
    io.AddKeyEvent(ImGuiMod_Ctrl,  IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
    io.AddKeyEvent(ImGuiMod_Shift, IsKeyDown(KEY_LEFT_SHIFT)   || IsKeyDown(KEY_RIGHT_SHIFT));
    io.AddKeyEvent(ImGuiMod_Alt,   IsKeyDown(KEY_LEFT_ALT)     || IsKeyDown(KEY_RIGHT_ALT));
    io.AddKeyEvent(ImGuiMod_Super, IsKeyDown(KEY_LEFT_SUPER)   || IsKeyDown(KEY_RIGHT_SUPER));

    // Keyboard keys — only send events on state change to avoid flooding
    for (int i = 0; i < g_KeyMapSize; i++)
    {
        bool down = IsKeyDown(g_KeyMap[i].raylib);
        int idx = g_KeyMap[i].raylib;
        if (idx >= 0 && idx < 512)
        {
            if (down != g_PrevKeyState[idx])
            {
                io.AddKeyEvent(g_KeyMap[i].imgui, down);
                g_PrevKeyState[idx] = down;
            }
        }
    }

    // Text input (Unicode characters)
    int codepoint;
    while ((codepoint = GetCharPressed()) != 0)
    {
        io.AddInputCharacter((unsigned int)codepoint);
    }

    // Update mouse cursor
    UpdateMouseCursor();

    ImGui::NewFrame();
}

void rlImGuiEnd()
{
    ImGui::Render();
    RenderImDrawData(ImGui::GetDrawData());
}

void rlImGuiShutdown()
{
    CleanupFontTexture();
    // Note: we do NOT destroy the ImGui context here — the caller manages it.
}

void rlImGuiReloadFonts()
{
    CleanupFontTexture();
    BuildFontTexture();
}
