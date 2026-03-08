#include "debug.h"
#include "Digital_Power.h"

Digital_Power_Dev dp;
mean_filter_t vin_mf, vout_mf, Iin_mf, Iout_mf;

void OLED_UI_Init (void) {
    OLED_Printf (30, 0, OLED_8X16, "IN");
    OLED_Printf (72, 0, OLED_8X16, "OUT");
    OLED_Printf (115, 20, OLED_8X16, "V");
    OLED_Printf (115, 32, OLED_8X16, "A");
    OLED_Printf (115, 48, OLED_8X16, "W");
    OLED_DrawLine (62, 0, 62, 64);
    Mean_Filter_Init (&vin_mf);
    Mean_Filter_Init (&vout_mf);
    Mean_Filter_Init (&Iin_mf);
    Mean_Filter_Init (&Iout_mf);
}

void OLED_Data_Update (void) {


    Mean_Filter_Update (&vin_mf, ADC_Value.Vin);
    Mean_Filter_Update (&vout_mf, ADC_Value.Vout);
    Mean_Filter_Update (&Iin_mf, ADC_Value.Iin);
    Mean_Filter_Update (&Iout_mf, ADC_Value.Iout);

    OLED_ShowFloatNum (0, 9, ADC_Value.Inductance_Temperature, 2, 1, OLED_6X8);
    OLED_ShowFloatNum (20, 17, vin_mf.filter_out, 2, 2, OLED_8X16);
    OLED_ShowFloatNum (20, 32, Iin_mf.filter_out, 2, 2, OLED_8X16);
    OLED_ShowFloatNum (20, 48, vin_mf.filter_out * Iin_mf.filter_out, 2, 2, OLED_8X16);

    if (dp.System_Enable_Flag) {
        OLED_ShowFloatNum (70, 17, vout_mf.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum (70, 32, Iout_mf.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum (70, 48, vout_mf.filter_out * Iout_mf.filter_out, 2, 2, OLED_8X16);
        if (dp.sys_state == CV) {
            OLED_Printf (2, 0, OLED_6X8, "CV  ");
        } else if (dp.sys_state == CC) {
            OLED_Printf (2, 0, OLED_6X8, "CC  ");
        }
    } else {
        OLED_ShowFloatNum (70, 17, dp.Vset, 2, 2, OLED_8X16);
        OLED_ShowFloatNum (70, 32, dp.Iset, 2, 2, OLED_8X16);
        OLED_Printf (2, 0, OLED_6X8, "IDLE");
    }
}

void System_Init (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init (115200);
    printf("Systemclock %d\r\n",SystemCoreClock);
    BSP_TIM1_Init();
    BSP_TIM2_Init();
    BSP_WS2812_Init();
    BSP_Key_Init();
    BSP_Encoder_Init();
    BSP_ADC_Init();
    OLED_Init();
    OLED_UI_Init();
    dp.current_input_limit = 10.0f;
    dp.current_output_limit = 2.0f;
    dp.voltage_output_limit = 45.0f;
    dp.System_Enable_Flag = 0;
    dp.sys_state = IDLE;

    dp.Vset = 5.0f;
    BSP_Encoder_Set_Cnt (&dp.Vset);
    BSP_Encoder_Set_Step_Value (1.0f);
    BSP_EncoderCNT_Set_Range (2.0f, 42.0f);

}

int main (void) {
    System_Init();
    while (1) {

        OLED_Data_Update();
        OLED_Update();
    }
}

// 1Khz INT
void BSP_TIM1_IQR_Callback() {
    BSP_Buzzer_Task();
    BSP_Key_Task();
    BSP_ADC_Loop();
    if (dp.System_Enable_Flag) {
        PID_Position_Calc (&PID_Voltage, dp.Vset, ADC_Value.Vout);  // 考虑两个低端采样电阻压降
        BSP_PWM_DAC_Set_CCR (PID_Voltage.output);
        // PID_Position_Calc (&PID_Voltage, dp.Vset, ADC_Value.Vout - 0.0278f * ADC_Value.Iout);  // 考虑两个低端采样电阻压降
        // BSP_PWM_DAC_Set_CCR (PID_Voltage.output + 540);
    } else {
        PID_Reset (&PID_Voltage);
    }
}

void BSP_Key_Task (void) {
    BSP_Key_Scan();
    if (Key_Encoder.pressed_flag) {
        Key_Encoder.pressed_flag = 0;
    }
    if (Key_Enable.pressed_flag) {
        Key_Enable.pressed_flag = 0;
        dp.System_Enable_Flag = !dp.System_Enable_Flag;
        if (dp.System_Enable_Flag) {
            GPIO_SetBits (GPIOC, GPIO_Pin_5);
            Buzzer_Play (2000, 200);
            BSP_WS2812_Set_Color (0xff0000);
        } else {
            Buzzer_Play (440, 200);
            GPIO_ResetBits (GPIOC, GPIO_Pin_5);
            BSP_WS2812_Set_Color (0x00000f);
        }
    }
}