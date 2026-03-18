#include "vga_driver.h"

#include <string.h>  //for memset

#include "../../DE1-SoC/software/address_map_niosV.h"

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

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
static volatile struct frame_buffer_controller* frameBuf =
    (struct frame_buffer_controller*)PIXEL_BUF_CTRL_BASE;
static volatile int pixel_buffer_start;

// ----- Two frame buffers (double buffering) ----- //
static uint16_t Buffer1[240][512];
static uint16_t Buffer2[240][512];

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

// draw a horizontal line
void draw_hline(int x1, int x2, int y, uint16_t color) {
    for (int x = x1; x <= x2; x++)
        plot_pixel(x, y, color);
}

// draw a vertical line
void draw_vline(int x, int y1, int y2, uint16_t color) {
    for (int y = y1; y <= y2; y++)
        plot_pixel(x, y, color);
}

// draw a filled rectangle (to draw bars / labels)
void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x + i, y + j, color);
}

/********************************
 * Major functions
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

void draw_digital_waveform(const uint8_t* samples, int count, int x0, int y0, int w, int h, uint16_t color) {
    int y_high = y0 + 10;
    int y_low = y0 + h - 10;

    int prev = samples[0];

    for (int x = 0; x < w; x++) {
        int idx = (x * count) / w;
        int cur = samples[idx];

        int y = cur ? y_high : y_low;

        // horizontal segment
        plot_pixel(x0 + x, y, color);

        // vertical edge
        if (x > 0 && cur != prev) {
            int y_prev = prev ? y_high : y_low;
            draw_vline(x0 + x, y_prev, y, color);
        }

        prev = cur;
    }
}

/*
// implemented for debugging purposes
int getXres() {
    return frameBuf->xRes;
}
*/
