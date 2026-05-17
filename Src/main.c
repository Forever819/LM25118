/*coding : utf-8 */
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

#define FAN_ENABLE GPIO_SetBits(GPIOD, GPIO_Pin_7);
#define FAN_DISABLE GPIO_ResetBits(GPIOD, GPIO_Pin_7);

flash_param_t flash_data;
Digital_Power_Dev dp;
/**
 * @brief 编码器增量累加器
 *
 * 由 EXTI ISR 写入，由 main 循环在临界区内读取并清零。
 * 编码器在初始化时一次性绑定到这两个累加器，之后永不重新绑定。
 *
 * - g_enc_delta_unpressed: 非按压旋转增量（edit_value / param_index 导航）
 * - g_enc_delta_pressedessed:   按压旋转增量（cursor_pos 移动）
 */
volatile s32 g_enc_delta_unpressed = 0;
volatile s32 g_enc_delta_pressedessed = 0;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    xprintf("Systemclock %ld %f\r\n", SystemCoreClock, 3.14f);

    // 初始化硬件
    MCU_Flash_Read_Cfg();
    OLED_UI_Init();

    /* 编码器一次性绑定到增量累加器，之后永不重新绑定 */
    BSP_Encoder_CNT_Attach(&encoder1.unpressed, (s32 *)&g_enc_delta_unpressed);
    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
    BSP_Encoder_Set_Range(&encoder1.unpressed, -1000, 1000);
    BSP_Encoder_CNT_Attach(&encoder1.pressed, (s32 *)&g_enc_delta_pressedessed);
    BSP_Encoder_Set_Step(&encoder1.pressed, 1);
    BSP_Encoder_Set_Range(&encoder1.pressed, -1000, 1000);

    Digital_Power_Init();

    // 初始化业务逻辑和事件系统
    Digital_Power_Core_Init();
    Event_Bus_Init();
    Responders_Init_All(); // 初始化所有事件响应器

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    while (1)
    {
        // 读取温度传感器
        ADC_Value.Inductance_Temperature = NTC_GetTemperature(ADC_Regular_Data[4]);
        if(ADC_Value.Inductance_Temperature > 50)
        {
            FAN_ENABLE;
        }
        else if(ADC_Value.Inductance_Temperature < 30)
        {
            FAN_DISABLE;

        }
        // 处理用户交互
        Key_Event_Proc();

        // 消费 ISR 中入队的保护事件
        Event_Bus_Flush();

        // 执行业务逻辑，获取事件
        PowerEvent_t ev = Digital_Power_State_Event();
        if (ev.type != EVENT_NONE)
        {
            Event_Bus_Publish(&ev, event_trigger_immediately);
        }

        // UI 更新
        OLED_UI_Reander();

        // xprintf("Vin %.2f Vout%.2f\r\n", ADC_Value.Vin, ADC_Value.Vout);
        // xprintf("pos%d,temp1%.1f,temp2%ld,Vset%.1f\r\n", g_param_editor.cursor_pos, *g_param_editor.target, g_param_editor.edit_value_raw, dp.Vset);
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
    dp.Pset = flash_data.cfg.Pset;
    dp.System_Enable_Flag = 0;
    dp.sys_state = DP_IDLE_Switch;
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
    static bool settings_btn_reverse_flag = 1;
    /* ---- 读取编码器增量（临界区保护，防止 ISR 并发写入）---- */
    s32 delta_unpressed = 0, delta_pressed = 0;
    __disable_irq();
    delta_unpressed = g_enc_delta_unpressed;
    g_enc_delta_unpressed = 0;
    delta_pressed = g_enc_delta_pressedessed;
    g_enc_delta_pressedessed = 0;
    __enable_irq();

    /* ============================================================
     *  编码器按键事件（优先级高于增量）
     * ============================================================ */

    /* 长按编码器：
     * 主页面下：长按尝试使能CV模式
     */
    if (ev_encoder == KEY_EVENT_LONG_PRESS)
    {
        // /* 设置页面中：取消编辑或退出设置 */
        // if (OLED_UI_IsInSettings())
        // {
        //     if (g_settings_is_editing)
        //     {
        //         FloatEditor_OnButton(&g_settings_editor, FE_BTN_CANCEL);
        //         g_settings_is_editing = false;
        //     }
        //     else
        //     {
        //         OLED_UI_ExitSettings();
        //     }
        //     return;
        // }
        // if (g_param_editor.is_editing)
        // {
        //     FloatEditor_OnButton(&g_param_editor, FE_BTN_CANCEL);
        // }

        if (!dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 1;
            dp.sys_state = DP_CV_Switch;
        }
        return;
    }

    /* 单击编码器
     * 主页面下：短按取消使能，清零错误标志
     */

    if (ev_encoder == KEY_EVENT_CLICK)
    {
        // 清零错误标志
        if (dp.sys_state == DP_FAULT_LOCK)
        {
            dp.sys_state = DP_IDLE_Switch;
            return;
        }
        // 取消使能
        if (dp.System_Enable_Flag)
        {
            dp.System_Enable_Flag = 0;
            dp.sys_state = DP_IDLE_Switch;
            return;
        }
        /* 参数编辑：切换编辑/确认 */
        if (g_param_editor.is_editing)
        {
            FloatEditor_OnButton(&g_param_editor, FE_BTN_OK);
        }
        else
        {
            FloatEditor_StartEdit(&g_param_editor);
        }
        /* ---- 设置页面单击处理 ---- */
        if (OLED_UI_IsInSettings())
        {
            if (g_settings_cursor == SETTINGS_COUNT - 1)
            {
                /* "Back" 入口：返回主页 */
                OLED_UI_ExitSettings();
                settings_btn_reverse_flag = 1;
            }
            else if (g_settings_is_editing)
            {
                /* 编辑模式：确认写入 */
                FloatEditor_OnButton(&g_settings_editor, FE_BTN_OK);
                g_settings_is_editing = false;
            }
            else if (g_settings_cursor == 0)
            {
                /* OLED Flip：单击切换 */
                flash_data.cfg.oled_direction = !flash_data.cfg.oled_direction;
                OLED_Flip(flash_data.cfg.oled_direction);
            }
            else if (g_settings_cursor >= 2 && g_settings_cursor <= 5)
            {
                /* Slope 参数：进入编辑模式 */
                FloatEditor_StartEdit(&g_settings_editor);
                g_settings_is_editing = true;
            }
            /* Buzzer (cursor==1): 预留，无操作 */
            return;
        }

        /* 设置入口：单击进入设置页面 */
        if (g_param_index == 3)
        {
            OLED_UI_EnterSettings();
            return;
        }

        return;
    }

    /* ============================================================
     *  编码器增量处理
     * ============================================================ */

    /* 设置页面中的增量处理 */
    if (OLED_UI_IsInSettings())
    {
        if (g_settings_is_editing)
        {
            /* ---- 编辑模式：*/
            if (delta_unpressed != 0)
            {
                FloatEditor_OnEncoder(&g_settings_editor, (int8_t)delta_unpressed);
            }
            if (delta_pressed != 0)
            {
                FloatEditor_OnButton(&g_settings_editor,
                                     (delta_pressed > 0) ? FE_BTN_RIGHT : FE_BTN_LEFT);
            }
        }
        else
        {
            /* ---- 导航模式：切换设置项 ---- */
            if (delta_unpressed != 0)
            {
                int8_t idx = (int8_t)g_settings_cursor + (int8_t)delta_unpressed;
                if (idx < 0)
                    idx = 0;
                if (idx >= SETTINGS_COUNT)
                    idx = SETTINGS_COUNT - 1;
                g_settings_cursor = (uint8_t)idx;

                /* 切换到 float 设置项时重新绑定编辑器 */
                if (g_settings_cursor >= 2 && g_settings_cursor <= 5)
                {
                    uint8_t si = g_settings_cursor - 2;
                    FloatEditor_Init(&g_settings_editor, g_slope_targets[si],
                                     0.0001f, 0.1f, 1, 5);
                    g_settings_editor.cursor_pos = -4; /* 默认步长 0.0001 */
                }
            }
        }
    }
    /* 主页页面中的增量处理 */
    else if (g_param_editor.is_editing)
    {
        /* ---- 编辑模式 ---- */
        if (delta_unpressed != 0)
        {
            FloatEditor_OnEncoder(&g_param_editor, (int8_t)delta_unpressed);
        }
        if (delta_pressed != 0)
        {
            FloatEditor_OnButton(&g_param_editor,
                                 (delta_pressed > 0) ? FE_BTN_RIGHT : FE_BTN_LEFT);
        }
    }
    else
    {
        /* ---- 导航模式 ---- */
        if (delta_unpressed != 0)
        {
            int8_t idx = (int8_t)g_param_index + (int8_t)delta_unpressed;
            if (idx < 0)
                idx = 0;
            if (idx > 3)
                idx = 3;
            g_param_index = (uint8_t)idx;

            /* 切换到新参数时重新绑定编辑器 */
            if (g_param_index < 3)
            {
                if (!settings_btn_reverse_flag)
                {
                    OLED_ReverseArea(128 - 16, 0, 16, 16);
                    settings_btn_reverse_flag = 1;
                }
                float *target = NULL;
                float range_max = 0.0f;
                uint8_t int_d = 0, dec_d = 0;

                switch (g_param_index)
                {
                case 0:
                    target = &dp.Vset;
                    range_max = 45.0f;
                    int_d = 2;
                    dec_d = 2;
                    break;
                case 1:
                    target = &dp.Iset;
                    range_max = 10.0f;
                    int_d = 2;
                    dec_d = 2;
                    break;
                case 2:
                    target = &dp.Pset;
                    range_max = 150.0f;
                    int_d = 3;
                    dec_d = 1;
                    break;
                }
                FloatEditor_Init(&g_param_editor, target, 0.0f, range_max, int_d, dec_d);
            }
            else
            {
                // reader settings selection indicat
                if (settings_btn_reverse_flag)
                {
                    OLED_ReverseArea(128 - 16, 0, 16, 16);
                    settings_btn_reverse_flag = 0;
                }
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
        // static u8 flag = 1;
        // flag = !flag;
        // if(flag)
        // FAN_ENABLE
        // else
        // FAN_DISABLE
        // break;
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
    BSP_Key_Tick_GPIO(&Key_Enable);
    BSP_Buzzer_Tick();
}

void ADC_DMA_TC_Callback(void)
{
    BSP_ADC_Loop();

    // 检查所有保护条件（在中断中执行）
    // 返回 1 表示保护触发，阻止 PID 控制
    if (!Protection_Check())
    {
        if (dp.System_Enable_Flag)
        {
            // 未触发保护，执行 PID 控制
            PID_Incremental_Calc(&PID_Current, dp.Iset, ADC_Value.Iout);
            PID_Incremental_Calc(&PID_Voltage, dp.Vset + PID_Current.output, ADC_Value.Vout - ADC_Value.Iout * 0.01f);
            BSP_PWM_Set_CCR(PID_Voltage.output);
            LM25118_ENABLE
        }
        else
        {
            LM25118_DISABLE
        }
    }
    else
    {
        LM25118_DISABLE
    }
}
