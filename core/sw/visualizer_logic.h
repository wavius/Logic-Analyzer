#ifndef VISUALIZER_LOGIC_H
#define VISUALIZER_LOGIC_H

#include <stdint.h>

typedef struct {
    uint32_t sample_rate_Mhz;  // sampling frequency
    uint32_t buffer_size;      // amount of samples in the buffer (4096 planned for rn)
    uint32_t visible_samples;  // current zoom window
    uint32_t start_sample;     // scroll offset
    uint16_t waveform_width_px;
    uint8_t num_v_div;  // 8
} VisualizerState;

typedef enum {
    RENDER_DETAILED,
    RENDER_COMPRESSED
} RenderMode;

void visualizer_init(VisualizerState* state, uint32_t sample_rate_Mhz, uint32_t capture_size,
                     uint16_t waveform_width_px, uint8_t num_divisions);

void visualizer_set_zoom(VisualizerState* state, uint32_t visible_samples);

void visualizer_zoom_in(VisualizerState* state);
void visualizer_zoom_out(VisualizerState* state);

void visualizer_scroll_left(VisualizerState* state);
void visualizer_scroll_right(VisualizerState* state);

uint32_t visualizer_get_end_sample(const VisualizerState* state);
uint32_t visualizer_get_samples_per_div(const VisualizerState* state);
uint32_t visualizer_get_scroll_step(const VisualizerState* state);

double visualizer_get_visible_time_s(const VisualizerState* state);
double visualizer_get_time_per_div_s(const VisualizerState* state);

RenderMode visualizer_get_render_mode(const VisualizerState* state);
double visualizer_get_pixels_per_sample(const VisualizerState* state);

#endif