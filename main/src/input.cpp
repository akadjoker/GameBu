#include "bindings.hpp"
#include <raylib.h>

namespace BindingsInput
{

    //   RLAPI bool IsKeyPressed(int key);                             // Check if a key has been pressed once
    // RLAPI bool IsKeyPressedRepeat(int key);                       // Check if a key has been pressed again (Only PLATFORM_DESKTOP)
    // RLAPI bool IsKeyDown(int key);                                // Check if a key is being pressed
    // RLAPI bool IsKeyReleased(int key);                            // Check if a key has been released once
    // RLAPI bool IsKeyUp(int key);                                  // Check if a key is NOT being pressed
    // RLAPI int GetKeyPressed(void);                                // Get key pressed (keycode), call it multiple times for keys queued, returns 0 when the queue is empty
    // RLAPI int GetCharPressed(void);                               // Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty
    // RLAPI void SetExitKey(int key);

    static int native_key_down(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("key_down expects 1 argument (key code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("key_down expects 1 argument (key code)");
            return 0;
        }

        int keyCode = (int)args[0].asNumber();
        bool isDown = IsKeyDown(keyCode);
        vm->pushBool(isDown);
        return 1;
    }

    static int native_key_pressed(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("key_pressed expects 1 argument (key code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("key_pressed expects 1 argument (key code)");
            return 0;
        }

        int keyCode = (int)args[0].asNumber();
        bool isPressed = IsKeyPressed(keyCode);
        vm->pushBool(isPressed);
        return 1;
    }

    static int native_key_released(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("key_released expects 1 argument (key code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("key_released expects 1 argument (key code)");
            return 0;
        }

        int keyCode = (int)args[0].asNumber();
        bool isReleased = IsKeyReleased(keyCode);
        vm->pushBool(isReleased);
        return 1;
    }

    static int native_key_up(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("key_up expects 1 argument (key code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("key_up expects 1 argument (key code)");
            return 0;
        }

        int keyCode = (int)args[0].asNumber();
        bool isUp = IsKeyUp(keyCode);
        vm->pushBool(isUp);
        return 1;
    }

    static int native_get_key_pressed(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_key_pressed expects no arguments");
            return 0;
        }

        int keyCode = GetKeyPressed();
        vm->pushInt(keyCode);
        return 1;
    }

    static int native_get_char_pressed(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_char_pressed expects no arguments");
            return 0;
        }

        int charCode = GetCharPressed();
        vm->pushInt(charCode);
        return 1;
    }

    // Input-related functions: mouse
    // RLAPI bool IsMouseButtonPressed(int button);                  // Check if a mouse button has been pressed once
    // RLAPI bool IsMouseButtonDown(int button);                     // Check if a mouse button is being pressed
    // RLAPI bool IsMouseButtonReleased(int button);                 // Check if a mouse button has been released once
    // RLAPI bool IsMouseButtonUp(int button);                       // Check if a mouse button is NOT being pressed
    // RLAPI int GetMouseX(void);                                    // Get mouse position X
    // RLAPI int GetMouseY(void);                                    // Get mouse position Y
    // RLAPI Vector2 GetMousePosition(void);                         // Get mouse position XY
    // RLAPI Vector2 GetMouseDelta(void);                            // Get mouse delta between frames
    // RLAPI void SetMousePosition(int x, int y);                    // Set mouse position XY
    // RLAPI void SetMouseOffset(int offsetX, int offsetY);          // Set mouse offset
    // RLAPI void SetMouseScale(float scaleX, float scaleY);         // Set mouse scaling
    // RLAPI float GetMouseWheelMove(void);                          // Get mouse wheel movement for X or Y, whichever is larger
    // RLAPI Vector2 GetMouseWheelMoveV(void);                       // Get mouse wheel movement for both X and Y
    // RLAPI void SetMouseCursor(int cursor);                        // Set mouse cursor

    static int mousePressed(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("mouse_pressed expects 1 argument (button code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("mouse_pressed expects 1 argument (button code)");
            return 0;
        }

        int buttonCode = (int)args[0].asNumber();
        bool isPressed = IsMouseButtonPressed(buttonCode);
        vm->pushBool(isPressed);
        return 1;
    }

    static int mouseDown(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("mouse_down expects 1 argument (button code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("mouse_down expects 1 argument (button code)");
            return 0;
        }

        int buttonCode = (int)args[0].asNumber();
        bool isDown = IsMouseButtonDown(buttonCode);
        vm->pushBool(isDown);
        return 1;
    }

    static int mouseReleased(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("mouse_released expects 1 argument (button code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("mouse_released expects 1 argument (button code)");
            return 0;
        }

        int buttonCode = (int)args[0].asNumber();
        bool isReleased = IsMouseButtonReleased(buttonCode);
        vm->pushBool(isReleased);
        return 1;
    }

    static int mouseUp(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("mouse_up expects 1 argument (button code)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("mouse_up expects 1 argument (button code)");
            return 0;
        }

        int buttonCode = (int)args[0].asNumber();
        bool isUp = IsMouseButtonUp(buttonCode);
        vm->pushBool(isUp);
        return 1;
    }

    static int getMouseX(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_mouse_x expects no arguments");
            return 0;
        }

        int mouseX = GetMouseX();
        vm->pushInt(mouseX);
        return 1;
    }

    static int getMouseY(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_mouse_y expects no arguments");
            return 0;
        }

        int mouseY = GetMouseY();
        vm->pushInt(mouseY);
        return 1;
    }

    static int getMousePosition(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_mouse_position expects no arguments");
            return 0;
        }

        Vector2 pos = GetMousePosition();
        vm->pushFloat(pos.x);
        vm->pushFloat(pos.y);
       
        return 2;
    }

    static int getMouseDelta(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_mouse_delta expects no arguments");
            return 0;
        }

        Vector2 delta = GetMouseDelta();
        vm->pushFloat(delta.x);
        vm->pushFloat(delta.y);
       
        return 2;
    }

    static int setMousePosition(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_mouse_position expects 2 arguments (x, y)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("set_mouse_position expects 2 arguments (x, y)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        SetMousePosition(x, y);
        return 0;
    }

    static int setMouseOffset(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_mouse_offset expects 2 arguments (offsetX, offsetY)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("set_mouse_offset expects 2 arguments (offsetX, offsetY)");
            return 0;
        }

        int offsetX = (int)args[0].asNumber();
        int offsetY = (int)args[1].asNumber();
        SetMouseOffset(offsetX, offsetY);
        return 0;
    }

    static int setMouseScale(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_mouse_scale expects 2 arguments (scaleX, scaleY)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("set_mouse_scale expects 2 arguments (scaleX, scaleY)");
            return 0;
        }

        float scaleX = (float)args[0].asNumber();
        float scaleY = (float)args[1].asNumber();
        SetMouseScale(scaleX, scaleY);
        return 0;
    }

    void registerAll(Interpreter &vm)
    {

        vm.registerNative("key_down", native_key_down, 1);
        vm.registerNative("key_pressed", native_key_pressed, 1);
        vm.registerNative("key_released", native_key_released, 1);
        vm.registerNative("key_up", native_key_up, 1);
        vm.registerNative("get_key_pressed", native_get_key_pressed, 0);
        vm.registerNative("get_char_pressed", native_get_char_pressed, 0);

        vm.registerNative("mouse_pressed", mousePressed, 1);
        vm.registerNative("mouse_down", mouseDown, 1);
        vm.registerNative("mouse_released", mouseReleased, 1);
        vm.registerNative("mouse_up", mouseUp, 1);
        
        vm.registerNative("get_mouse_x", getMouseX, 0);
        vm.registerNative("get_mouse_y", getMouseY, 0);
        vm.registerNative("get_mouse_position", getMousePosition, 0);
        vm.registerNative("get_mouse_delta", getMouseDelta, 0);
        vm.registerNative("set_mouse_position", setMousePosition, 2);
        vm.registerNative("set_mouse_offset", setMouseOffset, 2);
        vm.registerNative("set_mouse_scale", setMouseScale, 2);
    }

} // namespace BindingsInput