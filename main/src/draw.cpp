#include "bindings.hpp"
#include "engine.hpp"
#include <raylib.h>

namespace BindingsDraw
{

    Color currentColor = WHITE;
    bool currentScreenSpace = true;

    struct TextCommand
    {
        String *text;
        int x, y, size;
        Color color;
    };

    struct RectangleCommand
    {
        int x, y, width, height;
        bool fill;
        Color color;
    };

    struct CircleCommand
    {
        int centerX, centerY, radius;
        bool fill;
        Color color;
    };

    struct LineCommand
    {
        int x1, y1, x2, y2;
        Color color;
    };

    struct DrawCommand
    {

        enum Type
        {
            LINE,
            POINT,
            TEXT,
            CIRCLE,
            RECTANGLE
        } type;

        union
        {
            LineCommand line;
            TextCommand text;
            CircleCommand circle;
            RectangleCommand rectangle;
        };
    };

    static Vector<DrawCommand> screenCommands;
 


    void RenderScreenCommands()
    {
        
        for (size_t i = 0; i < screenCommands.size(); i++)
        {
            DrawCommand &cmd = screenCommands[i];
            switch (cmd.type)
            {
            case DrawCommand::LINE:
                DrawLine(cmd.line.x1, cmd.line.y1, cmd.line.x2, cmd.line.y2, cmd.line.color);
                break;
            case DrawCommand::POINT:
                DrawPixel(cmd.line.x1, cmd.line.y1, cmd.line.color);
                break;
            case DrawCommand::TEXT:
                DrawText(cmd.text.text->chars(), cmd.text.x, cmd.text.y, cmd.text.size, cmd.text.color);
                break;
            case DrawCommand::CIRCLE:
                if (cmd.circle.fill)
                    DrawCircle(cmd.circle.centerX, cmd.circle.centerY, cmd.circle.radius, cmd.circle.color);
                else
                    DrawCircleLines(cmd.circle.centerX, cmd.circle.centerY, cmd.circle.radius, cmd.circle.color);
                break;
            case DrawCommand::RECTANGLE:
                if (cmd.rectangle.fill)
                    DrawRectangle(cmd.rectangle.x, cmd.rectangle.y, cmd.rectangle.width, cmd.rectangle.height, cmd.rectangle.color);
                else
                    DrawRectangleLines(cmd.rectangle.x, cmd.rectangle.y, cmd.rectangle.width, cmd.rectangle.height, cmd.rectangle.color);
                break;
            }
        }
     
    
    }

    void resetDrawCommands()
    {
        screenCommands.clear();
         
    }

   

     void addPointCommand(int x, int y, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::POINT;
            cmd.line = {x, y, x, y, currentColor};
            screenCommands.push(cmd);
        }
        else
        {
            DrawPixel(x, y, currentColor);
        }
    }

     void addLineCommand(int x1, int y1, int x2, int y2, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::LINE;
            cmd.line = {x1, y1, x2, y2, currentColor};
            screenCommands.push(cmd);
        }
        else
        {
            DrawLine(x1, y1, x2, y2, currentColor);
        }
    }

     void addTextCommand(String *text, int x, int y, int size, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::TEXT;
            cmd.text = {text, x, y, size, currentColor};
            screenCommands.push(cmd);
        }
        else
        {
            DrawText(text->chars(), x, y, size, currentColor);
        }
    }

    void addCircleCommand(int centerX, int centerY, int radius, bool fill, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::CIRCLE;
            cmd.circle = {centerX, centerY, radius, fill, currentColor};
            screenCommands.push(cmd);
        }
        else
        {
            if (fill)
                DrawCircle(centerX, centerY, radius, currentColor);
            else
                DrawCircleLines(centerX, centerY, radius, currentColor);
        }
    }

     void addRectangleCommand(int x, int y, int width, int height, bool fill, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::RECTANGLE;
            cmd.rectangle = {x, y, width, height, fill, currentColor};
            screenCommands.push(cmd);
        }
        else
        {
            if (fill)
                DrawRectangle(x, y, width, height, currentColor);
            else
                DrawRectangleLines(x, y, width, height, currentColor);
        }
    }

    static int native_line(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("line expects 4 arguments (x1, y1, x2, y2)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber())
        {
            Error("line expects 4 number arguments (x1, y1, x2, y2)");
            return 0;
        }

        int x1 = (int)args[0].asNumber();
        int y1 = (int)args[1].asNumber();
        int x2 = (int)args[2].asNumber();
        int y2 = (int)args[3].asNumber();

        addLineCommand(x1, y1, x2, y2, currentScreenSpace);

        return 0;
    }

    static int native_point(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("point expects 2 arguments (x, y)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("point expects 2 number arguments (x, y)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        addPointCommand(x, y, currentScreenSpace);

        return 0;
    }

    static int native_text(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("text expects 4 arguments (text, x, y, size)");
            return 0;
        }
        if (!args[0].isString() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber())
        {
            Error("text expects 1 string and 3 number arguments (text, x, y, size)");
            return 0;
        }

        String *text = args[0].asString();
        int x = (int)args[1].asNumber();
        int y = (int)args[2].asNumber();
        int size = (int)args[3].asNumber();
        addTextCommand(text, x, y, size, currentScreenSpace);

        return 0;
    }

    static int native_circle(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("circle expects 4 arguments (centerX, centerY, radius, fill)");
            return 0;
        }

        int centerX = (int)args[0].asNumber();
        int centerY = (int)args[1].asNumber();
        int radius = (int)args[2].asNumber();
        int fill = (int)args[3].asNumber();

        addCircleCommand(centerX, centerY, radius, fill != 0, currentScreenSpace);

        

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

        addRectangleCommand(x, y, width, height, fill != 0, currentScreenSpace);
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

    static int native_set_screen_space(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("set_screen_space expects 1 argument (enabled)");
            return 0;
        }
        if (!args[0].isBool())
        {
            Error("set_screen_space expects a boolean argument (enabled)");
            return 0;
        }
        currentScreenSpace = args[0].asBool();
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

    static void vector2_ctor(Interpreter *vm, void *buffer, int argc, Value *args)
    {
        Vector2 *vec = (Vector2 *)buffer;
        vec->x = args[0].asNumber();
        vec->y = args[1].asNumber();
    }

    void registerVector2(Interpreter &vm)
    {
        auto *vec2 = vm.registerNativeStruct(
            "Vec2",
            sizeof(Vector2),
            vector2_ctor,
            nullptr);

        vm.addStructField(vec2, "x", offsetof(Vector2, x), FieldType::FLOAT);
        vm.addStructField(vec2, "y", offsetof(Vector2, y), FieldType::FLOAT);
    }

    void registerAll(Interpreter &vm)
    {

        vm.registerNative("draw_line", native_line, 4);
        vm.registerNative("draw_circle", native_circle, 4);
        vm.registerNative("draw_point", native_point, 2);
        vm.registerNative("draw_text", native_text, 4);
        vm.registerNative("draw_rectangle", native_rectangle, 5);
        vm.registerNative("set_color", native_set_color, 3);
        vm.registerNative("set_alpha", native_set_alpha, 1);
        vm.registerNative("set_screen_space", native_set_screen_space, 1);

        vm.registerNative("start_fade", native_start_fade, 2);
        vm.registerNative("is_fade_complete", native_is_fade_complete, 0);
        vm.registerNative("get_fade_progress", native_get_fade_progress, 0);

        vm.registerNative("fade_in", native_fade_in, 1);
        vm.registerNative("fade_out", native_fade_out, 1);
        registerColor(vm);
        registerVector2(vm);
    }
}
