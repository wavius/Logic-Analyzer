#include "draw_screen.h"

#include "vga_driver.h"

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

/********************************
 *  Structs + global variables
 ********************************/
//-- screen sizing variables --//
static const int top_bar_height = 15;
static const int left_bar_width = 25;
static const int bottom_bar_height = 7;
static const int channel_area_height = SCREEN_H - (top_bar_height + bottom_bar_height);

//-- waveform / grid layout variables --//
static const int grid_spacing_x = 40;
static const int waveform_margin_divisor = 4;
static const int waveform_min_margin = 1;

//-- UI color variables --//
static const uint16_t top_bar_color = 0x39E7;
static const uint16_t bottom_bar_color = 0x2104;
static const uint16_t left_bar_color = 0x2104;
static const uint16_t separator_color = 0xFFFF;
static const uint16_t grid_color = 0x4208;

// color options for the signals drawn to screen
static const uint16_t channel_colors[16] = {
    0x5D6B, 0x8E24, 0x8E24, 0xC7C0,
    0xE7E0, 0xF580, 0xE3A0, 0xD820,
    0x72A9, 0x5249, 0x3A89, 0x44CB,
    0x5CFE, 0x65FF, 0x65D7, 0x61ED};

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
static void draw_hline(int x_start, int x_end, int y, uint16_t color);
static void draw_vline(int x, int y_start, int y_end, uint16_t color);
static void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
static int calculate_channel_height(const int lanes, const int available_height);
static void draw_text();  // temp, unfinished

/********************************
 *  Helper Functions
 ********************************/
// draw a horizontal line
static void draw_hline(int x_start, int x_end, int y, uint16_t color) {
    for (int x = x_start; x <= x_end; x++)
        plot_pixel(x, y, color);
}

// draw a vertical line
static void draw_vline(int x, int y_start, int y_end, uint16_t color) {
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
static void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color) {
    // note: x_cord and y_cord define the top left corner of the rectangle being drawn
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x_cord + i, y_cord + j, color);
}

// if want n channels, draw n - 1 lines spaced (available_height / lanes) apart
static int calculate_channel_height(const int lanes, const int available_height) {
    if (lanes <= 0)
        return 0;
    return available_height / lanes;
}

// draw text to screen
static void draw_text() {
    // draw title
    // draw channel labels
    // draw grid numbers
    // draw wtv else
}

/********************************
 *  Function Implementations
 ********************************/
// draws the main static part of the background
void draw_logic_ui_frame(const int lanes) {
    if (lanes <= 0)
        return;

    // Top bar
    fill_rect(0, 0, SCREEN_W, top_bar_height, top_bar_color);

    // bottom bar
    fill_rect(0, SCREEN_H - bottom_bar_height, SCREEN_W, bottom_bar_height, bottom_bar_color);

    // Left label column
    fill_rect(0, top_bar_height, left_bar_width, channel_area_height, left_bar_color);

    // Channel separators
    int spacing = calculate_channel_height(lanes, channel_area_height);
    for (int i = 1; i < lanes; i++) {
        int y = top_bar_height + i * spacing;
        draw_hline(0, SCREEN_W - 1, y, separator_color);
    }

    // Vertical grid lines
    for (int x = left_bar_width; x < SCREEN_W; x += grid_spacing_x)
        draw_vline(x, top_bar_height, SCREEN_H - bottom_bar_height - 1, grid_color);

    draw_text();
}

// based on recieved array samples and count (the amount of cycles), draws any given digital waveform
void draw_digital_waveform(const uint8_t* samples, const int count, int x_start, int y_top, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0)
        return;

    int margin = h / waveform_margin_divisor;
    if (margin < waveform_min_margin)
        margin = waveform_min_margin;

    int y_high = y_top + margin;
    int y_low = y_top + h - 1 - margin;

    if (y_low < y_high)
        y_low = y_high;

    if (count <= 0 || samples == 0) {  // given there is no waveform given, just draw a horizontal line
        draw_hline(x_start, x_start + w - 1, y_low, color);
        return;
    }

    int prev = samples[0] ? 1 : 0;
    int prev_y = prev ? y_high : y_low;

    for (int x = 0; x < w; x++) {
        int idx = (x * count) / w;
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

// draw signals
void draw_signals(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0)
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