#include "io.h"

#include "address_map_niosV.h"

/********************************
 *  Helper Functions and stuff
 ********************************/
volatile uint32_t* led_ptr = (volatile uint32_t*)LEDR_BASE;

// returns active-low 7-seg encoding for the fpga
static uint8_t hex_encode_char(char c) {
    switch (c) {
        // digits
        case '0':
            return 0x40;
        case '1':
            return 0x79;
        case '2':
            return 0x24;
        case '3':
            return 0x30;
        case '4':
            return 0x19;
        case '5':
            return 0x12;
        case '6':
            return 0x02;
        case '7':
            return 0x78;
        case '8':
            return 0x00;
        case '9':
            return 0x10;

        // hex letters
        case 'A':
        case 'a':
            return 0x08;

        case 'B':
        case 'b':
            return 0x03;  // displayed like lowercase b

        case 'C':
        case 'c':
            return 0x46;

        case 'D':
        case 'd':
            return 0x21;  // displayed like lowercase d

        case 'E':
        case 'e':
            return 0x06;

        case 'F':
        case 'f':
            return 0x0E;

        // a few extra useful ones
        case 'H':
        case 'h':
            return 0x0B;

        case 'L':
        case 'l':
            return 0x47;

        case 'P':
        case 'p':
            return 0x0C;

        case 'U':
        case 'u':
            return 0x41;

        case 'Y':
        case 'y':
            return 0x11;

        case '-':
            return 0x3F;
        case '_':
            return 0x77;
        case ' ':
            return 0x7F;  // blank, all segments off

        default:
            return 0x7F;  // unsupported char -> blank
    }
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
    if (hex_index < 0 || hex_index > 5)
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