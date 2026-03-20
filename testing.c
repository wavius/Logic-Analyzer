/*THIS FILE IS ONLY MEANT TO PROVIDE A SPACE TO SAVE CPULATOR TESTING. NOT FOR ACTUAL PROGRAM */
#include <stdbool.h>  // for boolean values
#include <stdint.h>   // for the sake of my struct
#include <stdlib.h>   // for abs val func.
#include <string.h>   //for memset

// ----- constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240
#define FRAME_BUF_CTRL_BASE 0xFF203020

/********************************
 *  Function Declarations
 ********************************/
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();
void draw_hline(int x1, int x2, int y, uint16_t color);
void draw_vline(int x, int y1, int y2, uint16_t color);
void fill_rect(int x, int y, int w, int h, uint16_t color);
void draw_logic_ui_frame();
void draw_digital_waveform(const uint8_t* samples, int count, int x0, int y0, int w, int h, uint16_t color);

/********************************
 *  Structs
 ********************************/
struct frame_buffer_controller {
    volatile uint32_t frontBuffer;
    volatile uint32_t backBuffer;
    volatile uint32_t resolution;
    volatile uint32_t statusReg;
};

/********************************
 *  Global Variables
 ********************************/
static volatile struct frame_buffer_controller* frameBuf = (struct frame_buffer_controller*)FRAME_BUF_CTRL_BASE;
static volatile int pixel_buffer_start;

// ----- Two frame buffers (double buffering) ----- //
static uint16_t Buffer1[240][512];
static uint16_t Buffer2[240][512];

/********************************
 *  Function Declarations
 ********************************/
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();

/********************************
 *  Function Implementations (FROM VGA DRIVER)
 ********************************/
int main(void) {
    vga_init();  // must always be done first
    // ----- Fake sample data (0 = low, 1 = high) ----- //
    static uint8_t ch0[64] = {
        0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0,
        0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
        0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0,
        1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0};
    static uint8_t ch1[64] = {
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
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
    while (1) {
        clear_screen();
        draw_logic_ui_frame();
        // Waveform area starts after label column
        int x0 = 54;
        int w = 320 - x0 - 2;
        draw_digital_waveform(ch0, 64, x0, 20, w, 50, 0xFFFF);
        draw_digital_waveform(ch1, 64, x0, 70, w, 50, 0xF800);
        draw_digital_waveform(ch2, 64, x0, 120, w, 50, 0x07E0);
        draw_digital_waveform(ch3, 64, x0, 170, w, 50, 0x001F);
        wait_for_vsync();
    }
    return 0;
}

// ----- Core VGA ----- //
void vga_init() {
    frameBuf->backBuffer = (uint32_t)Buffer1;
    wait_for_vsync();

    pixel_buffer_start = frameBuf->frontBuffer;
    clear_screen();

    frameBuf->backBuffer = (uint32_t)Buffer2;

    pixel_buffer_start = frameBuf->backBuffer;
    clear_screen();
}

void wait_for_vsync() {
    frameBuf->frontBuffer = 1;                  // request swap
    while (frameBuf->statusReg & 1);            // poll the status bit to see if the swap is done
    pixel_buffer_start = frameBuf->backBuffer;  // set a new main buffer to write into
}

void clear_screen() {
    memset((void*)pixel_buffer_start, 0, 245760);
}

// ----- Basic Drawing Stuff ----- //
void plot_pixel(int x, int y, uint16_t color) {
    volatile uint16_t* pixel_addr;
    pixel_addr = (volatile uint16_t*)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

void draw_hline(int x1, int x2, int y, uint16_t color) {
    for (int x = x1; x <= x2; x++)
        plot_pixel(x, y, color);
}

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

void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x + i, y + j, color);
}

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame() {
    fill_rect(0, 0, SCREEN_W, 20, 0x39E7);  // grey

    fill_rect(0, 20, 52, SCREEN_H - 20, 0x2104);  // darker grey

    for (int i = 0; i < 4; i++) {
        int y = 20 + i * 50;
        draw_hline(0, SCREEN_W - 1, y, 0xFFFF);  // white line
    }

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
