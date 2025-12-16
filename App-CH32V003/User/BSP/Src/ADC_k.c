#include "debug.h"
#include "ADC_k.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

ADC_Value_t ADC_Value;
static uint16_t ADC_Regular_Data[ADC_RegularGroup_Data_Size];

void BSP_ADC_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    ADC_InitTypeDef ADC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);
    RCC_ADCCLKConfig (RCC_PCLK2_Div2);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_Init (GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    ADC_DeInit (ADC1);
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = ADC_RegularGroup_Data_Size;

    ADC_Init (ADC1, &ADC_InitStructure);

    DMA_DeInit (DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->RDATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)ADC_Regular_Data;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = ADC_RegularGroup_Data_Size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init (DMA1_Channel1, &DMA_InitStructure);

    DMA_Cmd (DMA1_Channel1, ENABLE);
    // RegularGroup

    // 13.5 + 12.5 = 26c
    ADC_RegularChannelConfig (ADC1, ADC_Channel_0, 1, ADC_SampleTime_73Cycles);  // Vin
    // ADC_RegularChannelConfig (ADC1, ADC_Channel_1, 2, ADC_SampleTime_73Cycles);  // Iin
    ADC_RegularChannelConfig (ADC1, ADC_Channel_2, 2, ADC_SampleTime_73Cycles);  // Vout
    ADC_RegularChannelConfig (ADC1, ADC_Channel_3, 3, ADC_SampleTime_73Cycles);  // Iout
    ADC_RegularChannelConfig (ADC1, ADC_Channel_4, 4, ADC_SampleTime_73Cycles);  // NTC
    ADC_DMACmd (ADC1, ENABLE);
    // ADC_BufferCmd (ADC1, DISABLE);
    ADC_Cmd (ADC1, ENABLE);

    ADC_SoftwareStartConvCmd (ADC1, ENABLE);  // 未知 tim1trgo触发
}

void BSP_ADC_Loop (void) {
    ADC_Value.Vin = ADC_Regular_Data[0]*5.15625f;
    
    ADC_Value.Vout = ADC_Regular_Data[1]*5.15625f;
    ADC_Value.Iout = ADC_Regular_Data[2]*2.1484375f;
    ADC_Value.Pout = ADC_Value.Vout*ADC_Value.Iout/100;
    ADC_Value.Inductance_Temperature = NTC_GetTemperature_x10 (ADC_Regular_Data[3])*10.0f;
    for (u8 i = 0; i < 4; i++) {
        printf ("%4d ", ADC_Regular_Data[i]);
    }
    printf ("\r\n");
}
typedef struct {
    uint16_t adc;
    int16_t  temp_x10;   // 温度 ×10
} NTC_ADC_Table_t;

static const NTC_ADC_Table_t ntc_adc_table[] = {
    { 903, -200 },  // -20℃
    { 838, -100 },
    { 756,    0 },
    { 661,  100 },
    { 561,  200 },
    { 464,  300 },
    { 376,  400 },
    { 300,  500 },
    { 238,  600 },
    { 188,  700 },
    { 149,  800 },  
    { 119,  900 },  
    { 95,  1000 },  // 100℃
};
int16_t NTC_GetTemperature_x10(uint16_t adc)
{
    const int n = sizeof(ntc_adc_table) / sizeof(ntc_adc_table[0]);

    if (adc >= ntc_adc_table[0].adc)
        return ntc_adc_table[0].temp_x10;

    if (adc <= ntc_adc_table[n-1].adc)
        return ntc_adc_table[n-1].temp_x10;

    for (int i = 0; i < n - 1; i++)
    {
        uint16_t adc1 = ntc_adc_table[i].adc;
        uint16_t adc2 = ntc_adc_table[i + 1].adc;

        if (adc <= adc1 && adc >= adc2)
        {
            int16_t t1 = ntc_adc_table[i].temp_x10;
            int16_t t2 = ntc_adc_table[i + 1].temp_x10;

            // 线性插值（全整数）
            return t1 + (int32_t)(adc - adc1) * (t2 - t1) / (adc2 - adc1);
        }
    }

    return -2731; // 错误
}

// float NTC_GetTemperature (u16 adc) {
//     if (adc == 0 || adc >= ADX_MAX)
//         return -273.15f;  // 避免除零错误
//     // 计算NTC电阻
//     float v_adc = (float)adc / ADX_MAX * 3.3f;
//     float r_ntc = 10000.0f * v_adc / (3.3f - v_adc);

//     // 根据B值公式计算温度(K)
//     float t = 1.0f / (1.0f / 298.15f + (1.0f / 3380.0f) * logf (r_ntc / 10000.0f));

//     // 转换为摄氏度
//     return t - 273.15f;
// }

void Median_Filter_Update (median_filter_t *mf, uint16_t data) {
    mf->buffer[mf->index] = data;
    mf->index = (mf->index + 1) % MF_WINDOW_SIZE;  // 循环缓冲

    memcpy (mf->sorted, mf->buffer, MF_WINDOW_SIZE * sizeof (uint16_t));

    // 简单冒泡排序
    for (int i = 0; i < MF_WINDOW_SIZE - 1; i++) {
        for (int j = 0; j < MF_WINDOW_SIZE - 1 - i; j++) {
            if (mf->sorted[j] > mf->sorted[j + 1]) {
                u16 temp = mf->sorted[j];
                mf->sorted[j] = mf->sorted[j + 1];
                mf->sorted[j + 1] = temp;
            }
        }
    }
    mf->filter_out = mf->sorted[MF_WINDOW_SIZE / 2];
}

void Average_Filter_Update (average_filter_t *af, uint16_t data) {
    af->temp += data;
    af->index++;
    if (af->index > AF_WINDOW_SIZE) {
        af->filter_out = af->temp / af->index;
        af->index = 0;
        af->temp = 0;
    }
}

void IIR_Filter_Updatge (iir_filter_t *iir, uint16_t data) {
    if (!iir->flag) {
        iir->filter_out = data;
        iir->flag = 1;
    }
    iir->filter_out = (1 - iir->alpha) * data + iir->alpha * iir->filter_out;
}
