/**
 * @file main.c
 * @author Aritra Mondal
 * @copyright 2024
 */

#include "chip8.h"
#include "config.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

// ticks
LARGE_INTEGER t0;
LARGE_INTEGER t1;

// ticks per second
LARGE_INTEGER frequency;

#else
#include <sys/time.h>

// time intervals
struct timeval t0;
struct timeval t1;

#endif

double dt; // time required for one emulation cycle

extern uint8_t display[64][32];
extern uint8_t memory[4096];

extern struct config config;

uint8_t keymap[] = { SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e,
    SDLK_a, SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v };

extern bool key[16];

int main(int argc, char* argv[])
{
    config.scale_factor = 15;
    config.scr_width = 64;
    config.scr_height = 32;

    config.bg_r = 0;
    config.bg_g = 0;
    config.bg_b = 0;
    config.bg_a = SDL_ALPHA_OPAQUE;

    config.fg_r = 255;
    config.fg_g = 255;
    config.fg_b = 255;
    config.fg_a = SDL_ALPHA_OPAQUE;

    bool is_window_open = true;
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window
        = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, config.scr_width * config.scale_factor,
            config.scr_height * config.scale_factor, 0);

    SDL_Renderer* renderer
        = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    FILE* game_dat;

    SDL_Event event;

    if (argc < 2) {
        SDL_Surface* surface = SDL_LoadBMP("./assets/empty.bmp");

        SDL_RenderClear(renderer);
        SDL_Texture* img_texture
            = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, img_texture, NULL, NULL);

        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

        bool is_file_dropped = false;
        while (!is_file_dropped) {
            while (SDL_PollEvent(&event) > 0) {
                if (event.type == SDL_DROPFILE) {
                    char* filename = event.drop.file;
                    game_dat = fopen(filename, "rb");
                    is_file_dropped = true;
                }
                if (event.type == SDL_QUIT) {
                    SDL_Quit();
                    return EXIT_SUCCESS;
                }
            }
            SDL_RenderPresent(renderer);
        }

        SDL_DestroyTexture(img_texture);
        SDL_FreeSurface(surface);

    } else
        game_dat = fopen(argv[1], "rb");

    if (!game_dat) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "File cannot be opened!",
            "The file provided was either not present "
            "or sufficient permissions were not given!",
            window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    fread(&memory[0x200], sizeof(uint8_t), 0xFFF - 0x200, game_dat);
    fclose(game_dat);

    SDL_Rect* rect = (SDL_Rect*)(malloc(sizeof(SDL_Rect)));

    rect->w = config.scale_factor;
    rect->h = config.scale_factor;
    rect->x = 0;
    rect->y = 0;

    init();

    while (is_window_open) {
        SDL_SetRenderDrawColor(
            renderer, config.bg_r, config.bg_g, config.bg_b, config.bg_a);
        SDL_RenderClear(renderer);

        for (int x = 0; x < config.scr_width; ++x) {
            for (int y = 0; y < config.scr_height; ++y) {
                if (display[x][y]) {
                    rect->x = x * config.scale_factor;
                    rect->y = y * config.scale_factor;
                    SDL_SetRenderDrawColor(renderer, config.fg_r, config.fg_g,
                        config.fg_b, config.fg_a);
                    SDL_RenderFillRect(renderer, rect);

                    SDL_SetRenderDrawColor(renderer, config.bg_r, config.bg_g,
                        config.bg_b, config.bg_a);
                    SDL_RenderDrawRect(renderer, rect);
                }
            }
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event) > 0) {
            if (event.type == SDL_QUIT)
                is_window_open = false;

            else if (event.type == SDL_KEYDOWN) {
                for (int i = 0; i < 16; ++i)
                    if (keymap[i] == event.key.keysym.sym)
                        key[i] = 1;

                if (event.key.keysym.sym == SDLK_ESCAPE)
                    is_window_open = false;

            } else if (event.type == SDL_KEYUP)
                for (int i = 0; i < 16; ++i)
                    if (keymap[i] == event.key.keysym.sym)
                        key[i] = 0;
        }

        #ifdef _WIN32
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&t0);
        #else
            gettimeofday(&t0, NULL);
        #endif

        fetch();
        decode_exec();

        #ifdef _WIN32
            QueryPerformanceCounter(&t1);
            dt = (t1.QuadPart - t0.QuadPart) * 1000 / frequency.QuadPart;
        #else
            gettimeofday(&t1, NULL);
            dt = (t1.tv_sec - t0.tv_sec) * 1000;
            dt += (t1.tv_usec - t0.tv_usec) / 1000;
        #endif

        SDL_Delay(2 > dt ? 2 - dt : 0);
    }

    free(rect);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
