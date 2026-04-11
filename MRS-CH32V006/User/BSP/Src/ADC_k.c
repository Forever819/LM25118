#include "debug.h"
#include "ADC_k.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

#define ADC_RegularGroup_Data_Size 5
#define CAL_TIMES 500
ADC_Value_t ADC_Value;
int16_t ADC_Regular_Data[ADC_RegularGroup_Data_Size];
u32 cal[2] = {0};

void BSP_ADC_Init (void) {

    RCC_PB2PeriphClockCmd (RCC_PB2Periph_GPIOA | RCC_PB2Periph_GPIOC | RCC_PB2Periph_GPIOD, ENABLE);
    RCC_PB2PeriphClockCmd (RCC_PB2Periph_ADC1, ENABLE);
    RCC_HBPeriphClockCmd (RCC_HBPeriph_DMA1, ENABLE);
    RCC_ADCCLKConfig (RCC_PCLK2_Div4);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_Init (GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    DMA_InitTypeDef DMA_InitStructure = {0};
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
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init (&NVIC_InitStructure);
    DMA_ClearITPendingBit (DMA1_IT_TC1);
    DMA_ITConfig (DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_Cmd (DMA1_Channel1, ENABLE);

    ADC_InitTypeDef ADC_InitStructure = {0};
    ADC_DeInit (ADC1);
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = ADC_RegularGroup_Data_Size;
    ADC_Init (ADC1, &ADC_InitStructure);

    // RegularGroup
    ADC_RegularChannelConfig (ADC1, ADC_Channel_4, 5, ADC_SampleTime_CyclesMode4);  // NTC
    ADC_RegularChannelConfig (ADC1, ADC_Channel_0, 1, ADC_SampleTime_CyclesMode4);  // Vin
    ADC_RegularChannelConfig (ADC1, ADC_Channel_1, 2, ADC_SampleTime_CyclesMode4);  // Iin
    ADC_RegularChannelConfig (ADC1, ADC_Channel_2, 3, ADC_SampleTime_CyclesMode4);  // Vout
    ADC_RegularChannelConfig (ADC1, ADC_Channel_3, 4, ADC_SampleTime_CyclesMode4);  // Iout

    ADC_BufferCmd (ADC1, DISABLE);
    ADC_DMACmd (ADC1, ENABLE);
    ADC_Cmd (ADC1, ENABLE);
    Delay_Ms (100);  // 等待上电
}

void BSP_ADC_Loop (void) {
    ADC_Value.Vin = ADC_Regular_Data[0] * 0.0133731f - 0.0176f;
    ADC_Value.Vout = ADC_Regular_Data[2] * 0.0133731f - 0.0176f;

    ADC_Value.Iin = 0.0141f * ADC_Regular_Data[1] - 1.0862f;
    ADC_Value.Iout = 0.0077f * ADC_Regular_Data[3] - 0.6986f;
}

float NTC_GetTemperature (u16 adc) {
    if (adc == 0 || adc >= ADX_MAX)
        return -273.15f;  // 避免除零错误
    // 计算NTC电阻
    float v_adc = (float)adc / ADX_MAX * 3.3f;
    float r_ntc = 10000.0f * v_adc / (3.3f - v_adc);

    // 根据B值公式计算温度(K)
    float t = 1.0f / (1.0f / 298.15f + (1.0f / 3380.0f) * logf (r_ntc / 10000.0f));

    // 转换为摄氏度
    return t - 273.15f;
}

void IIR_Filter_Update (iir_filter_t *iir, uint16_t data) {
    if (!iir->flag) {
        iir->filter_out = data;
        iir->flag = 1;
    }
    iir->filter_out = (1 - iir->alpha) * data + iir->alpha * iir->filter_out;
}

void IIR_Filter_Init (iir_filter_t *iir, float alphas) {
    iir->filter_out = 0;
    iir->alpha = alphas;
}

void Mean_Filter_Update (mean_filter_t *filter, float data) {
#if 1
    if (!filter->flag) {
        // 首次使用，填充整个缓冲区
        for (uint16_t i = 0; i < MEAN_FILTER_SIZE; i++) {
            filter->buffer[i] = data;
        }
        filter->sum = data * MEAN_FILTER_SIZE;
        filter->filter_out = data;
        filter->flag = 1;
    }

    // 减去旧值，加上新值
    filter->sum -= filter->buffer[filter->index];
    filter->buffer[filter->index] = data;
    filter->sum += data;

    // 更新索引（循环）
    filter->index++;
    if (filter->index >= MEAN_FILTER_SIZE) {
        filter->index = 0;
    }

    // 计算均值
    filter->filter_out = filter->sum / MEAN_FILTER_SIZE;
#else
    filter->filter_out = data;
#endif
}

void Mean_Filter_Init (mean_filter_t *filter) {
    filter->index = 0;
    filter->sum = 0;
    filter->flag = 0;
    filter->filter_out = 0;
}
