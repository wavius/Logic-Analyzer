#include "test_la_c.h"

#include <string.h>

// ---------------------------------------------------------------------------
//  Internal state
// ---------------------------------------------------------------------------
static uint16_t test_buffer[BUFFER_SIZE];  // packed: bit i of sample[s] = channel i
static bool buffer_ready = false;
static int trigger_channel = 0;
static int trigger_index = 0;

// ---------------------------------------------------------------------------
//  Waveform generators
//  Each returns 0 or 1 for a given sample index s.
// ---------------------------------------------------------------------------

// Square wave: period in samples
static int square(int s, int period) {
    return (s % period) < (period / 2) ? 1 : 0;
}

// Single pulse: high for `width` samples starting at `offset`
static int pulse(int s, int offset, int width) {
    return (s >= offset && s < offset + width) ? 1 : 0;
}

// Clock-like burst: active only during [burst_start, burst_end)
static int burst(int s, int period, int burst_start, int burst_end) {
    if (s < burst_start || s >= burst_end) return 0;
    return square(s, period);
}

// Simple PWM: 30 % duty cycle
static int pwm(int s, int period) {
    return (s % period) < (period * 3 / 10) ? 1 : 0;
}

// Alternating 1-0 (fastest possible toggle each sample)
static int toggle(int s) {
    return s & 1;
}

// Always high / always low
static int constant(int val) { return val; }

// ---------------------------------------------------------------------------
//  Build the packed buffer
// ---------------------------------------------------------------------------
static void generate_signals(void) {
    for (int s = 0; s < BUFFER_SIZE; s++) {
        uint16_t packed = 0;

        // CH0  — slow square wave (period 512)
        packed |= (uint16_t)(square(s, 512)) << 0;
        // CH1  — medium square wave (period 128)
        packed |= (uint16_t)(square(s, 128)) << 1;
        // CH2  — fast square wave (period 32)
        packed |= (uint16_t)(square(s, 32)) << 2;
        // CH3  — very fast toggle (period 2)
        packed |= (uint16_t)(toggle(s)) << 3;
        // CH4  — single wide pulse in the middle of the buffer
        packed |= (uint16_t)(pulse(s, 1800, 400)) << 4;
        // CH5  — three narrow pulses
        packed |= (uint16_t)(pulse(s, 200, 20) | pulse(s, 600, 20) | pulse(s, 1000, 20)) << 5;
        // CH6  — PWM 30 % duty, period 64
        packed |= (uint16_t)(pwm(s, 64)) << 6;
        // CH7  — burst: fast clock only in first quarter of buffer
        packed |= (uint16_t)(burst(s, 16, 0, BUFFER_SIZE / 4)) << 7;
        // CH8  — slow square offset by half period (inverted CH0)
        packed |= (uint16_t)(!square(s, 512)) << 8;
        // CH9  — square wave period 256
        packed |= (uint16_t)(square(s, 256)) << 9;
        // CH10 — always high
        packed |= (uint16_t)(constant(1)) << 10;
        // CH11 — always low
        packed |= (uint16_t)(constant(0)) << 11;
        // CH12 — slow PWM 70 % duty
        packed |= (uint16_t)(pwm(s, 256)) << 12;
        // CH13 — medium pulse train (period 200, width 40)
        packed |= (uint16_t)((s % 200) < 40 ? 1 : 0) << 13;
        // CH14 — staircase approximation (steps of 256 samples, cycling 0/1)
        packed |= (uint16_t)((s / 256) & 1) << 14;
        // CH15 — inverted CH2
        packed |= (uint16_t)(!square(s, 32)) << 15;

        test_buffer[s] = packed;
    }
}

// Find the first rising edge on `channel` and store the sample index.
static void compute_trigger_index(void) {
    trigger_index = 0;  // fallback: start of buffer

    for (int s = 1; s < BUFFER_SIZE; s++) {
        int prev = (test_buffer[s - 1] >> trigger_channel) & 1;
        int curr = (test_buffer[s] >> trigger_channel) & 1;
        if (!prev && curr) {  // rising edge
            trigger_index = s;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
//  Visible Functions
// ---------------------------------------------------------------------------
void la_set_trigger_channel(int channel) {
    if (channel >= 0 && channel < 16)
        trigger_channel = channel;
}

void la_start(void) {
    generate_signals();
    compute_trigger_index();
    buffer_ready = true;
}

bool la_is_done(void) {
    return buffer_ready;  // always ready instantly in test mode
}

void la_download_buffer(uint16_t* dst, int count) {
    if (!dst || count <= 0) return;
    if (count > BUFFER_SIZE) count = BUFFER_SIZE;
    memcpy(dst, test_buffer, count * sizeof(uint16_t));
}

void la_stop(void) {
    buffer_ready = false;  // reset so la_is_done() returns false until next la_start()
}

void la_reset_read_pointer(void) {
    // no-op: we use memcpy so there is no read pointer to reset
}

int la_get_trigger_index(void) {
    return trigger_index;
}