#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

struct config {
    uint32_t scale_factor;
    uint32_t scr_width;
    uint32_t scr_height;
    
    // background rgba
    uint8_t bg_r;
    uint8_t bg_g;
    uint8_t bg_b;
    uint8_t bg_a;

    // foreground rgba
    uint8_t fg_r;
    uint8_t fg_g;
    uint8_t fg_b;
    uint8_t fg_a;
};

extern struct config config;

#endif
