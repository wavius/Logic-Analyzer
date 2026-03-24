#include "visualizer_logic.h"

#include <stddef.h>

#define ZOOM_LVL_COUNT 6
/********************************
 *  Structs + global variables
 ********************************/
static const uint32_t zoom_levels_samples[ZOOM_LVL_COUNT] = {64, 96, 128, 256, 512, 1024};
static uint32_t zoom_levels_time_div[ZOOM_LVL_COUNT];

/********************************
 *  Helper Function Declarations
 ********************************/
static int find_best_zoom_index(uint32_t visible_samples);
static void clamp_scroll_offset(ZoomState* g_state);
static uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size);
uint32_t visualizer_get_scroll_step(const ZoomState* g_state);
static void calc_time_div_zoom_lvls();
uint32_t time_div_to_samples(uint32_t time_div);
uint32_t visualizer_get_scroll_step(const ZoomState* g_state);

/********************************
 *  Helper Functions
 ********************************/
// returns zoom index if found exactly, otherwise nearest valid index
static int find_best_zoom_index(uint32_t visible_samples) {
    int best_index = 0;
    uint32_t best_diff = (zoom_levels_samples[0] > visible_samples) ? (zoom_levels_samples[0] - visible_samples) : (visible_samples - zoom_levels_samples[0]);

    for (int i = 1; i < ZOOM_LVL_COUNT; i++) {
        uint32_t diff = (zoom_levels_samples[i] > visible_samples) ? (zoom_levels_samples[i] - visible_samples) : (visible_samples - zoom_levels_samples[i]);

        if (diff < best_diff) {
            best_diff = diff;
            best_index = i;
        }
    }

    return best_index;  // best index from the zoom levels struct
}

// ensures visible_samples is valid and does not exceed capture size
static uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size) {
    // for a hardcoded 4096 bit buffer, this function does nothing other than call find_best_zoom_index
    if (buffer_size == 0)  // buffer contains no samples (shouldn't ever happen)
        return 0;

    // choose nearest zoom level first
    int idx = find_best_zoom_index(visible_samples);
    uint32_t chosen = zoom_levels_samples[idx];

    // if chosen zoom is larger than capture, shrink to largest valid zoom
    if (chosen > buffer_size) {
        for (int i = ZOOM_LVL_COUNT - 1; i >= 0; i--) {  // short for loop since zoom level count is just 6
            if (zoom_levels_samples[i] <= buffer_size)
                return zoom_levels_samples[i];
        }

        // if capture is smaller than all zoom levels, just show entire capture (but this shouldn't ever happen since our buffer will be huge)
        return buffer_size;
    }

    return chosen;  // if buffer size is larger than all the chosen zoom levels, just return with chosen zoom level as visible samples.
}

// keeps start_sample within valid range
static void clamp_scroll_offset(ZoomState* g_state) {
    if (g_state == 0)  // uninitalized state->return
        return;

    if (g_state->buffer_size == 0) {  // nothing in the buffer
        g_state->scroll_offset = 0;
        return;
    }

    // if zoom level is greater than the buffer size, show the entire waveform by starting at 0th sample (shouldn't happpen
    // with current hard coded values)
    if (g_state->visible_samples >= g_state->buffer_size) {
        g_state->scroll_offset = 0;
        return;
    }

    uint32_t max_start = g_state->buffer_size - g_state->visible_samples;
    if (g_state->scroll_offset > max_start) {  // prevent how far into the buffer we can show the signal
        g_state->scroll_offset = max_start;
    }
}

// calculate time div
static void calc_time_div_zoom_lvls() {
    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        zoom_levels_time_div[i] = (zoom_levels_samples[i] / VERTICAL_DIVISIONS) / SAMPLE_RATE;
    }
}

// go from time/div to samples
uint32_t time_div_to_samples(uint32_t time_div) {
    return (time_div * SAMPLE_RATE) * VERTICAL_DIVISIONS;
}

// get the amount each attempt to scroll will shift the scroll offset of the buffer
uint32_t visualizer_get_scroll_step(const ZoomState* g_state) {
    if (g_state == 0 || g_state->vertical_divisions == 0)
        return 1;
    return g_state->visible_samples / (g_state->vertical_divisions * 3);  // integer division is fine here for display / scroll behavior
}

/********************************
 *  Function Implementations
 ********************************/
// initazlize the zoom state
void zoom_state_init(ZoomState* g_state) {
    if (g_state == 0)  // uninitalized state->return
        return;

    calc_time_div_zoom_lvls();  // populate time div array

    g_state->sample_rate = SAMPLE_RATE;
    g_state->buffer_size = BUFFER_SIZE;
    g_state->vertical_divisions = VERTICAL_DIVISIONS;

    // default zoom: 96 if possible, otherwise clamp to what fits
    g_state->visible_samples = clamp_visible_samples(96, BUFFER_SIZE);
    g_state->scroll_offset = 0;  // to start, show from the 0th position

    clamp_scroll_offset(g_state);
}

// pick a new zoom (time div) and check for errors
void visualizer_set_zoom(ZoomState* g_state, uint32_t time_div) {
    if (g_state == 0)
        return;
    g_state->time_div = time_div;
    g_state->visible_samples = time_div_to_samples(g_state->time_div);
    // uint32_t visible_samples = time_div_to_samples(time_div);
    // g_state->visible_samples = clamp_visible_samples(visible_samples, g_state->buffer_size);
    clamp_scroll_offset(g_state);
}

// Zoom in given the user presses a button
bool visualizer_zoom_in(ZoomState* g_state) {
    if (g_state == 0 || g_state->buffer_size == 0)
        return false;

    int inf = 1000000;
    int idx = inf;

    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        if (zoom_levels_time_div[i] == g_state->time_div)
            idx = i;
    }
    if (idx == inf)  // somehow, the wrong time_div was set, try again
        idx = find_best_zoom_index(g_state->visible_samples);

    // move to smaller window if possible
    if (idx > 0) {
        visualizer_set_zoom(g_state, zoom_levels_time_div[idx--]);
        return true;
    }
    return false;
}

// Zoom in given the user presses a button
bool visualizer_zoom_out(ZoomState* g_state) {
    if (g_state == 0 || g_state->buffer_size == 0)
        return false;

    int inf = -1000000;
    int idx = inf;

    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        if (zoom_levels_time_div[i] == g_state->time_div)
            idx = i;
    }
    if (idx == inf)  // somehow, the wrong time_div was set, try again
        idx = find_best_zoom_index(g_state->visible_samples);

    // move to smaller window if possible
    if (idx < ZOOM_LVL_COUNT - 1) {
        visualizer_set_zoom(g_state, zoom_levels_time_div[idx++]);
        return true;
    }
    return false;
}

// scroll left given the user tries to
void visualizer_scroll_left(ZoomState* g_state) {
    if (g_state == 0)
        return;

    uint32_t step = visualizer_get_scroll_step(g_state);

    // update the start
    int new_start = g_state->scroll_offset - step;
    g_state->scroll_offset = new_start;

    if (new_start < 0)
        g_state->scroll_offset = 0;
    else if (new_start > g_state->buffer_size)  // if the user scrolls beyond buffer size, stop them
        clamp_scroll_offset(g_state);
}

// scroll right given the user tries to
void visualizer_scroll_left(ZoomState* g_state) {
    if (g_state == 0)
        return;

    uint32_t step = visualizer_get_scroll_step(g_state);

    // update the start
    int new_start = g_state->scroll_offset + step;
    g_state->scroll_offset = new_start;

    if (new_start < 0)
        g_state->scroll_offset = 0;
    else if (new_start > g_state->buffer_size)  // if the user scrolls beyond buffer size, stop them
        clamp_scroll_offset(g_state);
}
