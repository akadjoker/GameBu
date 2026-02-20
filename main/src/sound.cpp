#include "sound.hpp"
#include "engine.hpp"
#include <raylib.h>
#include <vector>
#include <string>

// SoundLib for short sound effects
struct SoundData
{
    Sound sound;
    int id;
    char name[MAXNAME];
};

struct SoundLib
{
    std::vector<SoundData> sounds;

    int load(const char *name, const char *soundPath)
    {
        for (const auto &s : sounds)
        {
            if (strcmp(s.name, name) == 0)
                return s.id;
        }

        Sound sound = LoadSound(soundPath);
        if (sound.frameCount == 0)
            return -1;

        SoundData data;
        data.sound = sound;
        data.id = (int)sounds.size();
        strncpy(data.name, name, MAXNAME - 1);
        data.name[MAXNAME - 1] = '\0';
        sounds.push_back(data);
        return data.id;
    }

    Sound *getSound(int id)
    {
        if (id < 0 || id >= (int)sounds.size())
            return nullptr;
        return &sounds[id].sound;
    }

    void play(int id, float volume, float pitch)
    {
        Sound *s = getSound(id);
        if (s)
        {
            SetSoundVolume(*s, volume);
            SetSoundPitch(*s, pitch);
            PlaySound(*s);
        }
    }

    void stop(int id)
    {
        Sound *s = getSound(id);
        if (s)
            StopSound(*s);
    }

    void pause(int id)
    {
        Sound *s = getSound(id);
        if (s)
            PauseSound(*s);
    }

    void resume(int id)
    {
        Sound *s = getSound(id);
        if (s)
            ResumeSound(*s);
    }

    bool isSoundPlaying(int id)
    {
        Sound *s = getSound(id);
        return s ? IsSoundPlaying(*s) : false;
    }

    void destroy()
    {
        for (auto &s : sounds)
            UnloadSound(s.sound);
        sounds.clear();
    }
};

// MusicLib for streaming music
struct MusicData
{
    Music music;
    int id;
    char name[MAXNAME];
};

struct MusicLib
{
    std::vector<MusicData> musics;

    int load(const char *name, const char *musicPath)
    {
        for (const auto &m : musics)
        {
            if (strcmp(m.name, name) == 0)
                return m.id;
        }

        Music music = LoadMusicStream(musicPath);
        if (!IsMusicReady(music))
            return -1;

        MusicData data;
        data.music = music;
        data.id = (int)musics.size();
        strncpy(data.name, name, MAXNAME - 1);
        data.name[MAXNAME - 1] = '\0';
        musics.push_back(data);
        return data.id;
    }

    MusicData *getMusicData(int id)
    {
        if (id < 0 || id >= (int)musics.size())
            return nullptr;
        return &musics[id];
    }

    void updateStreams()
    {
        for (auto &m : musics)
        {
            if (IsMusicStreamPlaying(m.music))
            {
                UpdateMusicStream(m.music);
            }
        }
    }

    void destroy()
    {
        for (auto &m : musics)
            UnloadMusicStream(m.music);
        musics.clear();
    }
};

static SoundLib gSoundLib;
static MusicLib gMusicLib;

namespace BindingsSound
{
    // --- Sound Functions ---
    int native_load_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isString())
        {
            Error("load_sound expects 1 string argument (path)");
            vm->pushInt(-1);
            return 1;
        }
        const char *path = args[0].asStringChars();
        int soundId = gSoundLib.load(GetFileNameWithoutExt(path), path);
        if (soundId < 0)
        {
            Error("Failed to load sound from path: %s", path);
            vm->pushInt(-1);
            return 1;
        }
        vm->pushInt(soundId);
        return 1;
    }

    int native_play_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount < 1 || argCount > 3 || !args[0].isInt())
        {
            Error("play_sound expects (soundId, [volume=1.0], [pitch=1.0])");
            return 0;
        }
        int soundId = args[0].asInt();
        float volume = (argCount > 1) ? (float)args[1].asNumber() : 1.0f;
        float pitch = (argCount > 2) ? (float)args[2].asNumber() : 1.0f;
        gSoundLib.play(soundId, volume, pitch);
        return 0;
    }

    int native_stop_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("stop_sound expects 1 int argument (soundId)"); return 0; }
        gSoundLib.stop(args[0].asInt());
        return 0;
    }

    int native_is_sound_playing(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("is_sound_playing expects 1 int argument (soundId)"); vm->pushBool(false); return 1; }
        vm->pushBool(gSoundLib.isSoundPlaying(args[0].asInt()));
        return 1;
    }

    int native_pause_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("pause_sound expects 1 int argument (soundId)"); return 0; }
        gSoundLib.pause(args[0].asInt());
        return 0;
    }

    int native_resume_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("resume_sound expects 1 int argument (soundId)"); return 0; }
        gSoundLib.resume(args[0].asInt());
        return 0;
    }

    // --- Music Functions ---
    int native_load_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isString()) { Error("load_music expects 1 string argument (path)"); vm->pushInt(-1); return 1; }
        const char *path = args[0].asStringChars();
        int musicId = gMusicLib.load(GetFileNameWithoutExt(path), path);
        if (musicId < 0) { Error("Failed to load music from path: %s", path); vm->pushInt(-1); return 1; }
        vm->pushInt(musicId);
        return 1;
    }

    int native_play_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("play_music expects 1 int argument (musicId)"); return 0; }
        MusicData *data = gMusicLib.getMusicData(args[0].asInt());
        if (data) PlayMusicStream(data->music);
        return 0;
    }

    int native_stop_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("stop_music expects 1 int argument (musicId)"); return 0; }
        MusicData *data = gMusicLib.getMusicData(args[0].asInt());
        if (data) StopMusicStream(data->music);
        return 0;
    }

    int native_pause_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("pause_music expects 1 int argument (musicId)"); return 0; }
        MusicData *data = gMusicLib.getMusicData(args[0].asInt());
        if (data) PauseMusicStream(data->music);
        return 0;
    }

    int native_resume_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("resume_music expects 1 int argument (musicId)"); return 0; }
        MusicData *data = gMusicLib.getMusicData(args[0].asInt());
        if (data) ResumeMusicStream(data->music);
        return 0;
    }

    int native_set_music_volume(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2 || !args[0].isInt() || !args[1].isNumber()) { Error("set_music_volume expects (musicId, volume)"); return 0; }
        MusicData *data = gMusicLib.getMusicData(args[0].asInt());
        if (data) SetMusicVolume(data->music, (float)args[1].asNumber());
        return 0;
    }

    int native_is_music_playing(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("is_music_playing expects 1 int argument (musicId)"); vm->pushBool(false); return 1; }
        MusicData *data = gMusicLib.getMusicData(args[0].asInt());
        vm->pushBool(data ? IsMusicStreamPlaying(data->music) : false);
        return 1;
    }

    int native_update_music_streams(Interpreter *vm, int argCount, Value *args)
    {
        (void)vm; (void)argCount; (void)args;
        gMusicLib.updateStreams();
        return 0;
    }

    void registerAll(Interpreter &vm)
    {
        vm.registerNative("load_sound", native_load_sound, 1);
        vm.registerNative("play_sound", native_play_sound, -1);
        vm.registerNative("stop_sound", native_stop_sound, 1);
        vm.registerNative("is_sound_playing", native_is_sound_playing, 1);
        vm.registerNative("pause_sound", native_pause_sound, 1);
        vm.registerNative("resume_sound", native_resume_sound, 1);

        vm.registerNative("load_music", native_load_music, 1);
        vm.registerNative("play_music", native_play_music, 1);
        vm.registerNative("stop_music", native_stop_music, 1);
        vm.registerNative("pause_music", native_pause_music, 1);
        vm.registerNative("resume_music", native_resume_music, 1);
        vm.registerNative("set_music_volume", native_set_music_volume, 2);
        vm.registerNative("is_music_playing", native_is_music_playing, 1);
        vm.registerNative("update_music_streams", native_update_music_streams, 0);
    }

    void updateMusicStreams() { gMusicLib.updateStreams(); }
    void shutdown() { gSoundLib.destroy(); gMusicLib.destroy(); }
}