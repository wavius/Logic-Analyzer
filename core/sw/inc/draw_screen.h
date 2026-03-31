#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

#include "constants.h"
#include "visualizer_logic.h"

// ----- Structs ----- //
// all info needed to draw a singal
typedef struct {
    uint8_t* samples;
    int count;
    bool enabled;
    char label[8];
    uint16_t color;
} Channel;

// ----- Global Variables ----- //
extern uint32_t current_page;  // declaration only (defined in draw_screen.c)

extern uint8_t channel_buffers[TOTAL_SIGNALS][BUFFER_SIZE];

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame(const Channel* channels, const int lanes);
void draw_signals(const ZoomState* state, const Channel* channels, const int signals_per_page);
void switch_ui_page();
void draw_ui_page(const Channel* channels, const ZoomState* state, uint32_t trigger_position);
void draw_digital_waveform(const uint8_t* samples, const int count, int x0, int y0, int w, int h, uint16_t color);
void draw_select_line(const Channel* channels, int selected_channel);
void draw_la_status_icon(bool la_is_running);
void channels_init(Channel* channels, const int size);
void text_clear();

#endif