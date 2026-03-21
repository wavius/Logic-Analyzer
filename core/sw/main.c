#include <stdint.h>

#include "draw_screen.h"
#include "vga_driver.h"
#include "visualizer_logic.h"

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
        draw_logic_ui_frame(lanes);
        // draw_signals(channels, lanes);
        draw_logic_view(&view, channels, lanes);
        wait_for_vsync();
        view.start_sample++;
    }

    return 0;
}