#include "bindings.hpp"
#include "engine.hpp"
#include <raylib.h>
#include <vector>

namespace BindingsDraw
{

    Color currentColor = WHITE;
    bool currentScreenSpace = true;

    static std::vector<Font> loadedFonts;

    struct TextCommand
    {
        String *text;
        int x, y, size;
        Color color;
    };

    struct FontCommand
    {
        String *text;
        int x, y, size;
        float spacing;
        int fontId;
        Color color;
    };

    struct FontCommandRotated
    {
        String *text;
        int x, y, size;
        Vector2 origin;
        float rotation;
        float spacing;
        int fontId;
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
            FONT,
            FONTROTATED,
            CIRCLE,
            RECTANGLE
        } type;

        union
        {
            LineCommand line;
            TextCommand text;
            FontCommand font;
            FontCommandRotated fontRotated;
            CircleCommand circle;
            RectangleCommand rectangle;
        };
    };

    static Vector<DrawCommand> screenCommands;

    static void draw_font(String *text, int x, int y, int size, float spacing, Color color, int fontId)
    {

        if (fontId < 0 || fontId >= loadedFonts.size())
        {

            DrawTextEx(GetFontDefault(), text->chars(), {(float)x, (float)y}, (float)size, spacing, color);
            return;
        }
        Font &font = loadedFonts[fontId];
        DrawTextEx(font, text->chars(), {(float)x, (float)y}, (float)size, spacing, color);
    }

    static void draw_font_rotate(String *text, int x, int y, int size, float rotation, float spacing, float pivot_x, float pivot_y, Color color, int fontId)
    {

        if (fontId < 0 || fontId >= loadedFonts.size())
        {
            Warning("draw_font_rotate: invalid fontId %d, using default font", fontId);
            DrawTextPro(GetFontDefault(), text->chars(), {(float)x, (float)y}, {pivot_x, pivot_y}, rotation, (float)size, spacing, color);
            return;
        }
        Font &font = loadedFonts[fontId];
        DrawTextPro(font, text->chars(), {(float)x, (float)y}, {pivot_x, pivot_y}, rotation, (float)size, spacing, color);
    }

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
            case DrawCommand::FONT:
                draw_font(cmd.font.text, cmd.font.x, cmd.font.y, cmd.font.size, cmd.font.spacing, cmd.font.color, cmd.font.fontId);
                break;
            case DrawCommand::FONTROTATED:
                draw_font_rotate(cmd.fontRotated.text, cmd.fontRotated.x, cmd.fontRotated.y, cmd.fontRotated.size, cmd.fontRotated.rotation, cmd.fontRotated.spacing, cmd.fontRotated.origin.x, cmd.fontRotated.origin.y, cmd.fontRotated.color, 0);
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

    void addFontCommand(String *text, int x, int y, int size, float spacing, Color color, int fontId, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::FONT;
            cmd.font = {text, x, y, size, spacing, fontId, color};
            screenCommands.push(cmd);
        }
        else
        {
            draw_font(text, x, y, size, spacing, color, fontId);
        }
    }

    void addFontRotateCommand(String *text, int x, int y, int size, float rotation, float spacing, float pivot_x, float pivot_y, Color color, int fontId, bool screenSpace)
    {
        if (screenSpace)
        {
            DrawCommand cmd;
            cmd.type = DrawCommand::FONTROTATED;
            cmd.fontRotated = {text, x, y, size, rotation, spacing, pivot_x, pivot_y, fontId, color};
            screenCommands.push(cmd);
        }
        else
        {
            draw_font_rotate(text, x, y, size, rotation, spacing, pivot_x, pivot_y, color, fontId);
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

    static int native_draw_font(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 6)
        {
            Error("draw_font expects 6 arguments (text, x, y, size, spacing, fontId)");
            return 0;
        }

        String *text = args[0].asString();
        int x = (int)args[1].asNumber();
        int y = (int)args[2].asNumber();
        int size = (int)args[3].asNumber();
        float spacing = (float)args[4].asNumber();
        int fontId = args[5].asInt();

        addFontCommand(text, x, y, size, spacing, currentColor, fontId, currentScreenSpace);

        return 0;
    }

    static int native_draw_font_rotate(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 9)
        {
            Error("draw_font_rotate expects 9 arguments (text, x, y, size, rotation, spacing, pivot_x, pivot_y, fontId)");
            return 0;
        }

        String *text = args[0].asString();
        int x = (int)args[1].asNumber();
        int y = (int)args[2].asNumber();
        int size = (int)args[3].asNumber();
        float rotation = (float)args[4].asNumber();
        float spacing = (float)args[5].asNumber();
        float pivot_x = (float)args[6].asNumber();
        float pivot_y = (float)args[7].asNumber();
        int fontId = args[8].asInt();

        addFontRotateCommand(text, x, y, size, rotation, spacing, pivot_x, pivot_y, currentColor, fontId, currentScreenSpace);

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
         

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        int width = (int)args[2].asNumber();
        int height = (int)args[3].asNumber();
        bool fill =  args[4].asBool();

        addRectangleCommand(x, y, width, height, fill , currentScreenSpace);
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

    static int native_load_font(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isString())
        {
            Error("load_font expects 1 string argument (path)");
            vm->pushInt(-1);
            return 1;
        }

        const char *path = args[0].asStringChars();
        Font font = LoadFont(path);
        if (font.texture.id <= 0)
        {
            Error("Failed to load font from path: %s", path);
            vm->pushInt(-1);
            return 1;
        }

        loadedFonts.push_back(font);
        vm->pushInt((int)loadedFonts.size() - 1);
        return 1;
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

    int native_draw_fps(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("draw_fps expects 2 arguments (x, y)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        DrawFPS(x, y);
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
        vm.registerNative("draw_font", native_draw_font, 6);
        vm.registerNative("draw_font_rotate", native_draw_font_rotate, 9);

        vm.registerNative("draw_rectangle", native_rectangle, 5);

        vm.registerNative("set_color", native_set_color, 3);
        vm.registerNative("set_alpha", native_set_alpha, 1);
        vm.registerNative("set_screen_space", native_set_screen_space, 1);

        vm.registerNative("draw_fps", native_draw_fps, 2);

        vm.registerNative("start_fade", native_start_fade, 2);
        vm.registerNative("is_fade_complete", native_is_fade_complete, 0);

        vm.registerNative("fade_in", native_fade_in, 1);
        vm.registerNative("fade_out", native_fade_out, 1);
        vm.registerNative("get_fade_progress", native_get_fade_progress, 0);

        vm.registerNative("load_font", native_load_font, 1);
        registerColor(vm);
        registerVector2(vm);
    }

    void unloadFonts()
    {
        for (size_t i = 0; i < loadedFonts.size(); i++)
        {
            UnloadFont(loadedFonts[i]);
        }
        loadedFonts.clear();
    }

}
