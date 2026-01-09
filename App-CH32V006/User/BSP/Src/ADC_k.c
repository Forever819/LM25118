#include "debug.h"
#include "ADC_k.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

ADC_Value_t ADC_Value;
uint16_t ADC_Regular_Data[ADC_RegularGroup_Data_Size];

void BSP_ADC_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    ADC_InitTypeDef ADC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_PB2PeriphClockCmd (RCC_PB2Periph_GPIOA | RCC_PB2Periph_GPIOC | RCC_PB2Periph_GPIOD, ENABLE);
    RCC_PB2PeriphClockCmd (RCC_PB2Periph_ADC1, ENABLE);
    RCC_HBPeriphClockCmd (RCC_HBPeriph_DMA1, ENABLE);
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
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
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
    ADC_RegularChannelConfig (ADC1, ADC_Channel_0, 1, ADC_SampleTime_CyclesMode2);  // Vin
    ADC_RegularChannelConfig (ADC1, ADC_Channel_1, 2, ADC_SampleTime_CyclesMode2);  // Iin
    ADC_RegularChannelConfig (ADC1, ADC_Channel_2, 3, ADC_SampleTime_CyclesMode2);  // Vout
    ADC_RegularChannelConfig (ADC1, ADC_Channel_3, 4, ADC_SampleTime_CyclesMode2);  // Iout
    ADC_RegularChannelConfig (ADC1, ADC_Channel_4, 5, ADC_SampleTime_CyclesMode2);  // NTC
    ADC_DMACmd (ADC1, ENABLE);
    ADC_BufferCmd (ADC1, ENABLE);
    ADC_Cmd (ADC1, ENABLE);

    ADC_SoftwareStartConvCmd (ADC1, ENABLE);  // 未知 tim1trgo触发
}

void BSP_ADC_Loop (void) {
    ADC_Value.Vin = ADC_Regular_Data[0] * 0.013427734375f;
    ADC_Value.Iin = ADC_Regular_Data[1] * 0.0114278590425532f;
    ADC_Value.Pin = ADC_Value.Vin * ADC_Value.Iin;
    ADC_Value.Vout = ADC_Regular_Data[2] * 0.013427734375f;
    ADC_Value.Iout = ADC_Regular_Data[3] * 0.00537109375f+0.05;
    ADC_Value.Pout = ADC_Value.Vout * ADC_Value.Iout;
    ADC_Value.Inductance_Temperature = NTC_GetTemperature (ADC_Regular_Data[4]);

    // printf("%.2f,%.2f,%.2f,%.2f\r\n",ADC_Value.Vin,ADC_Value.Vout, ADC_Value.Iout,ADC_Value.Inductance_Temperature);

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

void IIR_Filter_Updatge (iir_filter_t *iir, uint16_t data) {
    if (!iir->flag) {
        iir->filter_out = data;
        iir->flag = 1;
    }
    iir->filter_out = (1 - iir->alpha) * data + iir->alpha * iir->filter_out;
}

