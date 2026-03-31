#include <string.h>

#include "logic_analyzer_sw.h"

// ---------------------------------------------------------------------------
//  Hardware Register Mapping
// ---------------------------------------------------------------------------
#define GPIO_DATA_ADDR 0xFF200060

// ---------------------------------------------------------------------------
//  Internal state
// ---------------------------------------------------------------------------
static uint16_t la_buffer[BUFFER_SIZE];
static bool buffer_ready = false;
static int trigger_channel = 0;
static int trigger_ptr = 0;
static uint16_t pre_count = 0;
static uint16_t post_count = 0;

// ---------------------------------------------------------------------------
//  Hardware Capture FSM
// ---------------------------------------------------------------------------
void la_start(void) {
    buffer_ready = false;
    int write_ptr = 0;
    bool triggered = false;
    pre_count = 0;
    post_count = 0;

    // Prime the edge detector with the current state of the pins
    uint16_t last_sample = (uint16_t)(*(volatile uint32_t*)GPIO_DATA_ADDR & 0xFFFF);

    // 1. PRE_TRIGGER Phase
    // Continuously poll the hardware and write to the circular buffer.
    while (!triggered) {
        // Read live pins
        uint16_t current_sample = (uint16_t)(*(volatile uint32_t*)GPIO_DATA_ADDR & 0xFFFF);
        la_buffer[write_ptr] = current_sample;

        // Edge detection on the selected channel
        bool prev_bit = (last_sample >> trigger_channel) & 1;
        bool curr_bit = (current_sample >> trigger_channel) & 1;

        // Trigger condition: Rising edge AND we have enough pre-trigger history
        if (!prev_bit && curr_bit && (pre_count >= BUFFER_SIZE / 2)) {
            triggered = true;
            trigger_ptr = write_ptr;
        }

        last_sample = current_sample;
        write_ptr = (write_ptr + 1) % BUFFER_SIZE;

        if (pre_count < (BUFFER_SIZE / 2)) {
            pre_count++;
        }
    }

    // 2. POST_TRIGGER Phase
    // Continue polling until the remaining buffer space is filled.
    int post_length = BUFFER_SIZE - pre_count;
    while (post_count < post_length) {
        la_buffer[write_ptr] = (uint16_t)(*(volatile uint32_t*)GPIO_DATA_ADDR & 0xFFFF);
        write_ptr = (write_ptr + 1) % BUFFER_SIZE;
        post_count++;
    }

    buffer_ready = true;
}

// ---------------------------------------------------------------------------
//  Visible Driver API
// ---------------------------------------------------------------------------

void la_set_trigger_channel(int channel) {
    trigger_channel = (channel & 0xF);
}

bool la_is_done(void) {
    return buffer_ready;
}

void la_stop(void) {
    buffer_ready = false;
}

int la_get_trigger_index(void) {
    return pre_count;
}

void la_get_trigger_samples(uint16_t* post, uint16_t* pre) {
    if (post) *post = post_count;
    if (pre) *pre = pre_count;
}

void la_download_buffer(uint16_t* dst, int count) {
    if (!dst || count <= 0) return;
    if (count > BUFFER_SIZE) count = BUFFER_SIZE;

    // Calculate the start of the window (oldest data) to unpack the circular buffer
    // so the downloaded array flows chronologically from index 0.
    int start_idx = (trigger_ptr - pre_count + BUFFER_SIZE) % BUFFER_SIZE;

    for (int i = 0; i < count; i++) {
        dst[i] = la_buffer[(start_idx + i) % BUFFER_SIZE];
    }
}

// ---------------------------------------------------------------------------
//  Legacy UI Support
// ---------------------------------------------------------------------------
void la_reset_read_pointer(void) {
    // No-op: The new circular buffer logic handles chronological unrolling
    // automatically inside la_download_buffer().
    // This is just here to prevent linker errors from older interface code.
}