#include <stdio.h>
#include <stdint.h>

// Addresses
#define LOGIC_ANALYZER_0_BASE 0x4000000
#define LEDS_BASE             0xff200000

int main() {
    // Volatile pointers to your specific offsets
    volatile uint32_t * const la_trig_cfg = (uint32_t *)(LOGIC_ANALYZER_0_BASE + 0x8); // Address 2
    volatile uint32_t * const led_ptr     = (uint32_t *)(LEDS_BASE);

    // Step 1: Turn on 1 LED to show the program started
    *led_ptr = 0x1;

    // Step 2: Write a test pattern to your trigger_config register
    uint32_t test_pattern = 0xA; // Binary 1010
    *la_trig_cfg = test_pattern;

    // Small delay to let the bus settle (though usually not needed)
    for(volatile int i = 0; i < 1000; i++);

    // Step 3: Read it back
    uint32_t read_back = *la_trig_cfg;

    // Step 4: Display the result on LEDs
    if (read_back == test_pattern) {
        // SUCCESS: Turn on all LEDs
        *led_ptr = 0x3FF;
    } else if (read_back == 0xDEADBEEF) {
        // BUS REACHED BUT ERROR: Turn on middle LEDs
        *led_ptr = 0x0F0;
    } else {
        // FAILURE/MISMATCH: Turn on alternating LEDs
        *led_ptr = 0x155;
    }

    while(1); // Halt here
    return 0;
}
