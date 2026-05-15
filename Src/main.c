#include "debug.h"

#include "OLED_UI.h"
#include "Digital_Power.h"
#include "Event_Bus.h"
#include "Digital_Power_Core.h"
#include "Responders.h"

#include "ADC_k.h"
#include "flash_param.h"
#include "Encoder_k.h"
#include "Buzzer_K.h"

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

flash_param_t flash_data;
Digital_Power_Dev dp;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    xprintf("Systemclock %ld\r\n", SystemCoreClock);

    // 初始化硬件
    MCU_Flash_Read_Cfg();
    OLED_UI_Init();
    Digital_Power_Init();

    // 初始化业务逻辑和事件系统
    Digital_Power_Core_Init();
    Event_Bus_Init();
    Responders_Init_All(); // 初始化所有事件响应器

    while (1)
    {
        // 读取温度传感器
        ADC_Value.Inductance_Temperature = NTC_GetTemperature(ADC_Regular_Data[4]);
        cursor = &cursor_area[(u32)cursor_index % CURSOR_AREA_NUM];

        // 处理用户交互
        Key_Event_Proc();

        // 消费 ISR 中入队的保护事件
        Event_Bus_Flush();

        // 执行业务逻辑，获取事件
        PowerEvent_t ev = Digital_Power_State_Update();
        if (ev.type != EVENT_NONE)
        {
            Event_Bus_Publish(&ev, event_trigger_immediately);
        }

        // UI 更新
        OLED_Ram_Update();
        OLED_Update();

        // xprintf("Vin %.2f Vout%.2f\r\n", ADC_Value.Vin, ADC_Value.Vout);
        Delay_Ms(10);
    }
}

void Digital_Power_Init(void)
{
    BSP_PWM_Init();
    BSP_ADC_Init();
    BSP_ADC_Sync_Param();

    dp.Iset = flash_data.cfg.Iset;
    dp.Vset = flash_data.cfg.Vset;
    dp.System_Enable_Flag = 0;
    dp.sys_state = IDLE_Switch;
    PID_Reset(&PID_Voltage);
    PID_Reset(&PID_Current);
}

void Key_Event_Proc(void)
{
    Key_Event_t ev_encoder = BSP_Encoder_Get_Event(&encoder1);
    Key_Event_t ev_key = BSP_Key_Get_Event(&Key_Enable);

    switch (ev_encoder)
    {
    case KEY_EVENT_CLICK:
    {

        // 处于FAULT_Lock状态时，点击按键重置系统
        if (dp.sys_state == FAULT_Lock)
        {
            dp.sys_state = IDLE_Switch;
            return;
        }

        // 处于开启状态时,点击按键点击按键重置系统
        if (dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 0;
            dp.sys_state = IDLE_Switch;
            return;
        }

        /* 将 sub_cursor 管理委托给 UI 层 */
        OLED_UI_OnEncoderClick();
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

    switch (ev_key)
    {
    case KEY_EVENT_CLICK:
    {
        NVIC_SystemReset();
        break;
    }
    case KEY_EVENT_DOUBLE_PRESS:
    {
        xprintf("double\r\n");
        break;
    }
    case KEY_EVENT_LONG_PRESS:
    {
        flash_data.cfg.Iset = dp.Iset;
        flash_data.cfg.Vset = dp.Vset;
        flash_data.cfg.Pset = dp.Pset;
        flash_data.cfg.vin_slope = g_vin_slope;
        flash_data.cfg.vout_slope = g_vout_slope;
        flash_data.cfg.iin_slope = g_iin_slope;
        flash_data.cfg.iout_slope = g_iout_slope;
        xprintf("saving cfg\r\n");
        Flash_Save_Cfg();
        break;
    }
    default:
        break;
    }
}

// 1Khz ISR
void BSP_TIM1_ISR_Callback()
{
    static u16 psc1 = 0;
    if (psc1++ >= 5)
    {
        psc1 = 0;
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    }
    BSP_Encoder_Tick(&encoder1);
    BSP_Key_Tick(&Key_Enable);
    BSP_Buzzer_Tick();
}

void ADC_DMA_TC_Callback(void)
{
    BSP_ADC_Loop();

    // 检查所有保护条件（在中断中执行）
    // 返回 1 表示保护触发，阻止 PID 控制
    if (!Protection_Check())
    {
        // 未触发保护，执行 PID 控制
        if (dp.System_Enable_Flag)
        {
            PID_Incremental_Calc(&PID_Voltage, dp.Vset / 100.0f + PID_Current.output, ADC_Value.Vout - ADC_Value.Iout * 0.01f);
            BSP_PWM_Set_CCR(PID_Voltage.output);
            LM25118_ENABLE
        }
        else
        {
            PID_Reset(&PID_Voltage);
            PID_Reset(&PID_Current);
            LM25118_DISABLE
        }
    }
}
