//////////////////////////////////////////////////////////
// Soft implementation of Logic Analyzer Verilog Module //
//////////////////////////////////////////////////////////

#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#include <stdbool.h>
#include <stdint.h>

// Assuming BUFFER_SIZE is defined in your global constants
#include "constants.h"

// ---------------------------------------------------------------------------
//  Logic Analyzer Driver API
// ---------------------------------------------------------------------------

/**
 * @brief Sets the channel (0-15) to monitor for a rising edge trigger.
 * * @param channel The pin index on the JP1 header to trigger on.
 */
void la_set_trigger_channel(int channel);

/**
 * @brief Starts the hardware capture.
 * @note This is a BLOCKING function. It will poll the hardware pins continuously
 * until a rising edge is detected on the trigger channel and the buffer fills.
 */
void la_start(void);

/**
 * @brief Checks if the capture FSM has finished and data is ready.
 * * @return true if the buffer is full and ready for download, false otherwise.
 */
bool la_is_done(void);

/**
 * @brief Clears the ready flag, allowing the analyzer to be started again.
 */
void la_stop(void);

/**
 * @brief Gets the raw internal buffer index where the trigger event occurred.
 * * @return The index in the circular buffer.
 */
int la_get_trigger_index(void);

/**
 * @brief Retrieves the exact count of samples captured before and after the trigger.
 * * @param post Pointer to store the number of post-trigger samples.
 * @param pre  Pointer to store the number of pre-trigger samples.
 */
void la_get_trigger_samples(uint16_t* post, uint16_t* pre);

/**
 * @brief Copies the captured data into the provided destination array.
 * The data is automatically unrolled from the circular buffer so that
 * dst[0] is the oldest chronological sample.
 * * @param dst   Pointer to the destination array.
 * @param count Number of samples to read (usually BUFFER_SIZE).
 */
void la_download_buffer(uint16_t* dst, int count);

void la_reset_read_pointer(void);

#endif  // LOGIC_ANALYZER_H