/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v00X_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/04
 * Description        : Main Interrupt Service Routines.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include <ch32v00X_it.h>
#include "Timer.h"
#include "ADC.h"
#include "Encoder.h"
#include "OLED.h"
extern void ADC_DMA_TC_Callback (void);

void NMI_Handler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void HardFault_Handler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void DMA1_Channel1_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler (void) __attribute__ ((interrupt));
void I2C1_ER_IRQHandler (void) __attribute__ ((interrupt));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler (void) {
    while (1) {
    }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler (void) {
    NVIC_SystemReset();
    while (1) {
    }
}

extern volatile uint8_t i2c_bus_fault;
void I2C1_ER_IRQHandler (void)
{
    /* 清除所有错误标志：BERR + ARLO + AF + OVR */
    I2C1->STAR1 = (uint16_t)~(0x0F00);
    i2c_bus_fault = 1;
}

void TIM1_UP_IRQHandler (void) {
    TIM_ClearITPendingBit (TIM1, TIM_IT_Update);
    BSP_TIM1_ISR_Callback();
}

void DMA1_Channel1_IRQHandler (void) {
    if (DMA_GetITStatus (DMA1_IT_TC1) != RESET) {
        DMA_ClearITPendingBit (DMA1_IT_TC1);
        ADC_DMA_TC_Callback();
    }
}

void EXTI7_0_IRQHandler (void) {
    EXTI7_0_IRQHandler_Callback();
}