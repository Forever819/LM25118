/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *Multiprocessor communication mode routine:
 *Master:USART1_Tx(PD5)\USART1_Rx(PD6).
 *This routine demonstrates that USART1 receives the data sent by CH341 and inverts
 *it and sends it (baud rate 115200).
 *
 *Hardware connection:PD5 -- Rx
 *                     PD6 -- Tx
 *
 */

#include "debug.h"
#include "Timer_k.h"
#include "Key_k.h"
#include "Encoder_k.h"
#include "WS2812_k.h"
#include "OLED.h"
#include "ADC_k.h"

/* Global Variable */
extern void OLED_IO_Init (void);
extern void HAL_I2C_Master_Transmit (u8 addr, u8 *data, u8 data_length, u16 timeout);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    BSP_TIM1_Init();
    BSP_TIM2_Init();
    BSP_WS2812_Init();
    BSP_Key_Init();
    BSP_Encoder_Init();
    BSP_ADC_Init();
    OLED_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init (115200);
    OLED_Printf (12, 0, OLED_8X16, "INPUT OUTPUT");
    OLED_Printf (120, 16, OLED_8X16, "V");
    OLED_Printf (120, 32, OLED_8X16, "A");
    OLED_Printf (120, 48, OLED_8X16, "W");
    void OLED_Display (void);

#endif
    printf ("\r\nSystemClk:%d\r\n", SystemCoreClock);
    printf ("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    while (1) {
        BSP_ADC_Loop();
        OLED_Display();
        OLED_Update();
        TIM_SetCompare3(TIM1,BSP_Encoder_Get_Cnt());
    }
}

void OLED_Display (void) {
    OLED_Printf (0, 16, OLED_8X16, "%3d.%2d", ((int)ADC_Value.Vin) / 100, ((int)ADC_Value.Vin) % 100);
    OLED_Printf (0, 32, OLED_8X16, "%3d.%2d", (int)ADC_Value.Iin / 100, (int)ADC_Value.Iin % 100);
    OLED_Printf (0, 48, OLED_8X16, "%3d.%2d", (int)ADC_Value.Pin / 100, (int)ADC_Value.Pin % 100);

    OLED_Printf (54, 16, OLED_8X16, "%3d.%2d", (int)ADC_Value.Vout / 100, (int)ADC_Value.Vout % 100);
    OLED_Printf (54, 32, OLED_8X16, "%3d.%2d", (int)ADC_Value.Iout / 100, (int)ADC_Value.Iout % 100);
    OLED_Printf (54, 48, OLED_8X16, "%3d.%2d", (int)ADC_Value.Pout / 100, (int)ADC_Value.Pout % 100);
}

void BSP_TIM1_IQR_Callback() {
    BSP_Key_Task();
}

void BSP_Key_Task (void) {
    BSP_Key_Scan();
    if (Key_Encoder.pressed_flag) {
        Key_Encoder.pressed_flag = 0;
    }
    if (Key_Enable.pressed_flag) {
        static u8 flag = 1;
        Key_Enable.pressed_flag = 0;
        if (flag) {
            flag = 0;
            GPIO_SetBits (GPIOC, GPIO_Pin_5);
            Buzzer_Play (2000, 200);
            BSP_WS2812_Set_Color (0xff0000);
        } else {
            flag = 1;
            Buzzer_Play (440, 200);
            GPIO_ResetBits (GPIOC, GPIO_Pin_5);
            BSP_WS2812_Set_Color (0x00000f);
        }
    }
}