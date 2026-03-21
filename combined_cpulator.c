/* --- START OF core/sw/visualizer_logic.h --- */
#ifndef VISUALIZER_LOGIC_H
#define VISUALIZER_LOGIC_H

#include <stdint.h>

typedef struct {
    uint32_t sample_rate_Mhz;  // sampling frequency
    uint32_t capture_size;     // 4096
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
/* --- END OF core/sw/visualizer_logic.h --- */

/* --- START OF core/sw/visualizer_logic.c --- */
// Removed include: core/sw/visualizer_logic.c

#include <stddef.h>

/********************************
 *  Structs + global variables
 ********************************/
const uint32_t zoom_levels[] = {64, 96, 128, 256, 512, 1024};
const uint32_t zoom_level_count = 6;

/********************************
 *  Helper Function Declarations
 ********************************/
uint32_t min_u32(uint32_t a, uint32_t b);
int find_zoom_index(uint32_t visible_samples);
void clamp_start_sample(VisualizerState* state);
uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t capture_size);

/********************************
 *  Helper Functions
 ********************************/
// returns smaller of a and b
uint32_t min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

// returns zoom index if found exactly, otherwise nearest valid index
int find_best_zoom_index(uint32_t visible_samples) {
    int best_index = 0;
    uint32_t best_diff = (zoom_levels[0] > visible_samples) ? (zoom_levels[0] - visible_samples) : (visible_samples - zoom_levels[0]);

    for (int i = 1; i < zoom_level_count; i++) {
        uint32_t diff = (zoom_levels[i] > visible_samples) ? (zoom_levels[i] - visible_samples) : (visible_samples - zoom_levels[i]);

        if (diff < best_diff) {
            best_diff = diff;
            best_index = i;
        }
    }

    return best_index;  // best index from the zoom levels struct
}

// ensures visible_samples is valid and does not exceed capture size
uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t capture_size) {
    if (capture_size == 0)
        return 0;

    // choose nearest zoom level first
    int idx = find_best_zoom_index(visible_samples);
    uint32_t chosen = zoom_levels[idx];

    // if chosen zoom is larger than capture, shrink to largest valid zoom
    if (chosen > capture_size) {
        for (int i = zoom_level_count - 1; i >= 0; i--) {  // short for loop since zoom level count is just 6
            if (zoom_levels[i] <= capture_size)
                return zoom_levels[i];
        }

        // if capture is smaller than all zoom levels, just show entire capture
        return capture_size;
    }

    return chosen;  // if capture size is larger than the closest zoom level, return the closest zoom level
}

// keeps start_sample within valid range
void clamp_start_sample(VisualizerState* state) {
    if (state == 0)
        return;

    if (state->capture_size == 0) {
        state->start_sample = 0;
        return;
    }

    if (state->visible_samples >= state->capture_size) {
        state->start_sample = 0;
        return;
    }

    uint32_t max_start = state->capture_size - state->visible_samples;
    if (state->start_sample > max_start) {
        state->start_sample = max_start;
    }
}

/********************************
 *  Function Implementations
 ********************************/
// <add description>
void visualizer_init(VisualizerState* state, uint32_t sample_rate_Mhz, uint32_t capture_size,
                     uint16_t waveform_width_px, uint8_t num_divisions) {
    if (state == 0)
        return;

    state->sample_rate_Mhz = sample_rate_Mhz;
    state->capture_size = capture_size;
    state->waveform_width_px = waveform_width_px;
    state->num_v_div = num_divisions;

    // default zoom: 96 if possible, otherwise clamp to what fits
    state->visible_samples = clamp_visible_samples(96, capture_size);
    state->start_sample = 0;

    clamp_start_sample(state);
}

// <add description>
void visualizer_set_zoom(VisualizerState* state, uint32_t visible_samples) {
    if (state == 0) {
        return;
    }

    state->visible_samples = clamp_visible_samples(visible_samples, state->capture_size);
    clamp_start_sample(state);
}

// <add description>
void visualizer_zoom_in(VisualizerState* state) {
    if (state == 0 || state->capture_size == 0)
        return;

    int idx = find_best_zoom_index(state->visible_samples);

    // move to smaller window if possible
    if (zoom_levels[idx] >= state->visible_samples && idx > 0)
        idx--;
    else if (zoom_levels[idx] > state->visible_samples && idx > 0)
        idx--;

    state->visible_samples = clamp_visible_samples(zoom_levels[idx], state->capture_size);
    clamp_start_sample(state);
}

// <add description>
void visualizer_zoom_out(VisualizerState* state) {
    if (state == 0 || state->capture_size == 0)
        return;

    int idx = find_best_zoom_index(state->visible_samples);

    // move to larger window if possible
    if (zoom_levels[idx] <= state->visible_samples && idx < zoom_level_count - 1)
        idx++;
    else if (zoom_levels[idx] < state->visible_samples && idx < zoom_level_count - 1)
        idx++;

    state->visible_samples = clamp_visible_samples(zoom_levels[idx], state->capture_size);
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

    if (state->capture_size > state->visible_samples)
        max_start = state->capture_size - state->visible_samples;

    if (state->start_sample + step > max_start)
        state->start_sample = max_start;
    else
        state->start_sample += step;

    clamp_start_sample(state);
}

// <add description>
uint32_t visualizer_get_end_sample(const VisualizerState* state) {
    if (state == 0 || state->capture_size == 0)
        return 0;

    uint32_t end = state->start_sample + state->visible_samples;
    return min_u32(end, state->capture_size);
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
/* --- END OF core/sw/visualizer_logic.c --- */

/* --- START OF core/sw/draw_screen.h --- */
#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

// Removed include: core/sw/draw_screen.h

// ----- Structs ----- //
// all info needed to draw a singal
typedef struct {
    const uint8_t* samples;
    int count;
    // uint16_t color;  // bring back this feature later if I think of a better way to include it
    bool enabled;
    char label[8];
} Channel;

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame(const Channel* channels, const int lanes);
void draw_digital_waveform(const uint8_t* samples, const int count, int x0, int y0, int w, int h, uint16_t color);
void draw_signals(const Channel* channels, const int lanes);
void draw_logic_view(const VisualizerState* state, const Channel* channels, int lanes);

#endif
/* --- END OF core/sw/draw_screen.h --- */

/* --- START OF core/sw/vga_driver.h --- */
#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>

// ----- Core VGA ----- //
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();

// ----- Other ----- //
// int getXres(); //implemented for debugging purposes

#endif
/* --- END OF core/sw/vga_driver.h --- */

/* --- START OF core/sw/vga_driver.c --- */
// Removed include: core/sw/vga_driver.c

#include <string.h>  //for memset

// Removed include: core/sw/vga_driver.c

/********************************
 *  Structs
 ********************************/
struct frame_buffer_controller {
    volatile uint32_t frontBuffer;
    volatile uint32_t backBuffer;
    volatile uint16_t xRes;
    volatile uint16_t yRes;
    volatile uint32_t statusReg;
};

/********************************
 *  Global Variables
 ********************************/
volatile struct frame_buffer_controller* frameBuf =
    (struct frame_buffer_controller*)0xFF203020;
volatile int pixel_buffer_start;

// ----- Two frame buffers (double buffering) ----- //
uint16_t Buffer1[240][512];
uint16_t Buffer2[240][512];

/********************************
 *  Function Implementations
 ********************************/
// Initialize VGA system (from Lab 7)
void vga_init() {
    // Intialize front buffer to Buffer1, then swap to make it front
    frameBuf->backBuffer =
        (uint32_t)Buffer1;  // store in the back buffer the address of the 1st buffer
    wait_for_vsync();       // swap so buffer 1 address is held in front buffer

    // Set drawing pointer to front buffer and clear it
    pixel_buffer_start =
        frameBuf->frontBuffer;  // get front buffer, and then clear it
    clear_screen();

    // Intialize back buffer as Buffer2
    frameBuf->backBuffer = (uint32_t)Buffer2;

    //  clear the back buffer
    pixel_buffer_start = frameBuf->backBuffer;
    clear_screen();
}

// Wait for vertical sync and swap buffers
void wait_for_vsync() {
    frameBuf->frontBuffer = 1;        // request swap
    while (frameBuf->statusReg & 1);  // poll the status bit to see if the swap is done
    pixel_buffer_start =
        frameBuf->backBuffer;  // set a new main buffer to write into
}

// Clear entire screen
void clear_screen() {
    // 240 rows × 1024 bytes per row = 245760 bytes
    memset((void*)pixel_buffer_start, 0, 245760);  // write black into all the space in the front buffer
}

// Plot a single pixel
void plot_pixel(int x, int y, uint16_t color) {
    volatile uint16_t* pixel_addr;
    pixel_addr = (volatile uint16_t*)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

/*
// implemented for debugging purposes
int getXres() {
    return frameBuf->xRes;
}
*/

/* --- END OF core/sw/vga_driver.c --- */

/* --- START OF core/sw/draw_screen.c --- */
// Removed include: core/sw/draw_screen.c

// Removed include: core/sw/draw_screen.c
// Removed include: core/sw/draw_screen.c

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

// ----- Char buffer ----- //
#define CHAR_COLS 80
#define CHAR_ROWS 60

// HARD CODED CHANNEL VALUE
#define CHANNEL_LIMIT 16
/********************************
 *  Structs + global variables
 ********************************/
//-- screen sizing variables --//
const int top_bar_height = 15;
const int left_bar_width = 32;  // cause there are 8 major vertical grid ticks. (320 - 32)/8 = 36
const int bottom_bar_height = 7;
const int channel_area_height = SCREEN_H - (top_bar_height + bottom_bar_height);

//-- waveform / grid layout variables --//
const int vertical_grid_ticks = 8;
const int grid_spacing_x = (SCREEN_W - left_bar_width) / 8;
const int waveform_margin_divisor = 4;
const int waveform_min_margin = 1;

//-- UI color variables --//
const uint16_t top_bar_color = 0x18e3;
const uint16_t bottom_bar_color = 0x18e3;
const uint16_t left_bar_color = 0x18e3;
const uint16_t separator_color = 0x39c7;
const uint16_t grid_color = 0x18e3;
const uint16_t text_color = 0xd69a;

// color options for the signals drawn to screen
const uint16_t channel_colors[16] = {
    0x5D6B, 0x8E24, 0x8E24, 0xC7C0,
    0xE7E0, 0xF580, 0xE3A0, 0xD820,
    0x72A9, 0x5249, 0x3A89, 0x44CB,
    0x5CFE, 0x65FF, 0x65D7, 0x61ED};

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
void draw_hline(int x_start, int x_end, int y, uint16_t color);
void draw_vline(int x, int y_start, int y_end, uint16_t color);
void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
int calculate_channel_height(const int lanes, const int available_height);
void text_plot_char(int col, int row, char c);
void text_draw_string(int col, int row, const char* text);
void text_clear(void);
void draw_channel_labels(const Channel* channels, int lanes);
uint16_t dim_color(uint16_t color);
void draw_channel_labels(const Channel* channels, const int lanes);

/********************************
 *  Helper Functions
 ********************************/
// draw a horizontal line
void draw_hline(int x_start, int x_end, int y, uint16_t color) {
    for (int x = x_start; x <= x_end; x++)
        plot_pixel(x, y, color);
}

// draw a vertical line
void draw_vline(int x, int y_start, int y_end, uint16_t color) {
    // swap for negative lines
    if (y_start > y_end) {
        int temp = y_start;
        y_start = y_end;
        y_end = temp;
    }

    for (int y = y_start; y <= y_end; y++)
        plot_pixel(x, y, color);
}

// draw a filled rectangle (to draw bars / labels)
void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color) {
    // note: x_cord and y_cord define the top left corner of the rectangle being drawn
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x_cord + i, y_cord + j, color);
}

// if want n channels, draw n - 1 lines spaced (available_height / lanes) apart
int calculate_channel_height(const int lanes, const int available_height) {
    if (lanes <= 0)
        return 0;
    return available_height / lanes;
}

// store a single char in a string buffer
void text_plot_char(int col, int row, char c) {
    if (col < 0 || col >= CHAR_COLS || row < 0 || row >= CHAR_ROWS)
        return;

    // pointer to controller
    volatile int* ctrl = (int*)0xFF203030;

    // first register = character buffer address
    volatile char* char_buf = (volatile char*)(ctrl[0]);

    char_buf[row * CHAR_COLS + col] = c;
}

// draw text to screen
void text_draw_string(int col, int row, const char* text) {
    if (text == 0)
        return;

    int cur_col = col;
    int cur_row = row;

    while (*text) {
        if (*text == '\n') {
            cur_row++;
            cur_col = col;
        } else {
            text_plot_char(cur_col, cur_row, *text);
            cur_col++;

            if (cur_col >= CHAR_COLS) {
                cur_col = col;
                cur_row++;
            }
        }

        if (cur_row >= CHAR_ROWS)
            break;

        text++;
    }
}

// clear buffer or smth? idk
void text_clear(void) {
    volatile int* ctrl = (int*)0xFF203030;
    volatile char* char_buf = (volatile char*)(ctrl[0]);

    for (int row = 0; row < CHAR_ROWS; row++) {
        for (int col = 0; col < CHAR_COLS; col++) {
            char_buf[row * CHAR_COLS + col] = ' ';
        }
    }
}

// for dimming the color of text drawn to screen (used for channel labels for example)
uint16_t dim_color(uint16_t color) {
    uint16_t r = (color >> 11) & 0x1F;
    uint16_t g = (color >> 5) & 0x3F;
    uint16_t b = color & 0x1F;

    r >>= 1;
    g >>= 1;
    b >>= 1;

    return (r << 11) | (g << 5) | b;
}

// draws labels inside the left label column
void draw_channel_labels(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > CHANNEL_LIMIT)
        return;

    const int lane_height = calculate_channel_height(lanes, channel_area_height);

    // character buffer is 80x60 over a 320x240 logical screen
    // so 1 char cell = 4x4 logical pixels
    const int char_w = 4;
    const int char_h = 4;

    const int stripe_width = 2;
    const int text_pad_x = 2;  // pixel padding from stripe/right of stripe
    const int text_pad_y = 0;  // keep 0 for now unless you want a slight downward shift

    // start text just to the right of the color stripe
    const int text_x_px = stripe_width + text_pad_x;
    const int text_col = text_x_px / char_w;

    // how many chars fit in the label column
    const int max_label_chars =
        (left_bar_width - text_x_px) / char_w;

    if (max_label_chars <= 0)
        return;

    for (int i = 0; i < lanes; i++) {
        const int y_top = top_bar_height + i * lane_height;

        uint16_t stripe_color = channels[i].enabled
                                    ? channel_colors[i]
                                    : dim_color(channel_colors[i]);

        uint16_t label_bg = channels[i].enabled
                                ? left_bar_color
                                : dim_color(left_bar_color);

        // fill this lane's label background
        fill_rect(0, y_top, left_bar_width, lane_height, label_bg);

        // draw color stripe at far left
        fill_rect(0, y_top, stripe_width, lane_height, stripe_color);

        // vertically center one text row inside the lane
        int text_y_px = y_top + (lane_height - char_h) / 2 + text_pad_y;
        int label_row = text_y_px / char_h;

        // safety clamp
        if (label_row < 0)
            label_row = 0;
        if (label_row >= CHAR_ROWS)
            label_row = CHAR_ROWS - 1;

        // draw clipped label
        if (channels[i].label != 0) {
            const char* s = channels[i].label;
            for (int j = 0; j < max_label_chars && s[j] != '\0'; j++) {
                text_plot_char(text_col + j, label_row, s[j]);
            }
        }
    }
}
/********************************
 *  Function Implementations
 ********************************/
// draws the main static part of the background
void draw_logic_ui_frame(const Channel* channels, const int lanes) {
    if (lanes <= 0 || (lanes > CHANNEL_LIMIT))
        return;

    // Top bar
    fill_rect(0, 0, SCREEN_W, top_bar_height, top_bar_color);

    // bottom bar
    fill_rect(0, SCREEN_H - bottom_bar_height, SCREEN_W, bottom_bar_height, bottom_bar_color);

    // Left label column
    fill_rect(0, top_bar_height, left_bar_width, channel_area_height, left_bar_color);

    // Vertical grid lines
    for (int x = left_bar_width; x < SCREEN_W; x += grid_spacing_x)
        draw_vline(x, top_bar_height, SCREEN_H - bottom_bar_height - 1, grid_color);

    // Channel separators
    int spacing = calculate_channel_height(lanes, channel_area_height);
    for (int i = 1; i < lanes; i++) {
        int y = top_bar_height + i * spacing;
        draw_hline(0, SCREEN_W - 1, y, separator_color);
    }

    // labels + stripes
    draw_channel_labels(channels, lanes);
}

// based on recieved array samples and count (the amount of cycles), draws any given digital waveform
void draw_digital_waveform(const uint8_t* samples, const int count, int x_start, int y_top, int draw_width, int draw_heigth, uint16_t color) {
    if (draw_width <= 0 || draw_heigth <= 0)
        return;

    int margin = draw_heigth / waveform_margin_divisor;
    if (margin < waveform_min_margin)
        margin = waveform_min_margin;

    int y_high = y_top + margin;
    int y_low = y_top + draw_heigth - 1 - margin;

    if (y_low < y_high)
        y_low = y_high;

    if (count <= 0 || samples == 0) {  // given there is no waveform given, just draw a horizontal line
        draw_hline(x_start, x_start + draw_width - 1, y_low, color);
        return;
    }

    int prev = samples[0] ? 1 : 0;
    int prev_y = prev ? y_high : y_low;

    for (int x = 0; x < draw_width; x++) {
        int idx = (x * count) / draw_width;  // automatically scale the amount of samples given to fit the screen
        if (idx >= count)
            idx = count - 1;

        int cur = samples[idx] ? 1 : 0;
        int y = cur ? y_high : y_low;
        int screen_x = x_start + x;

        // vertical edge
        if (x > 0 && cur != prev)
            draw_vline(screen_x, prev_y, y, color);

        // horizontal segment
        plot_pixel(screen_x, y, color);

        prev = cur;
        prev_y = y;
    }
}

// draw signals (no zooming and scrolling logic, just prints start of given sample buffer)
void draw_signals(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > CHANNEL_LIMIT)
        return;

    int lane_height = calculate_channel_height(lanes, channel_area_height);
    int x_start = left_bar_width;
    int waveform_width = SCREEN_W - left_bar_width;

    // draw the signals
    for (int i = 0; i < lanes; i++) {
        if (!channels[i].enabled)
            continue;

        int y_top = top_bar_height + i * lane_height;
        draw_digital_waveform(channels[i].samples, channels[i].count, x_start, y_top, waveform_width, lane_height, channel_colors[i]);
    }
}

// handle zooming logic by determining the sample window for each enabled channel and prints it out using draw_digital_waveform(...)
void draw_logic_view(const VisualizerState* state, const Channel* channels, int lanes) {
    if (state == 0 || channels == 0 || lanes <= 0)
        return;

    uint32_t start = state->start_sample;
    uint32_t end = visualizer_get_end_sample(state);

    if (end <= start)
        return;

    uint32_t visible_count = end - start;

    int lane_height = calculate_channel_height(lanes, channel_area_height);
    int x_start = left_bar_width;
    int waveform_width = SCREEN_W - left_bar_width;

    for (int i = 0; i < lanes; i++) {
        if (!channels[i].enabled)
            continue;

        int y_top = top_bar_height + i * lane_height;

        draw_digital_waveform(
            &channels[i].samples[start],  // shifted pointer
            visible_count,
            x_start,
            y_top,
            waveform_width,
            lane_height,
            channel_colors[i]);
    }
}
/* --- END OF core/sw/draw_screen.c --- */

/* --- START OF core/sw/main.c --- */
#include <stdint.h>

// Removed include: core/sw/main.c
// Removed include: core/sw/main.c
// Removed include: core/sw/main.c

/********************************
 *  Structs + global variables
 ********************************/

/********************************
 *  Main Program
 ********************************/
// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
int main(void) {
    vga_init();  // must always be done first
    const int lanes = 8;

    // ----- Fake sample data (0 = low, 1 = high) ----- //
    static uint8_t ch0[64] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

    static uint8_t ch1[96] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

    static uint8_t ch2[64] = {
        0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0};

    static uint8_t ch3[64] = {
        0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1,
        0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0,
        0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0,
        1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0};

    static Channel channels[16] = {
        {ch0, 64, true, "CH0"},
        {ch1, 96, true, "CH1"},
        {ch2, 64, true, "CH2"},
        {ch3, 64, true, "CH3"},
        {0, 0, true, "CH4"},
        {0, 0, false, "CH5"},
        {0, 0, false, "CH6"},
        {0, 0, false, "CH7"},
        {0, 0, false, "CH8"},
        {0, 0, false, "CH9"},
        {0, 0, false, "CH10"},
        {0, 0, false, "CH11"},
        {0, 0, false, "CH12"},
        {0, 0, false, "CH13"},
        {0, 0, false, "CH14"},
        {0, 0, false, "CH15"}};

    VisualizerState view;

    visualizer_init(&view, 1000000, 4096, 288, 8);
    visualizer_set_zoom(&view, 256);

    // TEMP manual scroll test
    view.start_sample = 128;

    while (1) {
        clear_screen();
        draw_logic_ui_frame(channels, lanes);
        // draw_signals(channels, lanes);
        draw_logic_view(&view, channels, lanes);
        wait_for_vsync();
        view.start_sample++;
    }

    return 0;
}
/* --- END OF core/sw/main.c --- */
