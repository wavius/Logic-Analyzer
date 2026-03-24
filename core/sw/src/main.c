#include <stdbool.h>
#include <stdint.h>

#include "address_map_niosV.h"
#include "draw_screen.h"
#include "ps2_input.h"
#include "vga_driver.h"
#include "visualizer_logic.h"

#define LED_UP_MASK (1 << 0)
#define LED_DOWN_MASK (1 << 1)
#define LED_LEFT_MASK (1 << 2)
#define LED_RIGHT_MASK (1 << 3)

/********************************
 *  Structs + global variables
 ********************************/

/********************************
 *  Main Program
 ********************************/
// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
static volatile bool up_down = false;
static volatile bool down_down = false;
static volatile bool left_down = false;
static volatile bool right_down = false;
int main(void) {
    vga_init();  // must always be done first
                 /*
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
                 */
    Keyboard kb;
    KeyEvent ev;

    keyboard_init(&kb);
    volatile uint32_t* led_ptr = (volatile uint32_t*)LEDR_BASE;
    uint32_t led_val = 0;

    *led_ptr = led_val;  // set to 0 at first

    while (1) {
        if (keyboard_read_event(&kb, &ev)) {
            if (ev.key == KEY_UP) {
                up_down = ev.pressed;
            } else if (ev.key == KEY_DOWN) {
                down_down = ev.pressed;
            } else if (ev.key == KEY_LEFT) {
                left_down = ev.pressed;
            } else if (ev.key == KEY_RIGHT) {
                right_down = ev.pressed;
            }
        }
        if (up_down)
            led_val = 0b1;
        else if (down_down)
            led_val = 0b11;
        else if (left_down)
            led_val = 0b11;
        else if (right_down)
            led_val = 0b11;
        else
            led_val = 0;

        *led_ptr = led_val;
    }

    return 0;
}
