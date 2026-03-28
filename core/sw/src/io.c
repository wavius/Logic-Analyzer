#include "io.h"

#include "address_map_niosV.h"

/********************************
 *  Helper Functions and stuff
 ********************************/
volatile uint32_t* led_ptr = (volatile uint32_t*)LEDR_BASE;

// returns active-low 7-seg encoding for the fpga
static uint8_t hex_encode_char(char c) {
    uint8_t seg;
    switch (c) {
        // digits
        case '0':
            seg = 0x3F;
            break;  // abcdef
        case '1':
            seg = 0x06;
            break;  // bc
        case '2':
            seg = 0x5B;
            break;  // abdeg
        case '3':
            seg = 0x4F;
            break;  // abcdg
        case '4':
            seg = 0x66;
            break;  // bcfg
        case '5':
            seg = 0x6D;
            break;  // acdfg
        case '6':
            seg = 0x7D;
            break;  // acdefg
        case '7':
            seg = 0x07;
            break;  // abc
        case '8':
            seg = 0x7F;
            break;  // abcdefg
        case '9':
            seg = 0x6F;
            break;  // abcdfg

        // hex letters
        case 'A':
        case 'a':
            seg = 0x77;
            break;  // abcefg

        case 'B':
        case 'b':
            seg = 0x7C;
            break;  // cdefg

        case 'C':
        case 'c':
            seg = 0x39;
            break;  // adef

        case 'D':
        case 'd':
            seg = 0x5E;
            break;  // bcdeg

        case 'E':
        case 'e':
            seg = 0x79;
            break;  // adefg

        case 'F':
        case 'f':
            seg = 0x71;
            break;  // aefg

        // extra letters that look decent on 7-seg
        case 'H':
        case 'h':
            seg = 0x76;
            break;  // bcefg

        case 'L':
        case 'l':
            seg = 0x38;
            break;  // def

        case 'P':
        case 'p':
            seg = 0x73;
            break;  // abefg

        case 'U':
        case 'u':
            seg = 0x3E;
            break;  // bcdef

        case 'Y':
        case 'y':
            seg = 0x6E;
            break;  // bcdfg

        case '-':
            seg = 0x40;
            break;  // g
        case '_':
            seg = 0x08;
            break;  // d
        case ' ':
            seg = 0x00;
            break;  // blank

        default:
            seg = 0x00;
            break;  // unsupported -> blank
    }
    return seg;
}

/********************************
 * Visible Functions
 ********************************/
// put values on leds
void put_on_leds(uint32_t led_val) {
    *led_ptr = led_val;
}

// write one character to one HEX display
void hex_write_char(int hex_index, char c) {
    if (hex_index < 0 || hex_index > 5)  // options of hex displays to put stuff on is restricted
        return;

    uint8_t seg = hex_encode_char(c);

    if (hex_index <= 3) {
        volatile uint32_t* hex30 = (volatile uint32_t*)HEX3_HEX0_BASE;
        uint32_t shift = hex_index * 8;
        uint32_t mask = 0xFFu << shift;

        uint32_t value = *hex30;
        value &= ~mask;
        value |= ((uint32_t)seg << shift);
        *hex30 = value;
    } else {
        volatile uint32_t* hex54 = (volatile uint32_t*)HEX5_HEX4_BASE;
        uint32_t shift = (hex_index - 4) * 8;
        uint32_t mask = 0xFFu << shift;

        uint32_t value = *hex54;
        value &= ~mask;
        value |= ((uint32_t)seg << shift);
        *hex54 = value;
    }
}

// clear one HEX display
void hex_clear_digit(int hex_index) {
    hex_write_char(hex_index, ' ');
}

// clear all HEX displays
void hex_clear_all(void) {
    volatile uint32_t* hex30 = (volatile uint32_t*)HEX3_HEX0_BASE;
    volatile uint32_t* hex54 = (volatile uint32_t*)HEX5_HEX4_BASE;

    *hex30 = 0x7F7F7F7F;
    *hex54 = 0x00007F7F;
}