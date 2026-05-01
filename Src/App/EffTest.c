/**
 * @file EffTest.c
 * @brief 效率测试状态机实现
 *
 * 独立模块，通过 EFF_TEST_ENABLE 宏编译开关。
 * 状态流转: WARMING → (单击) → RUNNING → (步进完成) → FINISHED → (超时) → IDLE
 */

#include "debug.h"
#include "EffTest.h"
#include "ADC.h"
#include "Digital_Power.h"
#include "OLED_UI.h"
#include "OLED.h"
#include "flash_param.h"

#if EFF_TEST_ENABLE

/* ========== 硬件宏 ========== */
#define FAN_ENABLE  GPIO_SetBits(GPIOD, GPIO_Pin_7)
#define FAN_DISABLE GPIO_ResetBits(GPIOD, GPIO_Pin_7)

/* ========== 全局实例 ========== */
EffTest_t g_eff_test = {EFF_TEST_IDLE};

/* ========== 静态辅助 ========== */

/**
 * @brief 从 ADC_Value 计算实时效率
 * @return 效率值 (%), 无效时返回 0
 */
static float calc_eff(void)
{
    if (ADC_Value.Pout > 0.1f && ADC_Value.Pin > 0.1f)
    {
        float eff = (ADC_Value.Pout / ADC_Value.Pin) * 100.0f;
        if (eff > 100.0f) eff = 100.0f;
        if (eff < 0.0f)   eff = 0.0f;
        return eff;
    }
    return 0.0f;
}

/* ========== API 实现 ========== */

void EffTest_Start(void)
{
    g_eff_test.state = EFF_TEST_WARMING;
    g_eff_test.current_vout = EFF_VSTART;
    g_eff_test.step_count = 0;
    g_eff_test.step_timeout = 0;
    g_eff_test.step_settle = 0;
    g_eff_test.efficiency = 0.0f;
    printf("EFF_START\r\n");
}

uint8_t EffTest_IsFanForced(void)
{
    return (g_eff_test.state == EFF_TEST_RUNNING) ? 1 : 0;
}

void EffTest_OnKeyClick(void)
{
    if (g_eff_test.state == EFF_TEST_WARMING)
    {
        /* 单击 → 启动步进测试 */
        g_eff_test.state = EFF_TEST_RUNNING;
        g_eff_test.step_count = 0;
        g_eff_test.current_vout = EFF_VSTART;
        g_eff_test.step_timeout = 0;
        g_eff_test.step_settle = 0;
        FAN_ENABLE;
        dp.Vset = g_eff_test.current_vout;
        dp.System_Enable_Flag = 1;
        dp.sys_state = DP_CV_Switch;
        return;
    }

    if (g_eff_test.state == EFF_TEST_RUNNING)
    {
        /* 单击 → 中断测试 */
        printf("EFF_ABORT\r\n");
        dp.Vset = flash_data.cfg.Vset;
        dp.System_Enable_Flag = 0;
        dp.sys_state = DP_IDLE_Switch;
        g_eff_test.state = EFF_TEST_IDLE;
        OLED_UI_ExitSettings();
        return;
    }
}

void EffTest_OnKeyLongPress(void)
{
    if (g_eff_test.state == EFF_TEST_WARMING)
    {
        /* 长按 → 退出 */
        g_eff_test.state = EFF_TEST_IDLE;
        OLED_UI_ExitSettings();
    }
}

void EffTest_Proc(void)
{
    if (g_eff_test.state == EFF_TEST_IDLE ||
        g_eff_test.state == EFF_TEST_WARMING)
    {
        return;
    }

    /* ── RUNNING: 步进测试 ── */
    if (g_eff_test.state == EFF_TEST_RUNNING)
    {
        /* 保护/异常中止 */
        if ((dp.sys_state >= DP_OVP && dp.sys_state <= DP_FAULT_LOCK) ||
            !dp.System_Enable_Flag)
        {
            printf("EFF_ABORT");
            if (dp.sys_state >= DP_OVP) printf(" protection\r\n");
            else                        printf("\r\n");
            dp.Vset = flash_data.cfg.Vset;
            g_eff_test.state = EFF_TEST_IDLE;
            OLED_UI_ExitSettings();
            return;
        }

        g_eff_test.step_timeout++;

        /* ---- 静置等待电源稳定 ---- */
        if (g_eff_test.step_settle < EFF_SETTLE_TICKS)
        {
            g_eff_test.step_settle++;
            return;
        }

        /* ---- 读取效率并记录 ---- */
        g_eff_test.efficiency = calc_eff();
        printf("EFF,%.2f,%.1f\r\n",
               g_eff_test.current_vout, g_eff_test.efficiency);

        /* ---- 前进到下一步 ---- */
        g_eff_test.step_count++;
        g_eff_test.current_vout = EFF_VSTART + g_eff_test.step_count * EFF_VSTEP;

        /* ---- 超时自动放弃 ---- */
        if (g_eff_test.step_timeout >= EFF_TIMEOUT_TICKS)
        {
            printf("EFF,%.2f,T/O\r\n", g_eff_test.current_vout);
            g_eff_test.step_count++;
            g_eff_test.current_vout = EFF_VSTART + g_eff_test.step_count * EFF_VSTEP;
            g_eff_test.step_timeout = 0;
            g_eff_test.step_settle = 0;
        }

        /* ---- 完成或继续 ---- */
        if (g_eff_test.current_vout > EFF_VEND || g_eff_test.step_count >= EFF_TOTAL_STEPS)
        {
            printf("EFF_END\r\n");
            dp.Vset = flash_data.cfg.Vset;
            dp.System_Enable_Flag = 0;
            dp.sys_state = DP_IDLE_Switch;
            g_eff_test.state = EFF_TEST_FINISHED;
        }
        else
        {
            dp.Vset = g_eff_test.current_vout;
            g_eff_test.step_timeout = 0;
            g_eff_test.step_settle = 0;
        }
        return;
    }

    /* ── FINISHED: 停留后自动返回 ── */
    if (g_eff_test.state == EFF_TEST_FINISHED)
    {
        static uint16_t finish_ticks = 0;
        finish_ticks++;
        if (finish_ticks > EFF_FINISH_TICKS)
        {
            finish_ticks = 0;
            g_eff_test.state = EFF_TEST_IDLE;
            OLED_UI_ExitSettings();
        }
    }
}

void EffTest_Draw(void)
{
    OLED_Clear();

    if (g_eff_test.state == EFF_TEST_WARMING)
    {
        OLED_ShowString(0, 0, "Eff Test", OLED_8X16);
        OLED_ShowString(0, 20, "Connect 3A load", OLED_6X8);
        OLED_ShowString(0, 30, "keep load ON", OLED_6X8);
        OLED_ShowString(0, 50, "Press to start", OLED_6X8);
    }
    else if (g_eff_test.state == EFF_TEST_RUNNING)
    {
        /* 标题和进度 */
        OLED_ShowString(0, 0, "Eff Test", OLED_8X16);
        OLED_Printf(80, 0, OLED_6X8, "%d/87", g_eff_test.step_count + 1);

        /* 当前电压 */
        OLED_ShowString(0, 20, "Vout:", OLED_8X16);
        OLED_ShowFloatNum_Nosigned(50, 20, g_eff_test.current_vout, 2, 2, OLED_8X16);
        OLED_ShowString(100, 20, "V", OLED_8X16);

        /* 实时效率 */
        float eff = calc_eff();
        OLED_ShowString(0, 40, "Eff:", OLED_8X16);
        OLED_Printf(50, 40, OLED_8X16, "%3d", (int)(eff + 0.5f));
        OLED_ShowString(90, 40, "%", OLED_8X16);

        /* 简易进度条 */
        uint8_t bar_w = (g_eff_test.step_count * 128) / 86;
        if (bar_w > 128) bar_w = 128;
        OLED_DrawRectangle(0, 60, bar_w, 4, OLED_FILLED);
    }
    else if (g_eff_test.state == EFF_TEST_FINISHED)
    {
        OLED_ShowString(0, 0, "Eff Test", OLED_8X16);
        OLED_ShowString(0, 24, "Complete!", OLED_8X16);
        OLED_ShowString(0, 40, "Check UART data", OLED_6X8);
        OLED_ShowString(0, 50, "on PC", OLED_6X8);
    }

    OLED_Update();
}

#endif /* EFF_TEST_ENABLE */

/*
Usage example:
    // In main loop (every ~10ms):
    EffTest_Proc();

    // In key event handler:
    EffTest_OnKeyClick();
    EffTest_OnKeyLongPress();

    // From settings menu:
    EffTest_Start();

    // Fan control:
    if (EffTest_IsFanForced()) FAN_ENABLE;

    // In renderer:
    if (g_eff_test.state != EFF_TEST_IDLE) EffTest_Draw();
*/
