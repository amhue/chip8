/**
 * @file display.c
 * @author Aritra Mondal
 * @copyright 2024
 */

#include "config.h"
#include <SDL2/SDL.h>

extern uint8_t display[64][32];

void re_paint(SDL_Rect* rect, SDL_Renderer* renderer)
{
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
    SDL_RenderClear(renderer);
}
