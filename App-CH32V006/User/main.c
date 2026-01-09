#include "debug.h"
#include "Timer_k.h"
#include "Key_k.h"
#include "Encoder_k.h"
#include "WS2812_k.h"
#include "OLED.h"
#include "ADC_k.h"
#include "PID.h"

typedef enum {
    IDLE = 0,
    CC,
    CV,
    OVP,
    OCP
} Digital_Power_State;

typedef struct {
    Digital_Power_State sys_state;
    u8 System_Enable_Flag;

    float Vset;
    float Iset;

    float current_input_limit;
    float current_output_limit;
    float voltage_intput_limit;
    float voltage_output_limit;

} Digital_Power_Dev;

Digital_Power_Dev dp;

void OLED_Draw_Static_UI (void) {
    OLED_Printf (30, 0, OLED_8X16, "IN");
    OLED_Printf (72, 0, OLED_8X16, "OUT");
    OLED_Printf (115, 20, OLED_8X16, "V");
    OLED_Printf (115, 32, OLED_8X16, "A");
    OLED_Printf (115, 48, OLED_8X16, "W");
    OLED_DrawLine (62, 0, 62, 64);
}

void OLED_Data_Update (void) {
    OLED_ShowFloatNum (0, 9, ADC_Value.Inductance_Temperature, 2, 1, OLED_6X8);
    OLED_ShowFloatNum (20, 17, ADC_Value.Vin, 2, 2, OLED_8X16);
    OLED_ShowFloatNum (20, 32, ADC_Value.Iin, 2, 2, OLED_8X16);
    OLED_ShowFloatNum (20, 48, ADC_Value.Pin, 2, 2, OLED_8X16);

    if (dp.System_Enable_Flag) {
        OLED_ShowFloatNum (70, 17, ADC_Value.Vout, 2, 2, OLED_8X16);
        OLED_ShowFloatNum (70, 32, ADC_Value.Iout, 2, 2, OLED_8X16);
        OLED_ShowFloatNum (70, 48, ADC_Value.Pout, 2, 2, OLED_8X16);
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
    OLED_Draw_Static_UI();
    dp.current_input_limit = 10.0f;
    dp.current_output_limit = 2.0f;
    dp.voltage_output_limit = 45.0f;
    dp.System_Enable_Flag = 0;
    dp.sys_state = IDLE;
}

int main (void) {
    System_Init();
    while (1) {

        OLED_Data_Update();
        OLED_Update();
        dp.Vset = BSP_Encoder_Get_Cnt() % 40 + 1;
        // printf ("%d %.1f %.1f %.3f\r\n",TIM1->CH3CVR,PID_Voltage.output, dp.Vset, ADC_Value.Vout);
        // printf ("Enc %d\r\n", BSP_Encoder_Get_Cnt());
        for (u8 i = 0; i < 4; i++) {
            printf ("%4d ", ADC_Regular_Data[i]);
        }
        printf ("\r\n");
        Delay_Ms (10);
    }
}

// 1Khz INT
void BSP_TIM1_IQR_Callback() {
    BSP_Key_Task();
    BSP_ADC_Loop();
    if (dp.System_Enable_Flag) {
        PID_Position_Calc (&PID_Voltage, dp.Vset, ADC_Value.Vout);
        BSP_PWM_DAC_Set_CCR (PID_Voltage.output + 540);
        // BSP_PWM_DAC_Set_CCR (1000);
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