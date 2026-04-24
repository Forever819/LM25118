#ifndef ADC_K_H
#define ADC_K_H

#include <stdint.h>

#define ADX_MAX 4095

typedef struct {
    float Vin;
    float Iin;
    float Pin;
    float Vout;
    float Iout;
    float Pout;
    float Inductance_Temperature;
    float Vref;
} ADC_Value_t;

typedef struct {
    float alpha;
    float filter_out;
    u8 flag;
} iir_filter_t;

void BSP_ADC_Init (void);
void BSP_ADC_Loop (void);
void BSP_ADC_Update_PID (void);

float NTC_GetTemperature (u16 adc);

void IIR_Filter_Init (iir_filter_t *iir, float alphas);
void IIR_Filter_Update (iir_filter_t *iir, uint16_t data);

#define MEAN_FILTER_SIZE 10

typedef struct {
    float buffer[MEAN_FILTER_SIZE];
    uint16_t index;
    float sum;
    uint8_t flag;
    float filter_out;
} mean_filter_t;

void Mean_Filter_Init (mean_filter_t *filter);
void Mean_Filter_Update (mean_filter_t *filter, float data);

extern ADC_Value_t ADC_Value;
extern int16_t ADC_Regular_Data[];

#endif /* ADC_K_H */
