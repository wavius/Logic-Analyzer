#include "draw_screen.h"

#include "vga_driver.h"

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

// ----- global variables ----- //
int top_bar_height = 20;
int left_bar_width = 52;

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
void draw_hline(int x_start, int x_end, int y, uint16_t color);
void draw_vline(int x, int y_start, int y_end, uint16_t color);
void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
void draw_text();  // temp, unfinished

int calculate_channel_height(const int lanes, int available_height);

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
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x_cord + i, y_cord + j, color);
}

// calculate the channel height
int calculate_channel_height(const int lanes, int available_height) {
    // if want n channels, draw n - 1 lines spaced (available_height / lanes) apart
    /* 0
    -- 2

    -- 4

    -- 6

    -- 8

    -- 10


    12 */
    return available_height / (lanes);
}

void draw_text() {
    // draw title
    // draw channel labels
    // draw grid numbers
    // draw wtv else
}

/********************************
 *  Function Implementations
 ********************************/
// very basic implementation for the time being
void draw_logic_ui_frame(int lanes) {
    // Top bar
    fill_rect(0, 0, SCREEN_W, top_bar_height, 0x39E7);  // grey

    // Left label column
    fill_rect(0, top_bar_height, left_bar_width, SCREEN_H - top_bar_height, 0x2104);  // darker grey

    // Channel separators (4 lanes)
    int spacing = calculate_channel_height(lanes, SCREEN_H - top_bar_height);
    for (int i = 0; i < (lanes - 1); i++) {
        int y = spacing + i * spacing;
        draw_hline(0, SCREEN_W - 1, y, 0xFFFF);  // white line
    }

    // Vertical grid lines
    for (int x = 60; x < SCREEN_W; x += 40)
        draw_vline(x, 20, SCREEN_H - 1, 0x4208);  // faint grey
}

void draw_digital_waveform(const uint8_t* samples, int count, int x_start, int y_middle_cord, int w, int h, uint16_t color) {
    int y_high = y_middle_cord + 10;
    int y_low = y_middle_cord + h - 10;
    if (count <= 0) {  // given there is no waveform given, just draw a horizontal line
        draw_hline(x_start, x_start + w - 1, y_low, color);
        return;
    }
    int prev = samples[0];

    for (int x = 0; x < w; x++) {
        int idx = (x * count) / w;
        int cur = samples[idx];

        int y = cur ? y_high : y_low;

        // horizontal segment
        plot_pixel(x_start + x, y, color);

        // vertical edge
        if (x > 0 && ((cur != prev) || (cur == prev))) {
            int y_prev = prev ? y_high : y_low;
            draw_vline(x_start + x, y_prev, y, color);
        }

        prev = cur;
    }
}
