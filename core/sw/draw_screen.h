#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdint.h>

// ----- Structs ----- //
// all info needed to draw a singal
typedef struct {
    const uint8_t* samples;
    int count;
    // uint16_t color;  // bring back this feature later if I think of a better way to include it
    int enabled;
    char label[5];
} Channel;

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame(const int lanes);
void draw_digital_waveform(const uint8_t* samples, const int count, int x0, int y0, int w, int h, uint16_t color);
void draw_signals(const Channel* channels, const int lanes);

#endif