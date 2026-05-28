#ifndef ADC_K_H
#define ADC_K_H

#include <stdint.h>
#include "Filter.h"

#define ADX_MAX 4095

/** @brief ADC measurement value container */
typedef struct {
    float Vin;   /**< Input voltage */
    float Iin;   /**< Input current */
    float Pin;   /**< Input power */
    float Vout;  /**< Output voltage */
    float Iout;  /**< Output current */
    float Pout;  /**< Output power */
    float Inductance_Temperature;  /**< Inductor temperature */
    float Vref;  /**< Reference voltage */
} ADC_Value_t;

/**
 * @brief Initialize ADC and DMA
 */
void BSP_ADC_Init(void);

/**
 * @brief Process ADC samples with filtering and conversion
 */
void BSP_ADC_Loop(void);

/**
 * @brief Synchronize ADC calibration parameters from flash
 */
void BSP_ADC_Sync_Param(void);

/**
 * @brief Get NTC temperature from ADC value
 * @param adc Raw ADC reading
 * @return Temperature in Celsius
 */
float NTC_GetTemperature(u16 adc);

extern ADC_Value_t ADC_Value;
extern int16_t ADC_Regular_Data[];

extern float g_vin_slope, g_vout_slope;
extern float g_iin_slope, g_iout_slope;
#endif /* ADC_K_H */
