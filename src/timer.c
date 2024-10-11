/**
 * @file timer.c
 * @author Aritra Mondal
 * @copyright 2024
 */

#include "audio.h"
#include <stdint.h>

struct timer {
    uint8_t dt; // delay timer
    uint8_t st; // sound timer
} timer;

void update_timers()
{
    if (timer.dt > 0) {
        --timer.dt;
    }

    if (timer.st > 0) {
        --timer.st;
        if (timer.st > 0) {
            beep_start();
        } else {
            beep_stop();
        }
    }
}
