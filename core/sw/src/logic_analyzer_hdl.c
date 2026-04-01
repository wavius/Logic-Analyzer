///////////////////////////////////////////
// HAL for Logic Analyzer Verilog Module //
///////////////////////////////////////////

#ifdef USE_HW
#include "logic_analyzer.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

void la_set_trigger_channel(uint16_t channel) {
    LA->TRIGGER_CFG = (uint32_t)channel;
}

void la_start(void) {
    LA->CONTROL = 0x0;  // Write 0 to ensure FSM is reset
    LA->CONTROL = 0x1
    ;  // Write 1 to start capture
}

void la_stop(void) {
    LA->CONTROL = 0x0;
}

int la_is_done(void) {
    // Verilog: readdata = {29'b0, triggered, buffer_full, run};
    // Bit 1 is buffer_full.
    return (LA->STATUS & 0x2) ? 1 : 0;
}

void la_reset_read_pointer(void) {
    // Verilog: if (write && address == 3'h3) read_pointer <= '0;
    LA->DATA_WINDOW = 0x0;
}

void la_download_buffer(uint16_t* dest, int size) {
    // get raw hardware trigger data
    uint32_t trigger_idx = LA->TRIGGER_PTR;
    uint16_t post_count, pre_count;
    la_get_trigger_samples(&post_count, &pre_count);

    // calculate the exact hardware stop index in the circular buffer
    uint32_t stop_ptr = (trigger_idx + post_count) % size;

    // reset hardware read pointer and download raw data
    la_reset_read_pointer();
    
    static uint16_t temp_raw[4096]; 
    for (int i = 0; i < size; i++) {
        // The read_pointer increments automatically on every read of address 0xC.
        temp_raw[i] = (uint16_t)(LA->DATA_WINDOW & 0xFFFF);
    }

    // linearize the buffer
    for (int i = 0; i < size; i++) {
        uint32_t circular_idx = (stop_ptr + 1 + i) % size;
        dest[i] = temp_raw[circular_idx];
    }
}

uint32_t la_get_trigger_index(void) {
    uint16_t post_count, pre_count;
    la_get_trigger_samples(&post_count, &pre_count);
    uint32_t software_trigger_idx = BUFFER_SIZE - post_count - 1;

    return software_trigger_idx;
}

void la_get_trigger_samples(uint16_t* post_trigger, uint16_t* pre_trigger) {
    uint32_t samples = LA->TRIGGER_SAMPLES;
    *post_trigger = (uint16_t)(samples >> 16);
    *pre_trigger = (uint16_t)samples;
}
#endif // USE_HW