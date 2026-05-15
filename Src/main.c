#include "debug.h"

#include "OLED_UI.h"
#include "Digital_Power.h"
#include "Event_Bus.h"
#include "Digital_Power_Core.h"
#include "Responders.h"

#include "ADC.h"
#include "flash_param.h"
#include "Encoder.h"
#include "FloatEditor.h"
#include "Buzzer.h"

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

flash_param_t flash_data;
Digital_Power_Dev dp;

/**
 * @brief 编码器增量累加器
 *
 * 由 EXTI ISR 写入，由 main 循环在临界区内读取并清零。
 * 编码器在初始化时一次性绑定到这两个累加器，之后永不重新绑定。
 *
 * - g_enc_delta_unpressed: 非按压旋转增量（edit_value / param_index 导航）
 * - g_enc_delta_pressed:   按压旋转增量（cursor_pos 移动）
 */
volatile s32 g_enc_delta_unpressed = 0;
volatile s32 g_enc_delta_pressed   = 0;

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

    /* 编码器一次性绑定到增量累加器，之后永不重新绑定 */
    BSP_Encoder_CNT_Attach(&encoder1.unpressed, (s32 *)&g_enc_delta_unpressed);
    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
    BSP_Encoder_Set_Range(&encoder1.unpressed, -1000, 1000);
    BSP_Encoder_CNT_Attach(&encoder1.pressed, (s32 *)&g_enc_delta_pressed);
    BSP_Encoder_Set_Step(&encoder1.pressed, 1);
    BSP_Encoder_Set_Range(&encoder1.pressed, -1000, 1000);

    Digital_Power_Init();

    // 初始化业务逻辑和事件系统
    Digital_Power_Core_Init();
    Event_Bus_Init();
    Responders_Init_All(); // 初始化所有事件响应器

    while (1)
    {
        // 读取温度传感器
        ADC_Value.Inductance_Temperature = NTC_GetTemperature(ADC_Regular_Data[4]);

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

/**
 * @brief 按键/编码器事件处理 —— 输入分发层
 *
 * 分层架构中的输入层：
 *   [ 编码器 ISR ] → [ 增量累加器 ] → [ Key_Event_Proc ] → [ FloatEditor / 导航 / 设置 ]
 *
 * 编码器永远只写入增量累加器，本函数在临界区内读取并清零，
 * 然后根据当前状态（编辑/导航/设置页面）分发到对应处理逻辑。
 */
void Key_Event_Proc(void)
{
    Key_Event_t ev_encoder = BSP_Encoder_Get_Event(&encoder1);
    Key_Event_t evey = BSP_Key_Get_Event(&Key_Enable);

    /* ---- 读取编码器增量（临界区保护，防止 ISR 并发写入）---- */
    s32 delta_up = 0, delta_pr = 0;
    __disable_irq();
    delta_up = g_enc_delta_unpressed;
    g_enc_delta_unpressed = 0;
    delta_pr = g_enc_delta_pressed;
    g_enc_delta_pressed = 0;
    __enable_irq();

    /* ============================================================
     *  编码器按键事件（优先级高于增量）
     * ============================================================ */

    /* 长按编码器：取消编辑（如正在编辑），切换输出使能 */
    if (ev_encoder == KEY_EVENT_LONG_PRESS)
    {
        if (g_editor.is_editing)
        {
            FloatEditor_OnButton(&g_editor, FE_BTN_CANCEL);
        }
        if (!dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 1;
            dp.sys_state = CV_Switch;
        }
        return;
    }

    /* 单击编码器 */
    if (ev_encoder == KEY_EVENT_CLICK)
    {
        /* FAULT_Lock 状态：单击复位 */
        if (dp.sys_state == FAULT_Lock)
        {
            dp.sys_state = IDLE_Switch;
            return;
        }

        /* 输出使能状态：单击关断 */
        if (dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 0;
            dp.sys_state = IDLE_Switch;
            return;
        }

        /* 设置页面中：单击退出 */
        if (OLED_UI_IsInSettings())
        {
            OLED_UI_ExitSettings();
            return;
        }

        /* 设置入口：单击进入设置页面 */
        if (g_param_index == 3)
        {
            OLED_UI_EnterSettings();
            return;
        }

        /* 参数编辑：切换编辑/确认 */
        if (g_editor.is_editing)
        {
            FloatEditor_OnButton(&g_editor, FE_BTN_OK);
        }
        else
        {
            FloatEditor_StartEdit(&g_editor);
        }
        return;
    }

    /* ============================================================
     *  编码器增量处理
     * ============================================================ */

    /* 设置页面中的增量处理 */
    if (OLED_UI_IsInSettings())
    {
        if (delta_pr != 0)
        {
            /* 按压旋转：修改当前设置项的值 */
            switch (g_settings_cursor)
            {
            case 0: /* OLED Flip */
                flash_data.cfg.oled_direction += (int)delta_pr;
                if (flash_data.cfg.oled_direction < 0)
                    flash_data.cfg.oled_direction = 0;
                if (flash_data.cfg.oled_direction > 1)
                    flash_data.cfg.oled_direction = 1;
                OLED_Flip(flash_data.cfg.oled_direction);
                break;
            case 1: /* Buzzer (预留) */
                break;
            default:
                break;
            }
        }
        if (delta_up != 0)
        {
            /* 非按压旋转：切换设置项 */
            int8_t idx = (int8_t)g_settings_cursor + (int8_t)delta_up;
            if (idx < 0) idx = 0;
            if (idx >= SETTINGS_COUNT) idx = SETTINGS_COUNT - 1;
            g_settings_cursor = (uint8_t)idx;
        }
        /* 设置页面中不处理 Key_Enable 以外的后续逻辑 */
    }
    else if (g_editor.is_editing)
    {
        /* ---- 编辑模式 ---- */
        if (delta_up != 0)
        {
            FloatEditor_OnEncoder(&g_editor, (int8_t)delta_up);
        }
        if (delta_pr != 0)
        {
            FloatEditor_OnButton(&g_editor,
                (delta_pr > 0) ? FE_BTN_RIGHT : FE_BTN_LEFT);
        }
    }
    else
    {
        /* ---- 导航模式 ---- */
        if (delta_up != 0)
        {
            int8_t idx = (int8_t)g_param_index + (int8_t)delta_up;
            if (idx < 0) idx = 0;
            if (idx > 3) idx = 3;
            g_param_index = (uint8_t)idx;

            /* 切换到新参数时重新绑定编辑器 */
            if (g_param_index < 3)
            {
                float *target = NULL;
                float range_max = 0.0f;
                uint8_t int_d = 0, dec_d = 0;

                switch (g_param_index)
                {
                case 0:
                    target = &dp.Vset;
                    range_max = 45.0f;
                    int_d = 2; dec_d = 2;
                    break;
                case 1:
                    target = &dp.Iset;
                    range_max = 10.0f;
                    int_d = 2; dec_d = 2;
                    break;
                case 2:
                    target = &dp.Pset;
                    range_max = 200.0f;
                    int_d = 3; dec_d = 1;
                    break;
                }
                FloatEditor_Init(&g_editor, target, 0.0f, range_max, int_d, dec_d);
            }
        }
    }

    /* ============================================================
     *  独立按键 Key_Enable (PD7) 处理
     * ============================================================ */

    switch (evey)
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
            PID_Incremental_Calc(&PID_Voltage, dp.Vset + PID_Current.output, ADC_Value.Vout - ADC_Value.Iout * 0.01f);
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
