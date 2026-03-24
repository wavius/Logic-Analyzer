#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#include <stdint.h>
#include "address_map_niosV.h"

/* --- Hardware Register Mapping --- */

/**
 * Logic Analyzer Register Map
 * Matches the 'case' statements in signal_capture.v
 */
typedef struct {
    volatile uint32_t CONTROL;        // Offset 0x00: [0] = Run
    volatile uint32_t STATUS;         // Offset 0x04: [0] = Run, [1] = Full, [2] = Triggered
    volatile uint32_t TRIGGER_CFG;    // Offset 0x08: [15:0] = Trigger Channel (1-16)
    volatile uint32_t DATA_WINDOW;    // Offset 0x0C: Read-only Buffer / Write-to-Reset Ptr
    volatile uint32_t TRIGGER_PTR;    // Offset 0x10: Buffer index where trigger fired
    volatile uint32_t TRIGGER_SAMPLES;// Offset 0x14: [31:16] = Post-samples, [15:0] = Pre-samples
} LA_t;

// Pointer to the hardware
#define LA ((LA_t*) LA_BASE)

/* --- Function Prototypes --- */

/**
 * @brief Sets the channel to monitor for a rising edge.
 * @param channel The channel index (1 to 16).
 */
void la_set_trigger_channel(uint16_t channel);

/**
 * @brief Starts the capture process.
 */
void la_start(void);

/**
 * @brief Stops the capture process and resets the FSM.
 */
void la_stop(void);

/**
 * @brief Checks if the capture buffer is completely full.
 * @return 1 if done, 0 otherwise.
 */
int la_is_done(void);

/**
 * @brief Resets the internal read pointer of the hardware buffer to 0.
 */
void la_reset_read_pointer(void);

/**
 * @brief Downloads the entire captured buffer from the FPGA RAM to CPU memory.
 * @param dest Pointer to a 16-bit array to store the data.
 * @param size Number of samples to read (usually BUFFER_SIZE).
 */
void la_download_buffer(uint16_t *dest, int size);

/**
 * @brief Gets the exact buffer index where the trigger occurred.
 */
uint32_t la_get_trigger_index(void);

/**
 * @brief Gets the number of post trigger and pre trigger samples.
 */
void la_get_trigger_samples(uint16_t *post_trigger, uint16_t *pre_trigger);

#endif // LOGIC_ANALYZER_H