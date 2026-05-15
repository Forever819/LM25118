#ifndef FILTER_K_H
#define FILTER_K_H

#include <stdint.h>

/**
 * @defgroup IIR_Filter IIR Low-pass Filter
 * @{
 */

/** @brief IIR filter instance */
typedef struct {
    float alpha;        /**< Filter coefficient (0-1), higher = more filtering */
    float filter_out;   /**< Current filter output */
    uint8_t flag;       /**< Initialization flag */
} iir_filter_t;

/**
 * @brief Initialize IIR filter
 * @param iir Pointer to IIR filter instance
 * @param alpha Filter coefficient (0.0-1.0, typically 0.8-0.95)
 */
void IIR_Filter_Init(iir_filter_t *iir, float alpha);

/**
 * @brief Update IIR filter with new sample
 * @param iir Pointer to IIR filter instance
 * @param data New input sample
 */
void IIR_Filter_Update(iir_filter_t *iir, uint16_t data);

/** @} */

/**
 * @defgroup Mean_Filter Moving Average Filter
 * @{
 */

#define MEAN_FILTER_SIZE 10

/** @brief Moving average filter instance */
typedef struct {
    float buffer[MEAN_FILTER_SIZE];  /**< Circular buffer for samples */
    uint16_t index;                  /**< Current buffer index */
    float sum;                       /**< Running sum of buffer values */
    uint8_t flag;                    /**< Initialization flag */
    float filter_out;                /**< Current filter output */
} mean_filter_t;

/**
 * @brief Initialize moving average filter
 * @param filter Pointer to mean filter instance
 */
void Mean_Filter_Init(mean_filter_t *filter);

/**
 * @brief Update moving average filter with new sample
 * @param filter Pointer to mean filter instance
 * @param data New input sample
 */
void Mean_Filter_Update(mean_filter_t *filter, float data);

/** @} */

#endif /* FILTER_K_H */
