#ifndef ADC_K_H


#define ADX_MAX 4095
#define ADC_RegularGroup_Data_Size 5

typedef struct{
    float Vin;
    float Iin;
    float Pin;
    float Vout;
    float Iout;
    float Pout;
    float Inductance_Temperature;
}ADC_Value_t;

typedef struct{
    float alpha;
    u8 flag;
    float filter_out;
}iir_filter_t;

void BSP_ADC_Init (void);
void BSP_ADC_Loop (void);
void BSP_ADC_Update_PID (void);

float NTC_GetTemperature (u16 adc);
void IIR_Filter_Updatge (iir_filter_t *iir, uint16_t data);

extern ADC_Value_t ADC_Value;
extern uint16_t ADC_Regular_Data[];
#define ADC_K_H
#endif