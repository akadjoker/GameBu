#pragma once

// rlImGui - Dear ImGui backend for raylib
// Replaces imgui_impl_sdl2 + imgui_impl_opengl3

// Setup Dear ImGui with raylib backend.
// Call AFTER ImGui::CreateContext() and font configuration.
// The raylib window must already be initialized via InitWindow().
bool rlImGuiSetup(bool darkTheme = true);

// Begin a new ImGui frame (updates input, calls ImGui::NewFrame).
// Call at the start of your main loop, AFTER BeginDrawing().
void rlImGuiBegin();

// End the ImGui frame and render draw data via rlgl.
// Call BEFORE EndDrawing().
void rlImGuiEnd();

// Shutdown the backend and release resources.
void rlImGuiShutdown();

// Rebuild the font texture atlas (call after adding/changing fonts).
void rlImGuiReloadFonts();
