#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdint.h>

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame();
void draw_digital_waveform(const uint8_t* samples, int count, int x0, int y0, int w, int h, uint16_t color);

#endif