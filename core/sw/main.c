#include <stdint.h>

#include "draw_screen.h"
#include "vga_driver.h"

typedef struct {
    uint16_t ch0;
    uint16_t ch1;
    uint16_t ch2;
    uint16_t ch3;
    uint16_t ch4;
    uint16_t ch5;
    uint16_t ch6;
    uint16_t ch7;
    uint16_t ch8;
    uint16_t ch9;
    uint16_t ch10;
    uint16_t ch11;
    uint16_t ch12;
    uint16_t ch13;
    uint16_t ch14;
    uint16_t ch15;
} Channel_Colors;

const Channel_Colors colors = {
    .ch0 = 0x5D6B,
    .ch1 = 0x8E24,
    .ch2 = 0x8E24,
    .ch3 = 0xC7C0,
    .ch4 = 0xE7E0,
    .ch5 = 0xF580,
    .ch6 = 0xE3A0,
    .ch7 = 0xD820,
    .ch8 = 0x72A9,
    .ch9 = 0x5249,
    .ch10 = 0x3A89,
    .ch11 = 0x44CB,
    .ch12 = 0x5CFE,
    .ch13 = 0x65FF,
    .ch14 = 0x65D7,
    .ch15 = 0x61ED};

int main(void) {
    vga_init();  // must always be done first
    const int lanes = 5;

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
        clear_screen();  // should update this erase only whats needed (unless memset = O(1)?)

        draw_logic_ui_frame(lanes);  // needs modifications

        // Waveform area starts after label column
        int x0 = 54;
        int w = 320 - x0 - 2;

        draw_digital_waveform(ch0, 64, x0, 20, w, 50, colors.ch0);
        draw_digital_waveform(ch1, 64, x0, 70, w, 50, colors.ch1);
        draw_digital_waveform(ch2, 64, x0, 120, w, 50, colors.ch2);
        draw_digital_waveform(ch3, 64, x0, 170, w, 50, colors.ch3);
        draw_digital_waveform(ch3, 64, x0, 170, w, 50, colors.ch4);  // try drawing 5 chanels

        wait_for_vsync();
    }

    return 0;
}
