#include "debug.h"
#include "flash_param.h"
#include "Timer_k.h"
#include "Key_k.h"
#include "Encoder_k.h"
#include "Digital_Power.h"
#include "WS2812_k.h"
#include "OLED_UI.h"
#include "ADC_k.h"
#include "PID.h"

#define W2812_CV_COLOR 0xff0000
#define W2812_SCP_COLOR 0xFFFFFF
#define W2812_CC_COLOR 0xFFD700
#define W2812_IDLE_COLOR 0x001f00
#define SCP_VOLTAGE_MARGIN 0.3f
#define SCP_CURRENT_MARGIN 0.3f
#define OVP_MARGIN 0.1f
#define OCP_MARGIN 0.1f

flash_param_t flash_data;
Digital_Power_Dev dp;
static u8 cc_flag = 1;

void System_Init(void);
void Key_Event_Proc(void);
void Digital_Power_State_Update(void);

int main(void)
{
    System_Init();
    while (1)
    {
        ADC_Value.Inductance_Temperature = NTC_GetTemperature(ADC_Regular_Data[4]);
        cursor = &cursor_area[(u32)cursor_index % CURSOR_AREA_NUM];
        Key_Event_Proc();
        Digital_Power_State_Update();
        OLED_Ram_Update();
        OLED_Update();
        // printf ("Vin %.1f Vout%.1f\r\n", ADC_Value.Vin, ADC_Value.Vout);
        // Delay_Ms (10);
    }
}

void System_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("Systemclock %d\r\n", SystemCoreClock);

    Flash_Read_Cfg();

    BSP_PWM_Init();
    BSP_Buzzer_Init();
    BSP_WS2812_Init();
    BSP_ADC_Init();
    OLED_Init();
    OLED_UI_Init();

    Delay_Ms(100);
    BSP_Key_Init(&Key_Enable);
    BSP_Encoder_Init(&encoder1);

    dp.Iset = flash_data.cfg.Iset;
    dp.Vset = flash_data.cfg.Vset;
    dp.System_Enable_Flag = 0;
    dp.sys_state = IDLE_Switch;

    PID_Reset(&PID_Voltage);
    PID_Reset(&PID_Current);

    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &cursor_index);
    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
    BSP_Encoder_Set_Range(&encoder1.unpressed, 0, CURSOR_AREA_NUM - 1);

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_ResetBits(GPIOC, GPIO_Pin_5);
    GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Key_Event_Proc(void)
{
    Key_Event_t ev1 = BSP_Encoder_Get_Event(&encoder1);
    Key_Event_t ev2 = BSP_Key_Get_Event(&Key_Enable);

    switch (ev1)
    {
    case KEY_EVENT_CLICK:
    {
        if (dp.sys_state == FAULT_Lock)
        {
            dp.sys_state = IDLE_Switch;
            return;
        }
        if (dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 0;
            dp.sys_state = IDLE_Switch;
            cc_flag = 1;
            return;
        }
        if (!sub_cursor)
        {
            sub_cursor = cursor->digit - 1;
        }
        else
        {
            sub_cursor = 0;
            BSP_Encoder_CNT_Attach(&encoder1.unpressed, &cursor_index);
            BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
            BSP_Encoder_Set_Range(&encoder1.unpressed, 0, CURSOR_AREA_NUM - 1);
        }
        break;
    }
    case KEY_EVENT_LONG_PRESS:
    {
        if (!dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 1;
            dp.sys_state = CV_Switch;
        }
        break;
    }
    default:
        break;
    }

    switch (ev2)
    {
    case KEY_EVENT_CLICK:
    {
        break;
    }
    case KEY_EVENT_DOUBLE_PRESS:
    {
        printf("double\r\n");
        break;
    }
    case KEY_EVENT_LONG_PRESS:
    {
        flash_data.cfg.Iset = dp.Iset;
        flash_data.cfg.Vset = dp.Vset;
        flash_data.cfg.Pset = dp.Pset;
        printf("saving cfg\r\n");
        Flash_Save_Cfg();
        break;
    }
    default:
        break;
    }
}

void Digital_Power_State_Update(void)
{

    if (dp.System_Enable_Flag)
    {
        if (PID_Current.output && cc_flag)
        {
            cc_flag = 0;
            dp.sys_state = CC_Switch;
        }
    }
    else if (!cc_flag && !PID_Current.output)
    {
        dp.sys_state = CV_Switch;
        cc_flag = 1;
    }

    switch (dp.sys_state)
    {
    case IDLE_Switch:
        BSP_WS2812_Set_Color(W2812_IDLE_COLOR);
        dp.sys_state = IDLE;
        break;
    case IDLE:
        break;
    case CC_Switch:
        BSP_WS2812_Set_Color(W2812_CC_COLOR);
        Buzzer_Play(0, 100);
        Buzzer_Play(3300, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(3300, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(3300, 100);
        dp.sys_state = CC;
        break;
    case CC:
        break;
    case CV_Switch:
        BSP_WS2812_Set_Color(W2812_CV_COLOR);
        Buzzer_Play(0, 100);
        Buzzer_Play(3000, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(3000, 100);
        dp.sys_state = CV;
        break;
    case CV:
        break;
    case OVP:
        printf("over voltage protect triggered!\r\n");
        BSP_WS2812_Set_Color(W2812_SCP_COLOR);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        OLED_UI_OVP();
        dp.sys_state = FAULT_Lock;
        break;
    case OCP:
        printf("over current protect triggered!\r\n");
        BSP_WS2812_Set_Color(W2812_SCP_COLOR);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        OLED_UI_OCP();
        dp.sys_state = FAULT_Lock;
        break;
    case SCP:
        printf("shortcut protect triggered!\r\n");
        BSP_WS2812_Set_Color(W2812_SCP_COLOR);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        OLED_UI_SCP();
        dp.sys_state = FAULT_Lock;
        break;
    case FAULT_Lock:
        break;
    default:
        break;
    }
}

void BSP_TIM1_IQR_Callback()
{
    static u16 psc1 = 0;
    if (psc1++ > 5)
    {
        psc1 = 0;
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    }
    BSP_Encoder_Tick(&encoder1);
    BSP_Key_Tick(&Key_Enable);
    BSP_Buzzer_Task();
}

void ADC_DMA_TC_Callback(void)
{
    BSP_ADC_Loop();

    // PID Control
    if (dp.System_Enable_Flag)
    {
        // SCP Detect: current much higher than setpoint AND voltage collapsed
        if (ADC_Value.Iout > (dp.Iset / 100.0f) * (1 + SCP_CURRENT_MARGIN) && (ADC_Value.Vout < (dp.Vset / 100.0f) * (1 - SCP_VOLTAGE_MARGIN)))
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_5);
            dp.System_Enable_Flag = 0;
            dp.sys_state = SCP;
            return;
        }
        // OCP
        if (ADC_Value.Iout > (dp.Iset / 100.0f) * (1 + OCP_MARGIN))
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_5);
            dp.System_Enable_Flag = 0;
            dp.sys_state = OCP;
            return;
        }
        // OVP
        if (ADC_Value.Vout > (dp.Vset / 100.0f) * (1 + OVP_MARGIN))
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_5);
            dp.System_Enable_Flag = 0;
            dp.sys_state = OVP;
            return;
        }

        PID_Incremental_Calc(&PID_Voltage, dp.Vset / 100.0f + PID_Current.output, ADC_Value.Vout - ADC_Value.Iout * 0.01f);
        BSP_PWM_Set_CCR(PID_Voltage.output);
        GPIO_SetBits(GPIOC, GPIO_Pin_5);
    }
    else
    {
        PID_Reset(&PID_Voltage);
        PID_Reset(&PID_Current);
        GPIO_ResetBits(GPIOC, GPIO_Pin_5);
    }
}
