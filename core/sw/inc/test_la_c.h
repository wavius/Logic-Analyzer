#ifndef TEST_LA_C_H
#define TEST_LA_C_H

#include <stdbool.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
//  Drop-in replacements for the custom LA hardware module
//  Include this instead of real LA header for CPUlator
//  All signals are synthetically generated in software
// ---------------------------------------------------------------------------

#define BUFFER_SIZE 4096

// Call once before la_start() to choose which channel edge triggers capture.
void la_set_trigger_channel(int channel);

// "Arms" the logic analyzer — in test mode this pre-fills the internal buffer
// with generated waveforms immediately.
void la_start(void);

// Always returns true in test mode (capture is instant).
bool la_is_done(void);

// Copies the generated buffer into dst (up to `count` samples).
void la_download_buffer(uint16_t* dst, int count);

// No-ops in test mode — kept so interface.c compiles unchanged.
void la_stop(void);
void la_reset_read_pointer(void);

// Returns the sample index of the first rising edge on the trigger channel.
int la_get_trigger_index(void);

#endif  // TEST_LA_C_H