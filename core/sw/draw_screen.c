#include "draw_screen.h"

#include "vga_driver.h"

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
void draw_hline(int x_start, int x_end, int y, uint16_t color);
void draw_vline(int x, int y_start, int y_end, uint16_t color);
void fill_rect(int x, int y, int w, int h, uint16_t color);
void draw_text();  // temp, unfinished

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
    for (int y = y_start; y <= y_end; y++)
        plot_pixel(x, y, color);
}

// draw a filled rectangle (to draw bars / labels)
void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x + i, y + j, color);
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
void draw_logic_ui_frame() {
    // Top bar
    fill_rect(0, 0, SCREEN_W, 20, 0x39E7);  // grey

    // Left label column
    fill_rect(0, 20, 52, SCREEN_H - 20, 0x2104);  // darker grey

    // Channel separators (4 lanes)
    for (int i = 0; i < 4; i++) {
        int y = 20 + i * 50;
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
        if (x > 0 && cur != prev) {
            int y_prev = prev ? y_high : y_low;
            draw_vline(x_start + x, y_prev, y, color);
        }

        prev = cur;
    }
}
