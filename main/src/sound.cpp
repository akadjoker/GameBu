#include "sound.hpp"
#include "engine.hpp"
#include "bugl_audio.hpp"
#include <vector>
#include <string>

extern bugl::audio::Engine gAudioEngine;

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
        int soundId = gAudioEngine.createSfx(path);
        if (soundId <= 0)
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
        if (argCount < 1 || argCount > 4 || !args[0].isInt())
        {
            Error("play_sound expects (soundId, [volume=1.0], [pitch=1.0], [pan=0.0])");
            vm->pushInt(0);
            return 1;
        }
        int soundId = args[0].asInt();
        float volume = (argCount > 1) ? (float)args[1].asNumber() : 1.0f;
        float pitch = (argCount > 2) ? (float)args[2].asNumber() : 1.0f;
        float pan = (argCount > 3) ? (float)args[3].asNumber() : 0.0f;
        int handle = gAudioEngine.playSfx(soundId, volume, pitch, pan);
        vm->pushInt(handle);
        return 1;
    }

    int native_stop_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("stop_sound expects 1 int argument (handle)"); return 0; }
        gAudioEngine.stop(args[0].asInt());
        return 0;
    }

    int native_is_sound_playing(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("is_sound_playing expects 1 int argument (handle)"); vm->pushBool(false); return 1; }
        vm->pushBool(gAudioEngine.isPlaying(args[0].asInt()));
        return 1;
    }

    int native_pause_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("pause_sound expects 1 int argument (handle)"); return 0; }
        gAudioEngine.pause(args[0].asInt());
        return 0;
    }

    int native_resume_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("resume_sound expects 1 int argument (handle)"); return 0; }
        gAudioEngine.resume(args[0].asInt());
        return 0;
    }

    // --- Music Functions ---
    int native_load_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isString()) { Error("load_music expects 1 string argument (path)"); vm->pushInt(-1); return 1; }
        const char *path = args[0].asStringChars();
        int musicId = gAudioEngine.createMusic(path);
        if (musicId <= 0) { Error("Failed to load music from path: %s", path); vm->pushInt(-1); return 1; }
        vm->pushInt(musicId);
        return 1;
    }

    int native_play_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount < 1 || argCount > 3 || !args[0].isInt()) { Error("play_music expects (musicId, [loop=true], [volume=1.0])"); vm->pushInt(0); return 1; }
        int musicId = args[0].asInt();
        bool loop = (argCount > 1) ? args[1].asBool() : true;
        float volume = (argCount > 2) ? (float)args[2].asNumber() : 1.0f;
        int handle = gAudioEngine.playMusic(musicId, loop, volume);
        vm->pushInt(handle);
        return 1;
    }

    int native_stop_music(Interpreter *vm, int argCount, Value *args)
    {
        (void)argCount; (void)args;
        gAudioEngine.stopMusic();
        return 0;
    }

    int native_pause_music(Interpreter *vm, int argCount, Value *args)
    {
        // pause uses the music handle internally
        if (argCount != 1 || !args[0].isInt()) { Error("pause_music expects 1 int argument (handle)"); return 0; }
        gAudioEngine.pause(args[0].asInt());
        return 0;
    }

    int native_resume_music(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("resume_music expects 1 int argument (handle)"); return 0; }
        gAudioEngine.resume(args[0].asInt());
        return 0;
    }

    int native_set_music_volume(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber()) { Error("set_music_volume expects (volume)"); return 0; }
        gAudioEngine.setMusicVolume((float)args[0].asNumber());
        return 0;
    }

    int native_is_music_playing(Interpreter *vm, int argCount, Value *args)
    {
        (void)argCount; (void)args;
        vm->pushBool(gAudioEngine.isMusicPlaying());
        return 1;
    }

    // --- Volume controls ---
    int native_set_sound_volume(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2 || !args[0].isInt() || !args[1].isNumber()) { Error("set_sound_volume expects (handle, volume)"); return 0; }
        gAudioEngine.setVolume(args[0].asInt(), (float)args[1].asNumber());
        return 0;
    }

    int native_set_sound_pitch(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2 || !args[0].isInt() || !args[1].isNumber()) { Error("set_sound_pitch expects (handle, pitch)"); return 0; }
        gAudioEngine.setPitch(args[0].asInt(), (float)args[1].asNumber());
        return 0;
    }

    int native_set_sound_pan(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 2 || !args[0].isInt() || !args[1].isNumber()) { Error("set_sound_pan expects (handle, pan)"); return 0; }
        gAudioEngine.setPan(args[0].asInt(), (float)args[1].asNumber());
        return 0;
    }

    // --- Master / SFX / Music group volumes ---
    int native_set_master_volume(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber()) { Error("set_master_volume expects (volume)"); return 0; }
        gAudioEngine.setMasterVolume((float)args[0].asNumber());
        return 0;
    }

    int native_set_sfx_volume(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isNumber()) { Error("set_sfx_volume expects (volume)"); return 0; }
        gAudioEngine.setSfxVolume((float)args[0].asNumber());
        return 0;
    }

    // --- Procedural Audio ---
    int native_create_waveform(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 3 || !args[0].isInt() || !args[1].isNumber() || !args[2].isNumber())
        {
            Error("create_waveform expects (type, amplitude, frequency)");
            vm->pushInt(-1);
            return 1;
        }
        int type = args[0].asInt();
        float amp = (float)args[1].asNumber();
        float freq = (float)args[2].asNumber();
        int id = gAudioEngine.createWaveform(type, amp, freq);
        vm->pushInt(id);
        return 1;
    }

    int native_create_noise(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount < 1 || argCount > 3 || !args[0].isInt())
        {
            Error("create_noise expects (type, [seed=0], [amplitude=0.5])");
            vm->pushInt(-1);
            return 1;
        }
        int type = args[0].asInt();
        int seed = (argCount > 1) ? args[1].asInt() : 0;
        float amp = (argCount > 2) ? (float)args[2].asNumber() : 0.5f;
        int id = gAudioEngine.createNoise(type, seed, amp);
        vm->pushInt(id);
        return 1;
    }

    // --- Effects ---
    int native_enable_sfx_delay(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount < 1) { Error("enable_sfx_delay expects (enable, [decay=0.5])"); return 0; }
        bool enable = args[0].asBool();
        float decay = (argCount > 1) ? (float)args[1].asNumber() : 0.5f;
        gAudioEngine.enableSfxDelay(enable, decay);
        return 0;
    }

    int native_enable_music_lowpass(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount < 1) { Error("enable_music_lowpass expects (enable, [cutoff=1000])"); return 0; }
        bool enable = args[0].asBool();
        float cutoff = (argCount > 1) ? (float)args[1].asNumber() : 1000.0f;
        gAudioEngine.enableMusicLowPass(enable, cutoff);
        return 0;
    }

    int native_stop_all_sounds(Interpreter *vm, int argCount, Value *args)
    {
        (void)vm; (void)argCount; (void)args;
        gAudioEngine.stopAll();
        return 0;
    }

    int native_remove_sound(Interpreter *vm, int argCount, Value *args)
    {
        if (argCount != 1 || !args[0].isInt()) { Error("remove_sound expects (soundId)"); vm->pushBool(false); return 1; }
        vm->pushBool(gAudioEngine.removeSound(args[0].asInt()));
        return 1;
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
        vm.registerNative("play_music", native_play_music, -1);
        vm.registerNative("stop_music", native_stop_music, 0);
        vm.registerNative("pause_music", native_pause_music, 1);
        vm.registerNative("resume_music", native_resume_music, 1);
        vm.registerNative("set_music_volume", native_set_music_volume, 1);
        vm.registerNative("is_music_playing", native_is_music_playing, 0);

        vm.registerNative("set_sound_volume", native_set_sound_volume, 2);
        vm.registerNative("set_sound_pitch", native_set_sound_pitch, 2);
        vm.registerNative("set_sound_pan", native_set_sound_pan, 2);
        vm.registerNative("set_master_volume", native_set_master_volume, 1);
        vm.registerNative("set_sfx_volume", native_set_sfx_volume, 1);

        // Procedural audio
        vm.registerNative("create_waveform", native_create_waveform, 3);
        vm.registerNative("create_noise", native_create_noise, -1);

        // Effects
        vm.registerNative("enable_sfx_delay", native_enable_sfx_delay, -1);
        vm.registerNative("enable_music_lowpass", native_enable_music_lowpass, -1);

        // Utility
        vm.registerNative("stop_all_sounds", native_stop_all_sounds, 0);
        vm.registerNative("remove_sound", native_remove_sound, 1);
    }

    void updateMusicStreams() { gAudioEngine.update(); }
    void shutdown() { /* handled by DestroySound() */ }
}