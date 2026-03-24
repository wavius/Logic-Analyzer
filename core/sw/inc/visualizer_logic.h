#ifndef VISUALIZER_LOGIC_H
#define VISUALIZER_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#define TOTAL_SIGANLS 16       // hard coded to be 16
#define BUFFER_SIZE 4096       // hard coded to be 4096
#define SAMPLE_RATE 100000000  // hard coded to be 100 MHz
#define VERTICAL_DIVISIONS 8   // hard coded to hold 8

/********************************
 *  Structs
 ********************************/
typedef struct {
    //-- typically hardcoded values --//
    uint32_t sample_rate;         // sampling frequency
    uint32_t buffer_size;         // buffer size (set to be 4096)
    uint32_t vertical_divisions;  // vertical divisions (set to be 8)

    //-- zoom values --//
    uint32_t time_div;         // time div
    uint32_t visible_samples;  // current zoom window
    uint32_t scroll_offset;    // where the signal will start in the sample array (determined by user)
} ZoomState;

/********************************
 *  Functions
 ********************************/
void zoom_state_init(ZoomState* g_state);
void visualizer_set_zoom(ZoomState* g_state, uint32_t time_div);
bool visualizer_zoom_in(ZoomState* g_state);
bool visualizer_zoom_out(ZoomState* g_state);
void visualizer_scroll_left(ZoomState* g_state);
void visualizer_scroll_left(ZoomState* g_state);

#endif