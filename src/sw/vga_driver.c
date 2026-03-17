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
    volatile uint32_t frontBuffer;  // Address: Base + 0
    volatile uint32_t backBuffer;   // Address: Base + 4
    volatile uint32_t resolution;   // Address: Base + 8
    volatile uint32_t statusReg;    // Address: Base + 12
};

/********************************
 *  Global Variables
 ********************************/
static volatile struct frame_buffer_controller* frameBuf = (struct frame_buffer_controller*)FPGA_PIXEL_BUF_BASE;
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
    frameBuf->backBuffer = (uint32_t)Buffer1;
    wait_for_vsync();

    // Set drawing pointer to front buffer and clear it
    pixel_buffer_start = frameBuf->frontBuffer;
    clear_screen();

    // Intialize back buffer as Buffer2
    frameBuf->backBuffer = (uint32_t)Buffer2;

    //  clear the back buffer
    pixel_buffer_start = frameBuf->backBuffer;
    clear_screen();
}

// Plot a single pixel
void plot_pixel(int x, int y, uint16_t color) {
    volatile uint16_t* pixel_addr;
    pixel_addr = (volatile uint16_t*)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

// Wait for vertical sync and swap buffers
void wait_for_vsync() {
    frameBuf->frontBuffer = 1;        // request swap
    while (frameBuf->statusReg & 1);  // wait for completion

    // After swap, update draw pointer to new back buffer
    pixel_buffer_start = frameBuf->backBuffer;
}

// Clear entire screen
void clear_screen() {
    // 240 rows × 1024 bytes per row = 245760 bytes
    memset((void*)pixel_buffer_start, 0, 245760);
}