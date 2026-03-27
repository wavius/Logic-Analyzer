#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // for snprintf

#include "visualizer_logic.h"

#define TOTAL_SIGNALS 16
#define BUFFER_SIZE 4096

// ----- Structs ----- //
// all info needed to draw a singal
typedef struct {
    uint8_t* samples;
    int count;
    // uint16_t color;  // bring back this feature later if I think of a better way to include it
    bool enabled;
    char label[8];
    uint16_t color;
} Channel;

extern uint32_t current_page;  // declaration only

extern uint8_t channel_buffers[TOTAL_SIGNALS][BUFFER_SIZE];

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame(const Channel* channels, const int lanes);
void draw_signals(const ZoomState* state, const Channel* channels, const int signals_per_page);
void switch_ui_page();
void draw_ui_page(const Channel* channels, const ZoomState* state, uint32_t trigger_position);
void draw_digital_waveform(const uint8_t* samples, const int count, int x0, int y0, int w, int h, uint16_t color);
void channels_init(Channel* channels, const int size);

#endif