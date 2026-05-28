#ifndef FILTER_K_H
#define FILTER_K_H

#include <stdint.h>

/**
 * @defgroup Mean_Filter Moving Average Filter
 * @{
 */

#define MEAN_FILTER_SIZE 10

/** @brief Moving average filter instance (integer/fixed-point) */
typedef struct {
    int16_t buffer[MEAN_FILTER_SIZE];  /**< Circular buffer for raw ADC samples */
    uint16_t index;                    /**< Current buffer index */
    int32_t sum;                       /**< Running sum of buffer values */
    uint8_t flag;                      /**< Initialization flag */
    int32_t filter_out;                /**< Current filter output (raw ADC, 4-byte aligns) */
} mean_filter_t;

/**
 * @brief Initialize moving average filter
 * @param filter Pointer to mean filter instance
 */
void Mean_Filter_Init(mean_filter_t *filter);

/**
 * @brief Update moving average filter with new sample
 * @param filter Pointer to mean filter instance
 * @param data New input sample (int16_t, e.g. raw ADC value)
 */
void Mean_Filter_Update(mean_filter_t *filter, int16_t data);

/** @} */

#endif /* FILTER_K_H */
