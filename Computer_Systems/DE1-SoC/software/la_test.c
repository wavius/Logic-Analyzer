// Tests read and write functionality of Logic Analyzer module using the VGA as indication
// Waits for trigger on channel 1 with black screen
// Once buffer is full post trigger, screen turns red

#include <stdbool.h>
#include <stdlib.h>

// Function prototypes
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_red();
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void swap(int* a, int* b);

// Global variables
int pixel_buffer_start;

struct LA_Controller {
    volatile int CONTROL;       // Offset 0x00: [0]=Run
    volatile int STATUS;        // Offset 0x04: [0]=Run, [1]=Full, [2]=Triggered
    volatile int TRIGGER_CFG;   // Offset 0x08: [15:0]=Trigger Channel (1-16)
    volatile int DATA_WINDOW;   // Offset 0x0C: Read-only Buffer / Write-to-Reset Ptr
    volatile int TRIGGER_PTR;   // Offset 0x10: Buffer index where trigger fired
    volatile int TRIGGER_COUNTS;// Offset 0x14: [31:16]=Post-count, [15:0]=Pre-count
};

// Main
int main(void)
{
    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    volatile struct LA_Controller* la_ptr = (struct LA_Controller*) 0xFF205000;

    clear_screen();
    la_ptr->TRIGGER_CFG = 0x0;
    la_ptr->CONTROL	    = 0x1;

    while(1) {
        if ((la_ptr->STATUS & 0b100) == 0b100) {
            draw_red();
        }
        else {
            clear_screen();
        }
    }
    return 0;
}

// Function definitions
void plot_pixel(int x, int y, short int line_color) {
    volatile short int *one_pixel_address;

    one_pixel_address = (short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *one_pixel_address = line_color;
}

void clear_screen() {
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, 0); // Black
        }
    }
}

void draw_red() {
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, 0xF800); // Black
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color) {
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = -(dx / 2);
    int y = y0;
    int y_step = y0 < y1 ? 1 : -1;

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, line_color);
        } else {
            plot_pixel(x, y, line_color);
        }
        error = error + dy;
        if (error > 0) {
            y += y_step;
            error -= dx;
        }
    }
}

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
