#include "debug.h"
#include "Digital_Power.h"

typedef struct {
    u8 trigger_flag;
    float *monitor_value;
    float up_trigger_threshold;
    float down_trigger_threshold;
} Warning_t;

Digital_Power_Dev dp;
mean_filter_t vin_mf, vout_mf, Iin_mf, Iout_mf;
Warning_t Input_Power_Limit,Temperature_Limit;

void Warning_Trigger_Init(Warning_t *wt,float up_thres,float down_thres,float *monitor)
{
    wt->down_trigger_threshold=down_thres;
    wt->up_trigger_threshold=up_thres;
    wt->monitor_value=monitor;
    wt->trigger_flag = 0;
}

u8 Warning_Trigger_Update (Warning_t *wt) {
    if (*wt->monitor_value > wt->up_trigger_threshold) {
        if(!wt->trigger_flag)wt->trigger_flag = 1;
        return 1;
    } else if (*wt->monitor_value < wt->down_trigger_threshold) {
        if(wt->trigger_flag)wt->trigger_flag = 0;
    }
    return 0;
}

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

    Warning_Trigger_Init(&Input_Power_Limit,55.0f,50.0f,&ADC_Value.Pin);
    Warning_Trigger_Init(&Temperature_Limit,70.0f,60.0f,&ADC_Value.Inductance_Temperature);

}

int main (void) {
    System_Init();
    while (1) {

        OLED_Data_Update();
        OLED_Update();
        if (Warning_Trigger_Update (&Input_Power_Limit)) {
            Buzzer_Play(3000,100);
            // Key_Enable.pressed_flag = 1;
        }
        if (Warning_Trigger_Update (&Temperature_Limit)) {
            Buzzer_Play(3000,100);
            Key_Enable.pressed_flag = 1;
        }


        // printf ("%d %.1f %.1f %.3f\r\n",TIM1->CH3CVR,PID_Voltage.output, dp.Vset, ADC_Value.Vout);
        // printf ("Enc %d\r\n", BSP_Encoder_Get_Cnt());
        // for (u8 i = 0; i < 4; i++) {
        //     printf ("%4d ", ADC_Regular_Data[i]);
        // }
        // printf ("\r\n");
        // printf ("%.2f,%.2f,%.2f,%.2f \r\n", vin_i.filter_out, Iin_i.filter_out, vout_i.filter_out, Iout_i.filter_out);
        // printf ("%.2f,%.2f,%.2f,%.2f,%.2f \r\n", ADC_Value.Vin, ADC_Value.Iin, ADC_Value.Vout, ADC_Value.Iout,ADC_Value.Vref);
        // printf ("%.2f,%.2f,%.2f,%.2f \r\n", vin_mf.filter_out,vout_mf.filter_out,Iin_mf.filter_out,Iout_mf.filter_out);
        Delay_Ms (10);
    }
}

// 1Khz INT
void BSP_TIM1_IQR_Callback() {
    BSP_Buzzer_Task();
    BSP_Key_Task();
    BSP_ADC_Loop();
    if (dp.System_Enable_Flag) {
        PID_Position_Calc (&PID_Voltage, dp.Vset, ADC_Value.Vout - 0.0278f * ADC_Value.Iout);  // 考虑两个低端采样电阻压降
        BSP_PWM_DAC_Set_CCR (PID_Voltage.output + 540);
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