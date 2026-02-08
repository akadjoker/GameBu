#include "bindings.hpp"
#include "engine.hpp"
#include "render.hpp"
#include <raylib.h>
#include <vector>

extern GraphLib gGraphLib;
extern Scene gScene;

namespace BindingsDraw
{

    Color currentColor = WHITE;
    int layer = 0;
    bool screen= false;

    static std::vector<Font> loadedFonts;

    static void draw_font_impl(String *text, int x, int y, int size, float spacing, Color color, int fontId)
    {
        if (fontId < 0 || fontId >= (int)loadedFonts.size())
        {
            DrawTextEx(GetFontDefault(), text->chars(), {(float)x, (float)y}, (float)size, spacing, color);
            return;
        }
        Font &font = loadedFonts[fontId];
        DrawTextEx(font, text->chars(), {(float)x, (float)y}, (float)size, spacing, color);
    }

    static void draw_font_rotate_impl(String *text, int x, int y, int size, float rotation, float spacing, float pivot_x, float pivot_y, Color color, int fontId)
    {
        
        if (fontId < 0 || fontId >= (int)loadedFonts.size())
        {
            DrawTextPro(GetFontDefault(), text->chars(), {(float)x, (float)y}, {pivot_x, pivot_y}, rotation, (float)size, spacing, color);
            return;
        }
        Font &font = loadedFonts[fontId];
        DrawTextPro(font, text->chars(), {(float)x, (float)y}, {pivot_x, pivot_y}, rotation, (float)size, spacing, color);
    }

    // === Native functions ===

    static int native_set_draw_layer(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("set_draw_layer expects 1 argument (layer)");
            return 0;
        }
       
        layer = args[0].asInt();
        if (layer < 0 || layer >= MAX_LAYERS)
        {
            Error("set_draw_layer: layer out of bounds");
            layer = 0;
        }
        return 0;
    }

    static int native_set_draw_screen(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("set_draw_screen expects 1 argument (bool)");
            return 0;
        }
       
        screen = args[0].asBool();
        return 0;
    }

    static int native_line(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("draw_line expects 4 arguments (x1, y1, x2, y2)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber())
        {
            Error("draw_line expects 4 number arguments (x1, y1, x2, y2)");
            return 0;
        }
        

        int x1 = (int)args[0].asNumber();
        int y1 = (int)args[1].asNumber();
        int x2 = (int)args[2].asNumber();
        int y2 = (int)args[3].asNumber();
            if (!screen)
            {
                Layer &l = gScene.layers[layer];
                x1 -= l.scroll_x;
                y1 -= l.scroll_y;
                x2 -= l.scroll_x;
                y2 -= l.scroll_y;
                
            }
        DrawLine(x1, y1, x2, y2, currentColor);
        return 0;
    }

    static int native_point(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("draw_point expects 2 arguments (x, y)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("draw_point expects 2 number arguments (x, y)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        if (!screen)
        {
            Layer &l = gScene.layers[layer];
            x -= l.scroll_x;
            y -= l.scroll_y;
        }
        DrawPixel(x, y, currentColor);
        return 0;
    }

    static int native_text(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("draw_text expects 4 arguments (text, x, y, size)");
            return 0;
        }
        if (!args[0].isString() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber())
        {
            Error("draw_text expects 1 string and 3 number arguments (text, x, y, size)");
            return 0;
        }

        String *text = args[0].asString();
        int x = (int)args[1].asNumber();
        int y = (int)args[2].asNumber();
        int size = (int)args[3].asNumber();
            if (!screen)
            {
                Layer &l = gScene.layers[layer];
                x -= l.scroll_x;
                y -= l.scroll_y;
            }
        DrawText(text->chars(), x, y, size, currentColor);
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

        if (!screen)
        {
            Layer &l = gScene.layers[layer];
            x -= l.scroll_x;
            y -= l.scroll_y;
        }

        int size = (int)args[3].asNumber();
        float spacing = (float)args[4].asNumber();
        int fontId = args[5].asInt();
        draw_font_impl(text, x, y, size, spacing, currentColor, fontId);
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

        if (!screen)
        {
            Layer &l = gScene.layers[layer];
            x -= l.scroll_x;
            y -= l.scroll_y;
        }


        int size = (int)args[3].asNumber();
        float rotation = (float)args[4].asNumber();
        float spacing = (float)args[5].asNumber();
        float pivot_x = (float)args[6].asNumber();
        float pivot_y = (float)args[7].asNumber();
        int fontId = args[8].asInt();
        draw_font_rotate_impl(text, x, y, size, rotation, spacing, pivot_x, pivot_y, currentColor, fontId);
        return 0;
    }

    static int native_circle(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4)
        {
            Error("draw_circle expects 4 arguments (centerX, centerY, radius, fill)");
            return 0;
        }

        int centerX = (int)args[0].asNumber();
        int centerY = (int)args[1].asNumber();

        if (!screen)
        {
            Layer &l = gScene.layers[layer];
            centerX -= l.scroll_x;
            centerY -= l.scroll_y;

            
        }

        int radius = (int)args[2].asNumber();
        bool fill = (int)args[3].asNumber() != 0;

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
            Error("draw_rectangle expects 5 arguments (x, y, width, height, fill)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        int width = (int)args[2].asNumber();
        int height = (int)args[3].asNumber();
        bool fill = args[4].asBool();
        if (!screen)
        {
            Layer &l = gScene.layers[layer];
            x -= l.scroll_x;
            y -= l.scroll_y;
        }

        if (fill)
            DrawRectangle(x, y, width, height, currentColor);
        else
            DrawRectangleLines(x, y, width, height, currentColor);
        return 0;
    }

    static int native_triangle(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 7)
        {
            Error("draw_triangle expects 7 arguments (x1, y1, x2, y2, x3, y3, fill)");
            return 0;
        }

        Vector2 v1 = {(float)args[0].asNumber(), (float)args[1].asNumber()};
        Vector2 v2 = {(float)args[2].asNumber(), (float)args[3].asNumber()};
        Vector2 v3 = {(float)args[4].asNumber(), (float)args[5].asNumber()};
            if (!screen)
            {
                Layer &l = gScene.layers[layer];
                v1.x -= l.scroll_x;
                v1.y -= l.scroll_y;
                v2.x -= l.scroll_x;
                v2.y -= l.scroll_y;
                v3.x -= l.scroll_x;
                v3.y -= l.scroll_y;
            }
        bool fill = args[6].asBool();

        if (fill)
            DrawTriangle(v1, v2, v3, currentColor);
        else
            DrawTriangleLines(v1, v2, v3, currentColor);
        return 0;
    }

    static int native_draw_graph(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("draw_graph expects 3 arguments (graphId, x, y)");
            return 0;
        }

        int graphId = (int)args[0].asNumber();
        float x = (float)args[1].asNumber();
        float y = (float)args[2].asNumber();

        if (!screen)        
        {
            Layer &l = gScene.layers[layer];
            x -= l.scroll_x;
            y -= l.scroll_y;
        }

        Graph *graph = gGraphLib.getGraph(graphId);
        if (!graph) return 0;
        Texture2D *tex = gGraphLib.getTexture(graph->texture);
        if (!tex) return 0;
        DrawTextureRec(*tex, graph->clip, {x, y}, currentColor);
        return 0;
    }

    static int native_draw_graph_ex(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 8)
        {
            Error("draw_graph_ex expects 8 arguments (graphId, x, y, angle, sizeX, sizeY, flipX, flipY)");
            return 0;
        }

        int graphId = (int)args[0].asNumber();
        float x = (float)args[1].asNumber();
        float y = (float)args[2].asNumber();
        float angle = (float)args[3].asNumber();
        float sizeX = (float)args[4].asNumber();
        float sizeY = (float)args[5].asNumber();
        bool flipX = args[6].asBool();
        bool flipY = args[7].asBool();

        if (!screen)
        {
            Layer &l = gScene.layers[layer];
            x -= l.scroll_x;
            y -= l.scroll_y;
        }

        Graph *graph = gGraphLib.getGraph(graphId);
        if (!graph) return 0;
        Texture2D *tex = gGraphLib.getTexture(graph->texture);
        if (!tex) return 0;

        if (angle == 0 && sizeX == 100 && sizeY == 100 && !flipX && !flipY)
        {
            DrawTextureRec(*tex, graph->clip, {x, y}, currentColor);
        }
        else
        {
            int pivotX = (int)(graph->clip.width / 2);
            int pivotY = (int)(graph->clip.height / 2);
            RenderTexturePivotRotateSizeXY(*tex, pivotX, pivotY, graph->clip,
                                          x, y, angle, sizeX, sizeY,
                                          flipX, flipY, currentColor);
        }
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
        currentColor.r = (int)args[0].asNumber();
        currentColor.g = (int)args[1].asNumber();
        currentColor.b = (int)args[2].asNumber();
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
        currentColor.a = (int)args[0].asNumber();
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
            Error("start_fade expects 2 arguments (targetAlpha, speed)");
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
        vm->pushDouble(GetFadeProgress());
        return 1;
    }

    int native_fade_in(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber())
        {
            Error("fade_in expects 1 number argument (speed)");
            return 0;
        }
        FadeIn((float)args[0].asNumber(), currentColor);
        return 0;
    }

    int native_fade_out(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber())
        {
            Error("fade_out expects 1 number argument (speed)");
            return 0;
        }
        FadeOut((float)args[0].asNumber(), currentColor);
        return 0;
    }

    int native_draw_fps(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("draw_fps expects 2 arguments (x, y)");
            return 0;
        }
        DrawFPS((int)args[0].asNumber(), (int)args[1].asNumber());
        return 0;
    }

    static int native_get_text_width(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2 || !args[0].isString() || !args[1].isNumber())
        {
            Error("get_text_width expects 2 arguments (text, size)");
            vm->pushInt(0);
            return 1;
        }
        vm->pushInt(MeasureText(args[0].asStringChars(), (int)args[1].asNumber()));
        return 1;
    }

    static int native_get_font_text_width(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 4 || !args[0].isString() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber())
        {
            Error("get_font_text_width expects 4 arguments (text, size, spacing, fontId)");
            vm->pushInt(0);
            return 1;
        }

        int fontId = (int)args[3].asNumber();
        Font font = GetFontDefault();
        if (fontId >= 0 && fontId < (int)loadedFonts.size())
            font = loadedFonts[fontId];

        Vector2 measure = MeasureTextEx(font, args[0].asStringChars(), (float)args[1].asNumber(), (float)args[2].asNumber());
        vm->pushInt((int)measure.x);
        return 1;
    }

    static int native_get_graph_width(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber())
        {
            Error("get_graph_width expects 1 argument (graphId)");
            vm->pushInt(0);
            return 1;
        }
        Graph *g = gGraphLib.getGraph((int)args[0].asNumber());
        vm->pushInt(g ? g->width : 0);
        return 1;
    }

    static int native_get_graph_height(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber())
        {
            Error("get_graph_height expects 1 argument (graphId)");
            vm->pushInt(0);
            return 1;
        }
        Graph *g = gGraphLib.getGraph((int)args[0].asNumber());
        vm->pushInt(g ? g->height : 0);
        return 1;
    }

    // === Structs ===

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

    // === Registration ===

    void registerAll(Interpreter &vm)
    {
        vm.registerNative("draw_line", native_line, 4);
        vm.registerNative("draw_circle", native_circle, 4);
        vm.registerNative("draw_point", native_point, 2);
        vm.registerNative("draw_text", native_text, 4);
        vm.registerNative("draw_font", native_draw_font, 6);
        vm.registerNative("draw_font_rotate", native_draw_font_rotate, 9);
        vm.registerNative("draw_rectangle", native_rectangle, 5);
        vm.registerNative("draw_triangle", native_triangle, 7);
        vm.registerNative("draw_graph", native_draw_graph, 3);
        vm.registerNative("draw_graph_ex", native_draw_graph_ex, 8);

        vm.registerNative("set_draw_layer", native_set_draw_layer, 1);
        vm.registerNative("set_draw_screen", native_set_draw_screen, 1);

        vm.registerNative("get_text_width", native_get_text_width, 2);
        vm.registerNative("get_font_text_width", native_get_font_text_width, 4);
        vm.registerNative("get_graph_width", native_get_graph_width, 1);
        vm.registerNative("get_graph_height", native_get_graph_height, 1);

        vm.registerNative("set_color", native_set_color, 3);
        vm.registerNative("set_alpha", native_set_alpha, 1);

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
