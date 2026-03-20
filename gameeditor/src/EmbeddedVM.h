#pragma once
// EmbeddedVM — runs the BuLang VM in-process inside the game editor.
// When running, the VM takes over the entire window for rendering.
// Press Escape to stop and return to the editor.

#include <string>
#include <raylib.h>

class Interpreter;

/// Possible states for the embedded VM session
enum class EmbeddedVMState
{
    Idle,       ///< No script loaded
    Running,    ///< VM processes are alive and ticking each frame
    Error       ///< Last run ended with an error
};

/// Manages an in-process BuLang VM session.
/// When running, the game renders directly to the window.
class EmbeddedVM
{
public:
    EmbeddedVM();
    ~EmbeddedVM();

    /// Start executing `sourceCode` inside the VM.
    /// `projectDir` is used to resolve include paths.
    /// Returns true on success. Check `getError()` on failure.
    bool start(const std::string& sourceCode, const std::string& projectDir);

    /// Run one game frame: update + render directly to the window.
    /// Returns false if the VM has stopped (all processes ended or error).
    bool frame();

    /// Stop the VM and free all resources.
    void stop();

    /// Current state
    EmbeddedVMState getState() const { return m_state; }

    /// Error message from the last failed start/run
    const std::string& getError() const { return m_error; }

private:
    void initVM(const std::string& projectDir);
    void shutdownVM();
    void saveWindowState();
    void restoreWindowState();
    void applyScriptWindowSettings();

    EmbeddedVMState m_state = EmbeddedVMState::Idle;
    std::string m_error;

    Interpreter* m_vm = nullptr;
    bool m_sceneInited = false;
    bool m_soundInited = false;

    // Saved editor window state (restored on stop)
    int m_savedWidth = 0;
    int m_savedHeight = 0;
    std::string m_savedTitle;
};
