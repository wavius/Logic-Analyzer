#ifndef VISUALIZER_LOGIC_H
#define VISUALIZER_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#include "constants.h"

/********************************
 *  Structs
 ********************************/
typedef struct {
    //-- typically hardcoded values --//
    uint32_t sample_rate;         // sampling frequency
    uint32_t buffer_size;         // buffer size (set to be 4096)
    uint32_t vertical_divisions;  // vertical divisions (set to be 8)

    //-- zoom values --//
    uint32_t time_div;         // time div in ns/div
    uint32_t visible_samples;  // current zoom window
    uint32_t scroll_offset;    // where the signal will start in the sample array (determined by user)
} ZoomState;

/********************************
 *  Functions
 ********************************/
void zoom_state_init(ZoomState* g_state, uint32_t default_visible_samples);
void visualizer_set_zoom(ZoomState* g_state, uint32_t time_div, uint32_t trigger_position);
bool visualizer_zoom_in(ZoomState* g_state, uint32_t trigger_position);
bool visualizer_zoom_out(ZoomState* g_state, uint32_t trigger_position);
void visualizer_scroll_left(ZoomState* g_state);
void visualizer_scroll_right(ZoomState* g_state);
uint32_t visualizer_get_end_sample(const ZoomState* g_state);
void center_view_on_trigger(ZoomState* g_state, uint32_t trigger_position);

#endif