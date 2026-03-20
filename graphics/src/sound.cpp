#include "bugl_audio.hpp"
#include "engine.hpp"

bugl::audio::Engine gAudioEngine;

void InitSound()
{
    gAudioEngine.init();
}

void DestroySound()
{
    gAudioEngine.shutdown();
}

