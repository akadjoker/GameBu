#include "bindings.hpp"
#include "engine.hpp"
#include <raylib.h>

namespace BindingsDraw
{

    Color currentColor = WHITE;

    static int native_line(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 5)
        {
            Error("line expects 5 arguments (x1, y1, x2, y2, color)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() ||
            !args[2].isNumber() || !args[3].isNumber() ||
            !args[4].isNumber())
        {
            Error("line expects 5 number arguments (x1, y1, x2, y2, color)");
            return 0;
        }

        int x1 = (int)args[0].asNumber();
        int y1 = (int)args[1].asNumber();
        int x2 = (int)args[2].asNumber();
        int y2 = (int)args[3].asNumber();
        int colorValue = (int)args[4].asNumber();

       
        DrawLine(x1, y1, x2, y2, currentColor);
        return 0;
    }

    static int native_circle(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("circle expects 4 arguments (centerX, centerY, radius, fill)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() ||
            !args[2].isNumber() || !args[3].isNumber())
        {
            Error("circle expects 4 number arguments (centerX, centerY, radius, fill)");
            return 0;
        }

        int centerX = (int)args[0].asNumber();
        int centerY = (int)args[1].asNumber();
        int radius = (int)args[2].asNumber();
        int fill = (int)args[3].asNumber();
        
        if (fill)
            DrawCircle(centerX, centerY, radius, currentColor);
        else
            DrawCircleLines(centerX, centerY, radius, currentColor);
        return 0;
    }

    static int native_rectangle(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 5)
        {
            Error("rectangle expects 5 arguments (x, y, width, height, fill)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() ||
            !args[2].isNumber() || !args[3].isNumber() ||
            !args[4].isNumber())
        {
            Error("rectangle expects 5 number arguments (x, y, width, height, fill)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        int width = (int)args[2].asNumber();
        int height = (int)args[3].asNumber();
        int fill = (int)args[4].asNumber();

        if (fill)
            DrawRectangle(x, y, width, height, currentColor);
        else
            DrawRectangleLines(x, y, width, height, currentColor);
        return 0;
    }

    static int native_set_color(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("set_color expects 3 arguments (red, green, blue)");
            return 0;
        }
         

        if (!args[0].isNumber() || !args[1].isNumber() || !args[2].isNumber())
        {
            Error("set_color expects 3 number arguments (red, green, blue)");
            return 0;
        }
        int red = (int)args[0].asNumber();
        int green = (int)args[1].asNumber();
        int blue = (int)args[2].asNumber();
        currentColor.r = red;
        currentColor.g = green;
        currentColor.b = blue;
        return 0;
    }

    static int native_set_alpha(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("set_alpha expects 1 argument (alpha)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("set_alpha expects a number argument (alpha)");
            return 0;
        }
        int alpha = (int)args[0].asNumber();
        currentColor.a = alpha;
        return 0;
    }


    int native_start_fade(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("start_fade expects 2 arguments (targetAlpha, speed )");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("start_fade expects 2 number arguments (targetAlpha, speed)");
            return 0;
        }

        float targetAlpha = (float)args[0].asNumber();
        float speed = (float)args[1].asNumber();
      

        StartFade(targetAlpha, speed, currentColor);
        return 0;
    }

    int native_is_fade_complete(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("is_fade_complete expects 0 arguments");
            vm->pushBool(false);
            return 1;
        }

        vm->pushBool(IsFadeComplete());
        return 1;
    }

    int native_get_fade_progress(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_fade_progress expects 0 arguments");
            vm->pushDouble(0.0);
            return 1;
        }

        float progress = GetFadeProgress();
        vm->pushDouble(progress);
        return 1;
    }

    int native_fade_in(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("fade_in expects 1 argument (speed)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("fade_in expects a number argument (speed)");
            return 0;
        }

        float speed = (float)args[0].asNumber();
        FadeIn(speed, currentColor);
        return 0;
    }

    int native_fade_out(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("fade_out expects 1 argument (speed)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("fade_out expects a number argument (speed)");
            return 0;
        }

        float speed = (float)args[0].asNumber();
        FadeOut(speed, currentColor);
        return 0;
    }

    static void color_ctor(Interpreter *vm, void *buffer, int argc, Value *args)
    {
        Color *v = (Color *)buffer;
        v->r = (uint8)args[0].asNumber();
        v->g = (uint8)args[1].asNumber();
        v->b = (uint8)args[2].asNumber();
        v->a = (uint8)args[3].asNumber();
    }

     void registerColor(Interpreter &vm)
    {
        auto *color = vm.registerNativeStruct(
            "Color",
            sizeof(Color),
            color_ctor,
            nullptr);

        vm.addStructField(color, "r", offsetof(Color, r), FieldType::BYTE);
        vm.addStructField(color, "g", offsetof(Color, g), FieldType::BYTE);
        vm.addStructField(color, "b", offsetof(Color, b), FieldType::BYTE);
        vm.addStructField(color, "a", offsetof(Color, a), FieldType::BYTE);
    }

    void registerAll(Interpreter &vm)
    {
        
        vm.registerNative("line", native_line, 5);
        vm.registerNative("circle", native_circle, 4);
        vm.registerNative("rectangle", native_rectangle, 5);
        vm.registerNative("set_color", native_set_color, 3);
        vm.registerNative("set_alpha", native_set_alpha, 1);

        vm.registerNative("start_fade", native_start_fade, 2);
        vm.registerNative("is_fade_complete", native_is_fade_complete, 0);
        vm.registerNative("get_fade_progress", native_get_fade_progress, 0);

        vm.registerNative("fade_in", native_fade_in, 1);
        vm.registerNative("fade_out", native_fade_out, 1);
        registerColor(vm);
    }
}
