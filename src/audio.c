/**
 * @file audio.c
 * @author Aritra Mondal
 * @copyright 2024
 */

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

static SDL_AudioDeviceID audio_dev;
static SDL_AudioSpec audio_spec;

void init_audio()
{
    audio_spec.freq = 44100;
    audio_spec.format = AUDIO_S16;
    audio_spec.channels = 1; // mono speaker
    audio_spec.samples = 1024;
    audio_spec.callback = NULL;

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
}

void close_audio() { SDL_CloseAudioDevice(audio_dev); }

void beep_start()
{
    for (int i = 0; i < audio_spec.freq / 10; i++) {
        // square wave amp = 3200
        // frequency ~= 440
        int16_t sample = (i / 50) % 2 ? 3200 : -3200;

        SDL_QueueAudio(audio_dev, &sample, sizeof sample);
    }
    SDL_PauseAudioDevice(audio_dev, 0);
}
void beep_stop() { SDL_PauseAudioDevice(audio_dev, 1); }
