/**
 * @file OELD_UI.c
 * @brief OLED 用户界面 —— 显示渲染层实现
 *
 * 纯渲染逻辑，不包含编辑状态管理。编辑状态由 FloatEditor 管理。
 * 本文件负责：主界面渲染、设置页面渲染、保护事件显示、温度柱状图、动画。
 */

#include "OLED_UI.h"
#include "bm_img.h"

#include "WS2812.h"
#include "Encoder.h"
#include "Buzzer.h"
#include "Digital_Power.h"
#include "ADC.h"
#include "Filter.h"
#include "flash_param.h"

/* ================================================================
 *  全局状态
 * ================================================================ */

FloatEditor_t  g_editor;
uint8_t        g_param_index = 0;

ParamDisplay_t g_param_display[3] = {
    {70, 17, OLED_8X16, 2, 2},   /* Vset: XX.XX */
    {70, 32, OLED_8X16, 2, 2},   /* Iset: XX.XX */
    {70, 48, OLED_8X16, 3, 1},   /* Pset: XXX.X */
};

/* ================================================================
 *  设置页面状态
 * ================================================================ */

static oled_page_e OLED_UI_Current_Page = page_main;
uint8_t g_settings_cursor = 0;
u8 Fault_trigger_flag = 0;

/* ================================================================
 *  均值滤波器（测量值平滑）
 * ================================================================ */

static mean_filter_t mf_vin, mf_iin, mf_vout, mf_iout;

/* ================================================================
 *  静态辅助函数
 * ================================================================ */

/**
 * @brief 绘制设置页面
 * @note 使用 g_settings_cursor 选择设置项，主循环中的编码器增量由 main.c 转发
 */
static void OLED_UI_Draw_Settings(void)
{
    OLED_Clear();
    OLED_ShowString(32, 0, "SETTINGS", OLED_8X16);

    OLED_ShowString(0, 20, "OLED Flip", OLED_8X16);
    OLED_ShowString(0, 36, "Buzzer", OLED_8X16);

    /* 高亮当前选中的设置项 */
    OLED_ReverseArea(0, 20 + (int)g_settings_cursor * 16, 128, 16);

    /* 显示当前值 */
    switch (g_settings_cursor) {
    case 0:
        OLED_ShowString(100, 20,
            flash_data.cfg.oled_direction ? "ON " : "OFF", OLED_8X16);
        break;
    case 1:
        /* 预留：蜂鸣器开关 */
        OLED_ShowString(100, 36, "--", OLED_8X16);
        break;
    }
}

/**
 * @brief 绘制设置入口页面
 */
static void OLED_UI_Draw_SettingsEntry(void)
{
    OLED_ShowString(32, 0, "SETTINGS", OLED_8X16);
    OLED_ShowString(0, 24, "Press to enter", OLED_8X16);
    OLED_ShowString(0, 40, "settings page", OLED_8X16);
}

/**
 * @brief 绘制静态 UI 元素（一次性绘制，除非被 Clear 覆盖）
 */
static void OLED_UI_Draw_Static(void)
{
    OLED_ShowString(32, 0, "IN", OLED_8X16);
    OLED_ShowString(78, 0, "OUT", OLED_8X16);

    OLED_Printf(115, 20, OLED_8X16, "V");
    OLED_Printf(115, 32, OLED_8X16, "A");
    OLED_Printf(115, 48, OLED_8X16, "W");
    OLED_DrawLine(65, 0, 65, 64);
}

/**
 * @brief 绘制温度柱状图
 */
static void OLED_UI_Draw_Temperature(float temperature)
{
    const s16 x = 0, y = 10;
    const u8 w = 20, h = 54;
    const s16 bar_x = x + 2, bar_y = y + 8;
    const u8 bar_w = 4, bar_h_total = 36;

    float temp_norm = (temperature - 20.0f) / 80.0f;
    if (temp_norm < 0.0f) temp_norm = 0.0f;
    if (temp_norm > 1.0f) temp_norm = 1.0f;

    OLED_ClearArea(x, y + 10, w, h);
    OLED_Printf(0, 56, OLED_6X8, "%2d", (int)(temperature + 0.5f));
    OLED_ShowChar(12, 56, 'C', OLED_6X8);

    OLED_DrawRectangle(bar_x, bar_y, bar_w, bar_h_total, OLED_UNFILLED);
    u8 bar_h = (u8)((bar_h_total - 2) * temp_norm);
    s16 bar_bottom = bar_y + bar_h_total - 2;
    s16 bar_top = bar_bottom - bar_h;
    if (bar_top < bar_y + 1) bar_top = bar_y + 1;
    OLED_DrawLine(bar_x + 1, bar_top, bar_x + 1, bar_bottom);
    OLED_DrawLine(bar_x + 2, bar_top, bar_x + 2, bar_bottom);
}

/**
 * @brief 旋转动画指示器
 */
static void OLED_UI_Animation(void)
{
    const char anim_char[4] = {'|', '/', '-', '\\'};
    static u8 temp_anim_frame = 0;
    OLED_ClearArea(120, 0, 8, 8);
    OLED_ShowChar(120, 0, anim_char[temp_anim_frame & 0x03], OLED_6X8);
    temp_anim_frame++;
}

/* ================================================================
 *  FloatEditor 渲染辅助
 * ================================================================ */

/**
 * @brief 在指定位置渲染参数值（编辑模式下带光标反显）
 *
 * @param pd          参数显示描述符
 * @param value       非编辑模式下的显示值
 * @param is_selected 是否被选中（导航高亮）
 * @param is_editing  是否正在编辑此参数
 */
static void OLED_UI_Draw_Param(const ParamDisplay_t *pd, float value,
                               bool is_selected, bool is_editing)
{
    char buf[12];
    uint8_t ch_w = (pd->font_size == OLED_8X16) ? 8 : 6;
    uint8_t ch_h = (pd->font_size == OLED_8X16) ? 16 : 12;

    if (is_editing) {
        /* 编辑模式：逐字符绘制，带光标反显 */
        uint8_t cursor_char;
        FloatEditor_Render(&g_editor, buf, &cursor_char);

        /* 绘制整数位 */
        for (uint8_t i = 0; i < g_editor.int_digits; i++) {
            OLED_ShowChar(pd->x + (int)i * ch_w, pd->y, buf[i], pd->font_size);
        }
        /* 小数点 */
        OLED_ShowChar(pd->x + (int)g_editor.int_digits * ch_w, pd->y,
                      '.', pd->font_size);
        /* 小数位 */
        for (uint8_t i = 0; i < g_editor.dec_digits; i++) {
            OLED_ShowChar(pd->x + (int)(g_editor.int_digits + 1 + i) * ch_w,
                          pd->y, buf[g_editor.int_digits + i], pd->font_size);
        }

        /* 反显当前编辑位 */
        uint8_t cx;
        if (cursor_char >= g_editor.int_digits) {
            cx = cursor_char + 1;  /* 跳过小数点字符 */
        } else {
            cx = cursor_char;
        }
        OLED_ReverseArea(pd->x + (int)cx * ch_w, pd->y, ch_w, ch_h);
    } else {
        /* 非编辑模式：标准浮点显示 */
        OLED_ShowFloatNum_Nosigned(pd->x, pd->y, value,
                                   pd->int_digits, pd->dec_digits, pd->font_size);

        /* 选中高亮 */
        if (is_selected) {
            u8 total_w = (u8)(pd->int_digits + 1 + pd->dec_digits) * ch_w;
            OLED_ReverseArea(pd->x, pd->y, total_w, ch_h);
        }
    }
}

/* ================================================================
 *  公开 API
 * ================================================================ */

/**
 * @brief OLED UI 初始化
 *
 * 初始化 OLED、均值滤波器、UI 数据结构，以及相关外设（按键、编码器、WS2812）。
 * 编码器不再在此处绑定 —— 由 main.c 统一绑定到增量累加器。
 */
void OLED_UI_Init(void)
{
    OLED_Init();

    Mean_Filter_Init(&mf_vin);
    Mean_Filter_Init(&mf_iin);
    Mean_Filter_Init(&mf_vout);
    Mean_Filter_Init(&mf_iout);

    OLED_UI_Draw_Static();

    Buzzer_Init();
    BSP_WS2812_Init();
    BSP_Key_Init(&Key_Enable);
    BSP_Encoder_Init(&encoder1);

    /* 初始化 FloatEditor，默认绑定 Vset */
    FloatEditor_Init(&g_editor, &dp.Vset, 0.0f, 45.0f, 2, 2);
    g_param_index = 0;
}

/**
 * @brief 主界面渲染（每 10ms 调用一次）
 *
 * 渲染流程：
 *   1. 故障锁定状态：跳过渲染
 *   2. 设置页面：委托给 OLED_UI_Draw_Settings
 *   3. 设置入口（g_param_index == 3）：显示入口页面
 *   4. 主界面：输入侧测量值 + 输出侧显示（使能时测量值，禁用时设定值）
 */
void OLED_Ram_Update(void)
{
    if (dp.sys_state == FAULT_Lock) {
        return;
    }

    /* 设置页面 */
    if (OLED_UI_Current_Page == page_settings) {
        OLED_UI_Draw_Settings();
        return;
    }

    /* 设置入口页面 */
    if (g_param_index == 3) {
        OLED_UI_Draw_SettingsEntry();
        /* 高亮设置入口区域 */
        OLED_ReverseArea(120, 0, 8, 8);
        return;
    }

    /* ========== 主界面 ========== */

    /* 更新均值滤波器 */
    Mean_Filter_Update(&mf_vin, ADC_Value.Vin);
    Mean_Filter_Update(&mf_iin, ADC_Value.Iin);
    Mean_Filter_Update(&mf_vout, ADC_Value.Vout);
    Mean_Filter_Update(&mf_iout, ADC_Value.Iout);

    float power_in = mf_vin.filter_out * mf_iin.filter_out;
    float power_out = mf_vout.filter_out * mf_iout.filter_out;

    /* 输入侧显示 */
    OLED_ShowFloatNum_Nosigned(20, 17, mf_vin.filter_out, 2, 2, OLED_8X16);
    OLED_ShowFloatNum_Nosigned(20, 32, mf_iin.filter_out, 2, 2, OLED_8X16);
    OLED_ShowFloatNum_Nosigned(20, 48, power_in, 3, 1, OLED_8X16);

    /* 输出侧显示 */
    if (dp.System_Enable_Flag) {
        /* 输出使能：显示测量值 */
        OLED_ShowFloatNum_Nosigned(70, 17, mf_vout.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum_Nosigned(70, 32, mf_iout.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum_Nosigned(70, 48, power_out, 3, 1, OLED_8X16);

        if (dp.sys_state == CV) {
            OLED_Printf(2, 0, OLED_6X8, "CV  ");
        } else if (dp.sys_state == CC) {
            OLED_Printf(2, 0, OLED_6X8, "CC  ");
        }

        /* 效率显示 */
        if (power_in > 0.1f) {
            s32 eff = (s32)((power_out / power_in) * 100.0f + 0.5f);
            if (eff < 0) eff = 0;
            if (eff > 100) eff = 100;
            OLED_Printf(0, 9, OLED_6X8, "%3d", eff);
        } else {
            OLED_ShowString(0, 9, "---", OLED_6X8);
        }

        /* 如果正在编辑，取消编辑 */
        if (g_editor.is_editing) {
            FloatEditor_OnButton(&g_editor, FE_BTN_CANCEL);
        }
    } else {
        /* 输出禁用：显示设定值 */
        OLED_Printf(2, 0, OLED_6X8, "IDLE");

        /* 遍历三个参数 */
        float *params[3] = {&dp.Vset, &dp.Iset, &dp.Pset};
        for (int i = 0; i < 3; i++) {
            bool is_active = (g_editor.is_editing && g_param_index == (uint8_t)i);
            bool is_selected = (!g_editor.is_editing && g_param_index == (uint8_t)i);
            OLED_UI_Draw_Param(&g_param_display[i], *params[i],
                               is_selected, is_active);
        }
    }

    /* 温度与动画 */
    OLED_UI_Draw_Temperature(ADC_Value.Inductance_Temperature);
    OLED_UI_Animation();
}

/* ================================================================
 *  设置页面 API
 * ================================================================ */

void OLED_UI_EnterSettings(void)
{
    OLED_UI_Current_Page = page_settings;
    g_settings_cursor = 0;
    OLED_Clear();
}

void OLED_UI_ExitSettings(void)
{
    OLED_UI_Current_Page = page_main;
    g_param_index = 0;
    OLED_Clear();
    OLED_UI_Draw_Static();

    /* 重新绑定编辑器到 Vset */
    FloatEditor_Init(&g_editor, &dp.Vset, 0.0f, 45.0f, 2, 2);
}

u8 OLED_UI_IsInSettings(void)
{
    return (OLED_UI_Current_Page == page_settings);
}

/* ================================================================
 *  保护事件显示
 * ================================================================ */

void OLED_UI_OVP(void)
{
    if (Fault_trigger_flag == 1) return;
    Fault_trigger_flag = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OVP DETECTED!", OLED_8X16);
}

void OLED_UI_OVT(void)
{
    if (Fault_trigger_flag == 1) return;
    Fault_trigger_flag = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OVER TEMP!", OLED_8X16);
}

void OLED_UI_OCP(void)
{
    if (Fault_trigger_flag == 1) return;
    Fault_trigger_flag = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OCP DETECTED!", OLED_8X16);
}

void OLED_UI_SCP(void)
{
    if (Fault_trigger_flag == 1) return;
    Fault_trigger_flag = 1;
    OLED_ShowImage(0, 0, 128, 64, IMG_SCP);
}

/*
 * ================================================================
 *  使用例程
 * ================================================================
 *
 * 参见 FloatEditor.c 末尾的完整例程。
 * 本文件的角色仅限于调用 FloatEditor_Render 并绘制到 OLED。
 *
 * 架构分层：
 *
 *   main.c (Key_Event_Proc)
 *     │ 读取编码器增量、处理按键事件
 *     │
 *     ├─→ FloatEditor_OnEncoder / FloatEditor_OnButton  (编辑中)
 *     ├─→ g_param_index += delta                        (导航中)
 *     └─→ OLED_UI_EnterSettings / ExitSettings          (设置页)
 *     │
 *     ▼
 *   OELD_UI.c (OLED_Ram_Update)
 *     │ 只读取 FloatEditor 状态，执行渲染
 *     │ 调用 FloatEditor_Render 获取显示字符串
 *     │ 调用 OLED_ReverseArea 绘制光标
 *
 *   FloatEditor.c (编辑状态机)
 *     │ 持有编辑状态，不依赖任何外设
 *     │ step_lookup 替代 powf，纯算术运算
 */
