#include "visualizer_logic.h"

#include <stddef.h>

#define ZOOM_LVL_COUNT 6
/********************************
 *  Structs + global variables
 ********************************/
static const uint32_t zoom_levels[ZOOM_LVL_COUNT] = {64, 96, 128, 256, 512, 1024};

/********************************
 *  Helper Function Declarations
 ********************************/
static uint32_t min_u32(uint32_t a, uint32_t b);
static int find_zoom_index(uint32_t visible_samples);
static void clamp_start_sample(VisualizerState* state);
static uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t capture_size);

/********************************
 *  Helper Functions
 ********************************/
// returns smaller of a and b
static uint32_t min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

// returns zoom index if found exactly, otherwise nearest valid index
static int find_best_zoom_index(uint32_t visible_samples) {
    int best_index = 0;
    uint32_t best_diff = (zoom_levels[0] > visible_samples) ? (zoom_levels[0] - visible_samples) : (visible_samples - zoom_levels[0]);

    for (int i = 1; i < ZOOM_LVL_COUNT; i++) {
        uint32_t diff = (zoom_levels[i] > visible_samples) ? (zoom_levels[i] - visible_samples) : (visible_samples - zoom_levels[i]);

        if (diff < best_diff) {
            best_diff = diff;
            best_index = i;
        }
    }

    return best_index;  // best index from the zoom levels struct
}

// ensures visible_samples is valid and does not exceed capture size
static uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size) {
    if (buffer_size == 0)  // buffer contains no samples (shouldn't ever happen)
        return 0;

    // choose nearest zoom level first
    int idx = find_best_zoom_index(visible_samples);
    uint32_t chosen = zoom_levels[idx];

    // if chosen zoom is larger than capture, shrink to largest valid zoom
    if (chosen > buffer_size) {
        for (int i = ZOOM_LVL_COUNT - 1; i >= 0; i--) {  // short for loop since zoom level count is just 6
            if (zoom_levels[i] <= buffer_size)
                return zoom_levels[i];
        }

        // if capture is smaller than all zoom levels, just show entire capture (but this shouldn't ever happen since our buffer will be huge)
        return buffer_size;
    }

    return chosen;  // if buffer size is larger than all the chosen zoom levels, just return with chosen zoom level as visible samples.
}

// keeps start_sample within valid range
static void clamp_start_sample(VisualizerState* sig_disp_state) {
    if (sig_disp_state == NULL)  // uninitalized state->return
        return;

    if (sig_disp_state->buffer_size == 0) {  // nothing in the buffer
        sig_disp_state->start_sample = 0;
        return;
    }

    // if zoom level is greater than the buffer size, show the entire waveform by starting at 0th sample (shouldn't happpen)
    if (sig_disp_state->visible_samples >= sig_disp_state->buffer_size) {
        sig_disp_state->start_sample = 0;
        return;
    }

    uint32_t max_start = sig_disp_state->buffer_size - sig_disp_state->visible_samples;
    if (sig_disp_state->start_sample > max_start) {  // prevent how far into the buffer we can show the signal
        sig_disp_state->start_sample = max_start;
    }
}

/********************************
 *  Function Implementations
 ********************************/
// <add description>
void visualizer_init(VisualizerState* sig_disp_state, uint32_t sample_rate_Mhz, uint32_t capture_size,
                     uint16_t waveform_width_px, uint8_t num_divisions) {
    if (sig_disp_state == NULL)  // uninitalized state->return
        return;

    sig_disp_state->sample_rate_Mhz = sample_rate_Mhz;
    sig_disp_state->buffer_size = capture_size;
    sig_disp_state->waveform_width_px = waveform_width_px;
    sig_disp_state->num_v_div = num_divisions;

    // default zoom: 96 if possible, otherwise clamp to what fits
    sig_disp_state->visible_samples = clamp_visible_samples(96, capture_size);
    sig_disp_state->start_sample = 0;  // to start, show from the 0th position

    clamp_start_sample(sig_disp_state);
}

// <add description>
void visualizer_set_zoom(VisualizerState* state, uint32_t visible_samples) {
    if (state == 0) {
        return;
    }

    state->visible_samples = clamp_visible_samples(visible_samples, state->buffer_size);
    clamp_start_sample(state);
}

// <add description>
void visualizer_zoom_in(VisualizerState* state) {
    if (state == 0 || state->buffer_size == 0)
        return;

    int idx = find_best_zoom_index(state->visible_samples);

    // move to smaller window if possible
    if (zoom_levels[idx] >= state->visible_samples && idx > 0)
        idx--;
    else if (zoom_levels[idx] > state->visible_samples && idx > 0)
        idx--;

    state->visible_samples = clamp_visible_samples(zoom_levels[idx], state->buffer_size);
    clamp_start_sample(state);
}

// <add description>
void visualizer_zoom_out(VisualizerState* state) {
    if (state == 0 || state->buffer_size == 0)
        return;

    int idx = find_best_zoom_index(state->visible_samples);

    // move to larger window if possible
    if (zoom_levels[idx] <= state->visible_samples && idx < ZOOM_LVL_COUNT - 1)
        idx++;
    else if (zoom_levels[idx] < state->visible_samples && idx < ZOOM_LVL_COUNT - 1)
        idx++;

    state->visible_samples = clamp_visible_samples(zoom_levels[idx], state->buffer_size);
    clamp_start_sample(state);
}

// <add description>
void visualizer_scroll_left(VisualizerState* state) {
    if (state == 0)
        return;

    uint32_t step = visualizer_get_scroll_step(state);

    if (state->start_sample < step)
        state->start_sample = 0;
    else
        state->start_sample -= step;

    clamp_start_sample(state);
}

// <add description>
void visualizer_scroll_right(VisualizerState* state) {
    if (state == 0)
        return;

    uint32_t step = visualizer_get_scroll_step(state);
    uint32_t max_start = 0;

    if (state->buffer_size > state->visible_samples)
        max_start = state->buffer_size - state->visible_samples;

    if (state->start_sample + step > max_start)
        state->start_sample = max_start;
    else
        state->start_sample += step;

    clamp_start_sample(state);
}

// <add description>
uint32_t visualizer_get_end_sample(const VisualizerState* state) {
    if (state == 0 || state->buffer_size == 0)
        return 0;

    uint32_t end = state->start_sample + state->visible_samples;
    return min_u32(end, state->buffer_size);
}

// <add description>
uint32_t visualizer_get_samples_per_div(const VisualizerState* state) {
    if (state == 0 || state->num_v_div == 0)
        return 0;

    // integer division is fine here for display / scroll behavior
    return state->visible_samples / state->num_v_div;
}

// <add description>
uint32_t visualizer_get_scroll_step(const VisualizerState* state) {
    if (state == 0)
        return 1;

    uint32_t samples_per_div = visualizer_get_samples_per_div(state);

    // avoid returning 0
    return (samples_per_div == 0) ? 1 : samples_per_div;
}

// <add description>
double visualizer_get_visible_time_s(const VisualizerState* state) {
    if (state == 0 || state->sample_rate_Mhz == 0)
        return 0.0;

    return (double)state->visible_samples / (double)state->sample_rate_Mhz;
}

// <add description>
double visualizer_get_time_per_div_s(const VisualizerState* state) {
    if (state == 0 || state->sample_rate_Mhz == 0 || state->num_v_div == 0)
        return 0.0;

    return (double)state->visible_samples / ((double)state->sample_rate_Mhz * (double)state->num_v_div);
}

// <add description>
double visualizer_get_pixels_per_sample(const VisualizerState* state) {
    if (state == 0 || state->visible_samples == 0)
        return 0.0;

    return (double)state->waveform_width_px / (double)state->visible_samples;
}

// <add description>
RenderMode visualizer_get_render_mode(const VisualizerState* state) {
    double px_per_sample = visualizer_get_pixels_per_sample(state);

    // 3 px/sample is your cutoff for detailed waveform drawing
    if (px_per_sample >= 3.0)
        return RENDER_DETAILED;

    return RENDER_COMPRESSED;
}