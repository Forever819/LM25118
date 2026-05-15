/*coding: utf-8*/
#include "debug.h"
#include "ADC_k.h"
#include "Filter_k.h"
#include "flash_param.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

#define ADC_RegularGroup_Data_Size 5
#define CAL_TIMES 500

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

ADC_Value_t ADC_Value;
int16_t ADC_Regular_Data[ADC_RegularGroup_Data_Size];
u32 cal[2] = {0};
iir_filter_t vin_iir, vout_iir;

float g_vin_slope  = 0.0129f;
float g_vout_slope = 0.0135904f;
float g_iin_slope  = 0.0172976f;
float g_iout_slope = 0.0081299f;

static int16_t clamf_int16(int16_t x, int16_t min, int16_t max)
{
    if (x > max)
        return max;
    else if (x < min)
        return min;
    else
        return x;
}

/**
 * @brief Initialize ADC and DMA for continuous sampling
 */
void BSP_ADC_Init(void)
{

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA | RCC_PB2Periph_GPIOC | RCC_PB2Periph_GPIOD, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_ADC1, ENABLE);
    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_DeInit(DMA1_Channel1);
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
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    DMA_ClearITPendingBit(DMA1_IT_TC1);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADC_InitTypeDef ADC_InitStructure = {0};
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = ADC_RegularGroup_Data_Size;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_CyclesMode4); // Vin
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_CyclesMode4); // Iin
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_CyclesMode4); // Vout
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_CyclesMode4); // Iout
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_CyclesMode4); // NTC

    ADC_BufferCmd(ADC1, DISABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    Delay_Ms(100);

    ADC_InjectedSequencerLengthConfig(ADC1, 2);
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);
    ADC_ExternalTrigInjectedConvCmd(ADC1, DISABLE);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_CyclesMode4);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_CyclesMode4);
    for (uint16_t i = 0; i < CAL_TIMES; i++)
    {
        ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
        ADC_SoftwareStartInjectedConvCmd(ADC1, ENABLE);
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == RESET)
            ;
        cal[0] += ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        cal[1] += ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2);
        if (i != 0)
        {
            cal[0] /= 2;
            cal[1] /= 2;
        }
    }
    xprintf("cal[0]: %lu, cal[1]: %lu\r\n", cal[0], cal[1]);

    IIR_Filter_Init(&vin_iir, 0.92f);
    IIR_Filter_Init(&vout_iir, 0.88f);
}

/** @brief Synchronize ADC calibration parameters from flash storage */
void BSP_ADC_Sync_Param(void)
{
    g_vin_slope  = flash_data.cfg.vin_slope;
    g_vout_slope = flash_data.cfg.vout_slope;
    g_iin_slope  = flash_data.cfg.iin_slope;
    g_iout_slope = flash_data.cfg.iout_slope;
}

/** @brief Process ADC samples: apply filtering and conversion to physical units */
void BSP_ADC_Loop(void)
{
    IIR_Filter_Update(&vin_iir, ADC_Regular_Data[0]);
    IIR_Filter_Update(&vout_iir, ADC_Regular_Data[2]);
    ADC_Value.Vin = vin_iir.filter_out * g_vin_slope;
    ADC_Value.Vout = vout_iir.filter_out * g_vout_slope;

    ADC_Value.Iin = g_iin_slope * (clamf_int16(ADC_Regular_Data[1] - cal[0], 0, 4096));
    ADC_Value.Iout = g_iout_slope * (clamf_int16(ADC_Regular_Data[3] - cal[0], 0, 4096));
}

/**
 * @brief Calculate temperature from NTC thermistor ADC reading
 * @param adc Raw ADC value
 * @return Temperature in Celsius
 */
float NTC_GetTemperature(u16 adc)
{
    if (adc == 0 || adc >= ADX_MAX)
        return -273.15f;

    float v_adc = (float)adc / ADX_MAX * 3.3f;
    float r_ntc = 10000.0f * v_adc / (3.3f - v_adc);

    float t = 1.0f / (1.0f / 298.15f + (1.0f / 3380.0f) * logf(r_ntc / 10000.0f));

    return t - 273.15f;
}

/*
Usage example:
    // Initialize ADC system
    BSP_ADC_Init();

    // In main loop / DMA interrupt:
    BSP_ADC_Loop();  // Process ADC samples with filtering

    // Access filtered measurements
    xprintf("Vin=%.2fV, Iin=%.2fA\r\n", ADC_Value.Vin, ADC_Value.Iin);
    xprintf("Vout=%.2fV, Iout=%.2fA\r\n", ADC_Value.Vout, ADC_Value.Iout);

    // Get temperature reading
    float temp = NTC_GetTemperature(ADC_Regular_Data[4]);
    xprintf("Temp=%.1fC\r\n", temp);

    // Reload calibration from flash if needed
    BSP_ADC_Sync_Param();
*/
