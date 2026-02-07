#include "bindings.hpp"
#include "engine.hpp"
extern GraphLib gGraphLib;
extern SoundLib gSoundLib;
extern Scene gScene;

namespace Bindings
{

    void CollisionCallback(Entity *a, Entity *b, void *userdata)
    {
    }

    static int native_load_graph(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("load_graph expects 1 string argument (path)");
            return 0;
        }
        if (!args[0].isString())
        {
            Error("load_graph expects 1 string argument (path)");
            return 0;
        }

        const char *path = args[0].asStringChars();
        const char *name = GetFileNameWithoutExt(path);
        int graphId = gGraphLib.load(name, path);
        if (graphId < 0)
        {
            Error("Failed to load graph: %s from path: %s", name, path);
            return 0;
        }

        vm->pushInt(graphId);

        return 1;
    }

    static int native_load_atlas(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("load_atlas expects 3 arguments (texturePath, countX, countY)");
            return 0;
        }
        const char *path = args[0].asStringChars();
        const char *name = GetFileNameWithoutExt(path);
        int count_x = (int)args[1].asNumber();
        int count_y = (int)args[2].asNumber();

        int graphId = gGraphLib.loadAtlas(name, path, count_x, count_y);
        if (graphId < 0)
        {
            Error("Failed to load atlas: %s from path: %s", name, path);
            return 0;
        }

        vm->pushInt(graphId);

        return 1;
    }

    static int native_load_subgraph(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 6)
        {
            Error("load_subgraph expects 6 arguments (parentId, name, x, y, width, height)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isString() || !args[2].isNumber() || !args[3].isNumber() || !args[4].isNumber() || !args[5].isNumber())
        {
            Error("load_subgraph expects 6 arguments (parentId, name, x, y, width, height)");
            return 0;
        }

        int parentId = (int)args[0].asNumber();
        const char *name = args[1].asStringChars();
        int x = (int)args[2].asNumber();
        int y = (int)args[3].asNumber();
        int width = (int)args[4].asNumber();
        int height = (int)args[5].asNumber();

        int graphId = gGraphLib.addSubGraph(parentId, name, x, y, width, height);
        if (graphId < 0)
        {
            Error("Failed to load subgraph: %s from parent ID: %d", name, parentId);
            return 0;
        }

        vm->pushInt(graphId);

        return 1;
    }

    static int native_save_graphics(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("save_graphics expects 1 string argument (filename)");
            return 0;
        }
        if (!args[0].isString())
        {
            Error("save_graphics expects 1 string argument (filename)");
            return 0;
        }

        const char *filename = args[0].asStringChars();

        bool success = gGraphLib.savePak(filename);
        if (!success)
        {
            Error("Failed to save graphics to file: %s", filename);
            return 0;
        }

        return 0;
    }

    static int native_load_graphics(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("load_graphics expects 1 string argument (filename)");
            return 0;
        }
        if (!args[0].isString())
        {
            Error("load_graphics expects 1 string argument (filename)");
            return 0;
        }

        const char *filename = args[0].asStringChars();

        bool success = gGraphLib.loadPak(filename);
        if (!success)
        {
            Error("Failed to load graphics from file: %s", filename);
            return 0;
        }

        return 0;
    }

    static int native_init_collision(Interpreter *vm, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 4)
        {
            Error("init_collision expects 4 arguments (x, y, width, height)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() ||
            !args[2].isNumber() || !args[3].isNumber())
        {
            Error("init_collision expects 4 number arguments (x, y, width, height)");
            return 0;
        }

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        int width = (int)args[2].asNumber();
        int height = (int)args[3].asNumber();
        InitCollision(x, y, width, height, nullptr);
        return 0;
    }

    int native_set_graphics_pointer(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("set_graphics_pointer expects 3 arguments (graphics, x, y)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNumber() || !args[2].isNumber())
        {
            Error("set_graphics_pointer expects 3 arguments (graphics, x, y)");
            return 0;
        }
        int graphId = (int)args[0].asInt();
        float x = (float)args[1].asNumber();
        float y = (float)args[2].asNumber();

        Graph *g = gGraphLib.getGraph(graphId);
        if (!g)
        {
            Error("Invalid graph ID: %d", graphId);
            return 0;
        }

        g->points.push_back({x, y});

        return 0;
    }

    int native_proc(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("proc expects 1 argument (process id)");
            vm->pushNil();
            return 1;
        }
        if (!args[0].isNumber())
        {
            Error("proc expects 1 number argument (process id)");
            vm->pushNil();
            return 1;
        }

        uint32 id = (uint32)args[0].asNumber();

        Process *target = vm->findProcessById(id);
        if (!target)
        {
            vm->pushNil();
            return 1;
        }

        vm->push(vm->makeProcess(target->id));

        return 1;
    }

    int native_type(Interpreter *vm, int argCount, Value *args)
    {

        if (argCount != 1)
        {
            Error("type expects 1 argument (process id)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("type expects 1 number argument (process id)");
            return 0;
        }

        uint32 id = (uint32)args[0].asNumber();

        Process *target = vm->findProcessById(id);
        if (!target || !target->name)
        {
            vm->pushString("nil");
            return 1;
        }

        vm->pushString(target->name->chars());
        return 1;
    }
    int native_signal(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("signal expects 2 arguments (process id, signal name)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("signal expects 1 number argument (process id)");
            return 0;
        }
        if (!args[1].isInt())
        {
            Error("signal expects 1 int argument (signal type)");
            return 0;
        }

        uint32 id = (uint32)args[0].asNumber();
        int type = (int)args[1].asInt();

        if (type == 0) // KILL
        {
            Process *proc = vm->findProcessById(id);
            if (proc)
            {
                proc->state = FiberState::DEAD;
            }
        }
        else
        {
            Process *proc = vm->findProcessById(id);
            proc->signal = type;
        }

        return 0;
    }

    int native_exists(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber())
        {
            vm->pushBool(false);
            return 1;
        }

        uint32 id = (uint32)args[0].asNumber();
        Process *target = vm->findProcessById(id);
        vm->pushBool(target != nullptr);
        return 1;
    }

    int native_get_id(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isString())
        {
            vm->pushInt(-1);
            return 1;
        }

        const char *typeName = args[0].asStringChars();

        const auto &alive = vm->getAliveProcesses();
        for (size_t i = 0; i < alive.size(); i++)
        {
            Process *proc = alive[i];
            if (proc && proc->name && strcmp(proc->name->chars(), typeName) == 0)
            {
                vm->push(vm->makeProcess(proc->id));
                return 1;
            }
        }

        vm->pushInt(-1);
        return 1;
    }

    int native_load_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("load_sound expects 1 string argument (path)");
            return 0;
        }
        if (!args[0].isString())
        {
            Error("load_sound expects 1 string argument (path)");
            return 0;
        }

        const char *path = args[0].asStringChars();
        int soundId = gSoundLib.load(GetFileNameWithoutExt(path), path);
        if (soundId < 0)
        {
            Error("Failed to load sound from path: %s", path);
            return 0;
        }

        vm->pushInt(soundId);

        return 1;
    }

    int native_play_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("play_sound expects 3 arguments (soundId, volume, pitch)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNumber() || !args[2].isNumber())
        {
            Error("play_sound expects 3 arguments (soundId, volume, pitch)");
            return 0;
        }

        int soundId = (int)args[0].asInt();
        float volume = (float)args[1].asNumber();
        float pitch = (float)args[2].asNumber();

        gSoundLib.play(soundId, volume, pitch);

        return 0;
    }

    int native_stop_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("stop_sound expects 1 argument (soundId)");
            return 0;
        }
        if (!args[0].isInt())
        {
            Error("stop_sound expects 1 int argument (soundId)");
            return 0;
        }

        int soundId = (int)args[0].asInt();
        gSoundLib.stop(soundId);
        return 0;
    }

    int native_is_sound_playing(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("is_sound_playing expects 1 argument (soundId)");
            vm->pushBool(false);
            return 1;
        }
        if (!args[0].isInt())
        {
            Error("is_sound_playing expects 1 int argument (soundId)");
            vm->pushBool(false);
            return 1;
        }

        int soundId = (int)args[0].asInt();
        bool playing = gSoundLib.isSoundPlaying(soundId);
        vm->pushBool(playing);
        return 1;
    }

    int native_pause_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("pause_sound expects 1 argument (soundId)");
            return 0;
        }
        if (!args[0].isInt())
        {
            Error("pause_sound expects 1 int argument (soundId)");
            return 0;
        }

        int soundId = (int)args[0].asInt();
        gSoundLib.pause(soundId);
        return 0;
    }

    int native_resume_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("resume_sound expects 1 argument (soundId)");
            return 0;
        }
        if (!args[0].isInt())
        {
            Error("resume_sound expects 1 int argument (soundId)");
            return 0;
        }

        int soundId = (int)args[0].asInt();
        gSoundLib.resume(soundId);
        return 0;
    }

    int native_set_layer_mode(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_layer_mode expects 2 arguments (layer, mode)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt())
        {
            Error("set_layer_mode expects 2 int arguments (layer, mode)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int mode = (int)args[1].asInt();

        SetLayerMode(layer, mode);
        return 0;
    }

    int native_set_layer_scroll_factor(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("set_layer_scroll_factor expects 3 arguments (layer, x, y)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNumber() || !args[2].isNumber())
        {
            Error("set_layer_scroll_factor expects 3 arguments (layer, x, y)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        double x = args[1].asNumber();
        double y = args[2].asNumber();

        SetLayerScrollFactor(layer, x, y);
        return 0;
    }

    int native_set_layer_size(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 5)
        {
            Error("set_layer_size expects 5 arguments (layer, x, y, width, height)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt() || !args[2].isInt() || !args[3].isInt() || !args[4].isInt())
        {
            Error("set_layer_size expects 5 int arguments (layer, x, y, width, height)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int x = (int)args[1].asInt();
        int y = (int)args[2].asInt();
        int width = (int)args[3].asInt();
        int height = (int)args[4].asInt();

        SetLayerSize(layer, x, y, width, height);
        return 0;
    }

    int native_set_layer_back_graph(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_layer_back_graph expects 2 arguments (layer, graph)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt())
        {
            Error("set_layer_back_graph expects 2 int arguments (layer, graph)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int graph = (int)args[1].asInt();

        SetLayerBackGraph(layer, graph);
        return 0;
    }

    int native_set_layer_front_graph(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_layer_front_graph expects 2 arguments (layer, graph)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt())
        {
            Error("set_layer_front_graph expects 2 int arguments (layer, graph)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int graph = (int)args[1].asInt();

        SetLayerFrontGraph(layer, graph);
        return 0;
    }

    int native_set_scroll(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_scroll expects 2 arguments (x, y)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("set_scroll expects 2 number arguments (x, y)");
            return 0;
        }

        double x = args[0].asNumber();
        double y = args[1].asNumber();

        SetScroll(x, y);
        return 0;
    }

    int native_set_tile_map(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 7)
        {
            Error("set_tile_map expects 7 arguments (layer, map_width, map_height, tile_width, tile_height, columns, graph)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt() || !args[2].isInt() || !args[3].isInt() || !args[4].isInt() || !args[5].isInt() || !args[6].isInt())
        {
            Error("set_tile_map expects 7 int arguments (layer, map_width, map_height, tile_width, tile_height, columns, graph)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int map_width = (int)args[1].asInt();
        int map_height = (int)args[2].asInt();
        int tile_width = (int)args[3].asInt();
        int tile_height = (int)args[4].asInt();
        int columns = (int)args[5].asInt();
        int graph = (int)args[6].asInt();

        SetTileMap(layer, map_width, map_height, tile_width, tile_height, columns, graph);
        return 0;
    }

    int native_clear_tile_map(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("clear_tile_map expects 1 argument (layer)");
            return 0;
        }
        if (!args[0].isInt())
        {
            Error("clear_tile_map expects 1 int argument (layer)");
            return 0;
        }

        int layer = (int)args[0].asInt();

        return 0;
    }

    int native_set_tile_map_free(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_free expects 2 arguments (layer, free)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt())
        {
            Error("set_tile_map_free expects 2 int arguments (layer, free)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int free = (int)args[1].asInt();

        SetTileMapFree(layer, free);
        return 0;
    }

    int native_set_tile_map_solid(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_solid expects 2 arguments (layer, solid)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt())
        {
            Error("set_tile_map_solid expects 2 int arguments (layer, solid)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int solid = (int)args[1].asInt();

        SetTileMapSolid(layer, solid);
        return 0;
    }

    int native_set_tile_map_spacing(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_spacing expects 2 arguments (layer, spacing)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNumber())
        {
            Error("set_tile_map_spacing expects 1 int and 1 number argument (layer, spacing)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        double spacing = args[1].asNumber();

        SetTileMapSpacing(layer, spacing);
        return 0;
    }

    int native_set_tile_map_margin(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_margin expects 2 arguments (layer, margin)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNumber())
        {
            Error("set_tile_map_margin expects 1 int and 1 number argument (layer, margin)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        double margin = args[1].asNumber();

        SetTileMapMargin(layer, margin);
        return 0;
    }

    int native_set_tile_debug(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("set_tile_debug expects 3 arguments (layer, grid, ids)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isBool() || !args[2].isBool())
        {
            Error("set_tile_debug expects 1 int and 2 bool arguments (layer, grid, ids)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        bool grid = args[1].asBool();
        bool ids = args[2].asBool();

        SetTileMapDebug(layer, grid != 0, ids != 0);
        return 0;
    }
    int native_set_tile_map_color(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_color expects 2 arguments (layer, color)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNativeStructInstance())
        {
            Error("set_tile_map_color expects 2 int arguments (layer, color)");
            return 0;
        }

        auto *inst = args[1].asNativeStructInstance();
        Color *color = (Color *)inst->data;
        int layer = (int)args[0].asInt();
        SetTileMapColor(layer, *color);
        return 0;
    }

    int native_set_tile_map_mode(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_mode expects 2 arguments (layer, mode)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt())
        {
            Error("set_tile_map_mode expects 2 int arguments (layer, mode)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int mode = (int)args[1].asInt();

        SetTileMapMode(layer, mode);
        return 0;
    }

    int native_set_tile_map_iso_compression(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("set_tile_map_iso_compression expects 2 arguments (layer, compression)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isNumber())
        {
            Error("set_tile_map_iso_compression expects 1 int and 1 number argument (layer, compression)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        double compression = args[1].asNumber();

        SetTileMapIsoCompression(layer, compression);
        return 0;
    }

    int native_set_tile_map_tile(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount < 3 || argCount > 4)
        {
            Error("set_tile_map_tile expects 3 or 4 arguments (layer, x, y, tile, [solid])");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt() || !args[2].isInt() || !args[3].isInt())
        {
            Error("set_tile_map_tile expects 4 int arguments (layer, x, y, tile, [solid])");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int x = (int)args[1].asInt();
        int y = (int)args[2].asInt();
        int tile = (int)args[3].asInt();
        int solid = 1;
        if (argCount == 5 && args[4].isInt())
        {
            solid = (int)args[4].asInt();
        }

        SetTileMapTile(layer, x, y, tile, solid);
        return 0;
    }

    int native_get_tile_map_tile(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3)
        {
            Error("get_tile_map_tile expects 3 arguments (layer, x, y)");
            return 0;
        }
        if (!args[0].isInt() || !args[1].isInt() || !args[2].isInt())
        {
            Error("get_tile_map_tile expects 3 int arguments (layer, x, y)");
            return 0;
        }

        int layer = (int)args[0].asInt();
        int x = (int)args[1].asInt();
        int y = (int)args[2].asInt();

        int tile = GetTileMapTile(layer, x, y);
        vm->pushInt(tile);
        return 1;
    }

    int native_import_tmx(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("import_tilemap expects 1 string argument (filename)");
            vm->pushBool(false);
            return 1;
        }
        if (!args[0].isString())
        {
            Error("import_tilemap expects 1 string argument (filename)");
            vm->pushBool(false);
            return 1;
        }

        bool success = gScene.ImportTileMap(args[0].asStringChars());
        vm->pushBool(success);

        return 1;
    }

    int native_delta_time(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("delta_time expects no arguments");
            return 0;
        }

        vm->pushDouble(GetFrameTime());
        return 1;
    }

    int native_time(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("time expects no arguments");
            return 0;
        }

        vm->pushDouble(GetTime());
        return 1;
    }

    void registerAll(Interpreter &vm)
    {

        vm.registerNative("load_graph", native_load_graph, 1);
        vm.registerNative("load_atlas", native_load_atlas, 3);
        vm.registerNative("load_subgraph", native_load_subgraph, 6);
        vm.registerNative("save_graphics", native_save_graphics, 1);
        vm.registerNative("load_graphics", native_load_graphics, 1);
        vm.registerNative("set_graphics_point", native_set_graphics_pointer, 3);
        vm.registerNative("init_collision", native_init_collision, 4);
        vm.registerNative("type", native_type, 1);
        vm.registerNative("proc", native_proc, 1);
        vm.registerNative("signal", native_signal, 2);
        vm.registerNative("exists", native_exists, 1);
        vm.registerNative("get_id", native_get_id, 1);
        vm.registerNative("play_sound", native_play_sound, 3);
        vm.registerNative("stop_sound", native_stop_sound, 1);
        vm.registerNative("is_sound_playing", native_is_sound_playing, 1);
        vm.registerNative("pause_sound", native_pause_sound, 1);
        vm.registerNative("resume_sound", native_resume_sound, 1);
        vm.registerNative("set_layer_mode", native_set_layer_mode, 2);

        vm.registerNative("set_layer_scroll_factor", native_set_layer_scroll_factor, 3);
        vm.registerNative("set_layer_size", native_set_layer_size, 5);
        vm.registerNative("set_layer_back_graph", native_set_layer_back_graph, 2);
        vm.registerNative("set_layer_front_graph", native_set_layer_front_graph, 2);
        vm.registerNative("set_scroll", native_set_scroll, 2);
        vm.registerNative("set_tile_map", native_set_tile_map, 7);
        vm.registerNative("set_tile_map_spacing", native_set_tile_map_spacing, 2);
        vm.registerNative("set_tile_map_free", native_set_tile_map_free, 2);
        vm.registerNative("set_tile_map_solid", native_set_tile_map_solid, 2);
        vm.registerNative("set_tile_map_margin", native_set_tile_map_margin, 2);
        vm.registerNative("set_tile_map_mode", native_set_tile_map_mode, 2);
        vm.registerNative("set_tile_map_color", native_set_tile_map_color, 2);
        vm.registerNative("set_tile_debug", native_set_tile_debug, 3);
        vm.registerNative("set_tile_map_iso_compression", native_set_tile_map_iso_compression, 2);
        vm.registerNative("set_tile_map_tile", native_set_tile_map_tile, 4);
        vm.registerNative("get_tile_map_tile", native_get_tile_map_tile, 3);
        vm.registerNative("import_tilemap", native_import_tmx, 1);

        vm.registerNative("delta", native_delta_time, 0);
        vm.registerNative("time", native_time, 0);

        vm.addGlobal("SKILL", vm.makeInt(0));
        vm.addGlobal("SFREEZE", vm.makeInt(1));
        vm.addGlobal("SHIDE", vm.makeInt(2));
        vm.addGlobal("SSHOW", vm.makeInt(3));

        BindingsInput::registerAll(vm);
        BindingsProcess::registerAll(vm);
        BindingsDraw::registerAll(vm);
        BindingsParticles::registerAll(vm);
    }

} // namespace OgreBindings
