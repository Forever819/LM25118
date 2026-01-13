#ifndef ADC_K_H


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

#define MEAN_FILTER_SIZE 10  // 定义缓冲区大小

typedef struct {
    float buffer[MEAN_FILTER_SIZE];  // 数据缓冲区
    uint16_t index;                  // 当前写入位置
    float sum;                       // 数据总和
    uint8_t flag;                    // 初始化标志
    float filter_out;                // 滤波输出
} mean_filter_t;

void Mean_Filter_Init (mean_filter_t *filter);
void Mean_Filter_Update (mean_filter_t *filter, float data);

extern ADC_Value_t ADC_Value;
extern uint16_t ADC_Regular_Data[];
#define ADC_K_H
#endif