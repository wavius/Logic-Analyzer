///////////////////////////////////////////
// HAL for Logic Analyzer Verilog Module //
///////////////////////////////////////////

#ifdef USE_HW
#include "logic_analyzer.h"

void la_set_trigger_channel(uint16_t channel) {
    // Pass channel - 1 to account for Verilog [15:0] indexing
    LA->TRIGGER_CFG = (uint32_t)channel;
}

void la_start(void) {
    LA->CONTROL = 0x0;  // Write 0 to ensure FSM is reset
    LA->CONTROL = 0x1;  // Write 1 to start capture
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
    la_reset_read_pointer();
    for (int i = 0; i < size; i++) {
        // Verilog: readdata = {16'b0, buffer[read_pointer]};
        // The read_pointer increments automatically on every read of address 0xC.
        dest[i] = (uint16_t)(LA->DATA_WINDOW & 0xFFFF);
    }
}

uint32_t la_get_trigger_index(void) {
    return LA->TRIGGER_PTR;
}

void la_get_trigger_samples(uint16_t* post_trigger, uint16_t* pre_trigger) {
    uint32_t samples = LA->TRIGGER_SAMPLES;
    *post_trigger = (uint16_t)(samples >> 16);
    *pre_trigger = (uint16_t)samples;
}
#endif // USE_HW