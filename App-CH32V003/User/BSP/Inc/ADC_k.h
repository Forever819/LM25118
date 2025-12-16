#ifndef ADC_K_H

#define ADC_RegularGroup_Data_Size 4
#define MF_WINDOW_SIZE 1  // 中值滤波窗口大小
#define AF_WINDOW_SIZE 1  // 均值窗口大小
#define ADX_MAX 4095

typedef struct {
    uint16_t buffer[MF_WINDOW_SIZE];
    uint16_t sorted[MF_WINDOW_SIZE];
    uint16_t index;
    uint16_t filter_out;
} median_filter_t;

typedef struct {
    uint16_t temp;
    uint16_t index;
    uint16_t filter_out;
} average_filter_t;

typedef struct{
    float alpha;
    u8 flag;
    float filter_out;
}iir_filter_t;

// typedef struct {
//     uint16_t adc_val;
//     float r_pullup;
//     float r0;
//     float t0;
//     float b_value;
// } NTC_t;

typedef struct{
    float Vin;
    float Iin;
    float Pin;
    float Vout;
    float Iout;
    float Pout;
    float Inductance_Temperature;
}ADC_Value_t;

void BSP_ADC_Init (void);
void BSP_ADC_Loop (void);
void BSP_ADC_Update_PID (void);
// float NTC_GetTemperature (NTC_t *NTC);
float NTC_GetTemperature (u16 adc);
int16_t NTC_GetTemperature_x10(uint16_t adc);

void Median_Filter_Update (median_filter_t *mf, uint16_t data);
void Average_Filter_Update (average_filter_t *af, uint16_t data);
void IIR_Filter_Updatge(iir_filter_t *iir,uint16_t data);

extern ADC_Value_t ADC_Value;
#define ADC_K_H
#endif