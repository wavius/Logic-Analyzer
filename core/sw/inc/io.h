#ifndef IO_H
#define IO_H

#include <stdint.h>

// -- Functions -- //
void hex_write_char(int hex_index, char c);
void hex_clear_digit(int hex_index);
void hex_clear_all(void);
void put_on_leds(uint32_t led_val);

#endif