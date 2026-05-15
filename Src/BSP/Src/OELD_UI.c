#include "OLED_UI.h"
#include "bm_img.h"
#include "Digital_Power.h"
#include "ADC_k.h"
#include "Filter_k.h"
#include "WS2812_k.h"
#include "Encoder_k.h"
#include "flash_param.h"

cursor_area_t cursor_area[CURSOR_AREA_NUM], *cursor;
s32 cursor_index = 1; // defualt vset
s32 sub_cursor = 0;
u8 Fault_trigger_flag = 0;

/* 设置页面状态 */
static oled_page_e OLED_UI_Current_Page = page_main;

mean_filter_t mf_vin, mf_iin, mf_vout, mf_iout;

static u32 my_pow(s32 x, u32 y)
{
    s32 val = 1;
    while (y--)
    {
        val *= x;
    }
    return val;
}

/* ===================================================================
 *  交互接口（供 Key_Event_Proc 调用）
 *  职责：管理 sub_cursor 的进入/退出，以及设置页面的切换
 *  注意：这些函数只操作 UI 内部状态，不涉及业务逻辑
 * =================================================================== */

/* ===================================================================
 *  设置页面渲染
 *  在设置页面中，cursor_index 表示选中的设置项
 * =================================================================== */

static void OLED_UI_Draw_Settings(void)
{
    OLED_Clear();
    OLED_ShowString(32, 0, "SETTINGS", OLED_8X16);

    /* 绘制设置项列表 */
    OLED_ShowString(0, 20, "OLED Flip", OLED_8X16);
    OLED_ShowString(0, 36, "Buzzer", OLED_8X16);

    /* 高亮当前选中的设置项 */
    OLED_ReverseArea(0, 20 + cursor_index * 16, 128, 16);

    /* 根据选中的设置项配置编码器 */
    switch (cursor_index)
    {
    case 0:
        BSP_Encoder_CNT_Attach(&encoder1.unpressed, &flash_data.cfg.oled_direction);
        BSP_Encoder_Set_Range(&encoder1.unpressed, 0, 1);
        BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
        OLED_Flip(flash_data.cfg.oled_direction);
        break;
    case 1:
        /* 预留：蜂鸣器开关等设置项 */
        break;
    default:
        break;
    }
}

/* ===================================================================
 *  设置入口页面（cursor_index == 4）
 * =================================================================== */

static void OLED_UI_Draw_SettingsEntry(void)
{
    OLED_ShowString(32, 0, "SETTINGS", OLED_8X16);
    OLED_ShowString(0, 24, "Press to enter", OLED_8X16);
    OLED_ShowString(0, 40, "settings page", OLED_8X16);

    if (!sub_cursor)
    {
        OLED_ReverseArea(cursor->x, cursor->y, cursor->width, cursor->height);
        BSP_Encoder_CNT_Detach(&encoder1.pressed);
    }
}

/* ===================================================================
 *  编码器配置（进入 sub_cursor 时调用一次）
 *  将原来散落在 OLED_Ram_Update 中的编码器配置集中到这里
 * =================================================================== */

static void OLED_UI_ConfigureEncoderForSubCursor(void)
{
    if (!sub_cursor)
        return;

    BSP_Encoder_Set_Range(&encoder1.pressed, 1, cursor->digit);
    BSP_Encoder_CNT_Attach(&encoder1.pressed, &sub_cursor);

    switch (cursor_index)
    {
    case 0:
        /* cursor_index == 0 现在是"设置入口"，进入设置页面 */
        OLED_UI_EnterSettings();
        return;
    case 1:
        BSP_Encoder_CNT_Attach(&encoder1.unpressed, &dp.Vset);
        BSP_Encoder_Set_Range(&encoder1.unpressed, 200, 4500);
        break;
    case 2:
        BSP_Encoder_CNT_Attach(&encoder1.unpressed, &dp.Iset);
        BSP_Encoder_Set_Range(&encoder1.unpressed, 0, 1000);
        break;
    case 3:
        BSP_Encoder_CNT_Attach(&encoder1.unpressed, &dp.Pset);
        BSP_Encoder_Set_Range(&encoder1.unpressed, 0, 20000);
        break;
    default:
        break;
    }

    u32 step = 0;
    if (sub_cursor < cursor->point_location)
        step = my_pow(10, cursor->digit - sub_cursor - 1 + cursor->point_location - 3);
    else if (sub_cursor > cursor->point_location)
        step = my_pow(10, cursor->digit - sub_cursor + cursor->point_location - 3);
    else
        step = 0;
    BSP_Encoder_Set_Step(&encoder1.unpressed, step);
}

static void OLED_UI_Draw_Static(void)
{
    OLED_ShowString(32, 0, "IN", OLED_8X16);
    OLED_ShowString(78, 0, "OUT", OLED_8X16);

    OLED_Printf(115, 20, OLED_8X16, "V");
    OLED_Printf(115, 32, OLED_8X16, "A");
    OLED_Printf(115, 48, OLED_8X16, "W");
    OLED_DrawLine(65, 0, 65, 64);
}

static void OLED_UI_Draw_Temperature(float temperature)
{
    const s16 x = 0, y = 10;
    const u8 w = 20, h = 54;
    const s16 bar_x = x + 2, bar_y = y + 8;
    const u8 bar_w = 4, bar_h_total = 36;

    float temp_norm = (temperature - 20.0f) / 80.0f; // 20~100C 映射到 0~100%
    if (temp_norm < 0.0f)
        temp_norm = 0.0f;
    if (temp_norm > 1.0f)
        temp_norm = 1.0f;

    OLED_ClearArea(x, y + 10, w, h);
    OLED_Printf(0, 56, OLED_6X8, "%2d", (int)(temperature + 0.5f));
    OLED_ShowChar(12, 56, 'C', OLED_6X8);

    OLED_DrawRectangle(bar_x, bar_y, bar_w, bar_h_total, OLED_UNFILLED);
    u8 bar_h = (u8)((bar_h_total - 2) * temp_norm);
    s16 bar_bottom = bar_y + bar_h_total - 2;
    s16 bar_top = bar_bottom - bar_h;
    if (bar_top < bar_y + 1)
        bar_top = bar_y + 1;
    OLED_DrawLine(bar_x + 1, bar_top, bar_x + 1, bar_bottom);
    OLED_DrawLine(bar_x + 2, bar_top, bar_x + 2, bar_bottom);
}

static void OLED_UI_Animation(void)
{
    const char anim_char[4] = {'|', '/', '-', '\\'};
    static u8 temp_anim_frame = 0;
    OLED_ClearArea(120, 0, 8, 8);
    OLED_ShowChar(120, 0, anim_char[temp_anim_frame & 0x03], OLED_6X8);
    temp_anim_frame++;
}

void OLED_UI_OnEncoderClick(void)
{
    if (OLED_UI_Current_Page == page_settings)
    {
        /* 在设置页面中：单击退出设置页面，返回主界面 */
        OLED_UI_ExitSettings();
        return;
    }

    /* 在设置入口页面（cursor_index == 4）单击：进入设置页面 */
    if (cursor_index == 4)
    {
        OLED_UI_EnterSettings();
        return;
    }

    if (!sub_cursor)
    {
        /* 进入 sub_cursor 模式：选中最后一位，并配置编码器 */
        sub_cursor = cursor->digit - 1;
        OLED_UI_ConfigureEncoderForSubCursor();
    }
    else
    {
        /* 退出 sub_cursor 模式：回到光标选择 */
        sub_cursor = 0;
        BSP_Encoder_CNT_Attach(&encoder1.unpressed, &cursor_index);
        BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
        BSP_Encoder_Set_Range(&encoder1.unpressed, 0, CURSOR_AREA_NUM - 1);
    }
}

void OLED_UI_EnterSettings(void)
{
    OLED_UI_Current_Page = page_settings;
    sub_cursor = 0;
    cursor_index = 0;
    OLED_Clear();
    OLED_UI_Draw_Static();

    /* 将编码器绑定到设置项选择 */
    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &cursor_index);
    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
    BSP_Encoder_Set_Range(&encoder1.unpressed, 0, 1); /* 暂时只有2个设置项 */
}

void OLED_UI_ExitSettings(void)
{
    OLED_UI_Current_Page = page_main;
    sub_cursor = 0;
    cursor_index = 1; /* 回到 Vset */
    OLED_Clear();
    OLED_UI_Draw_Static();

    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &cursor_index);
    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
    BSP_Encoder_Set_Range(&encoder1.unpressed, 0, CURSOR_AREA_NUM - 1);
}

u8 OLED_UI_IsInSettings(void)
{
    return (OLED_UI_Current_Page == page_settings);
}

void OLED_UI_OVP(void)
{
    if (Fault_trigger_flag == 1)
        return;
    Fault_trigger_flag = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OVP DETECTED!", OLED_8X16);
}

void OLED_UI_OVT(void)
{
    if (Fault_trigger_flag == 1)
        return;
    Fault_trigger_flag = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OVER TEMP!", OLED_8X16);
}

void OLED_UI_OCP(void)
{
    if (Fault_trigger_flag == 1)
        return;
    Fault_trigger_flag = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OCP DETECTED!", OLED_8X16);
}

void OLED_UI_SCP(void)
{
    if (Fault_trigger_flag == 1)
        return;
    Fault_trigger_flag = 1;
    OLED_ShowImage(0, 0, 128, 64, IMG_SCP);
}

/*
 * @brief OLED UI 初始化 - 在 main() 中调用
 *
 * 初始化 OLED 显示，均值滤波器，UI 相关数据结构，以及相关硬件（按键、编码器、WS2812 LED）
 *
 */
void OLED_UI_Init(void)
{
    OLED_Init();

    Mean_Filter_Init(&mf_vin);
    Mean_Filter_Init(&mf_iin);
    Mean_Filter_Init(&mf_vout);
    Mean_Filter_Init(&mf_iout);

    cursor_area[0].x = 120;
    cursor_area[0].y = 0;
    cursor_area[0].width = 8 * 8;
    cursor_area[0].height = 8;
    cursor_area[0].digit = 0;
    cursor_area[0].point_location = 0;

    cursor_area[1].x = 70;
    cursor_area[1].y = 17;
    cursor_area[1].width = 5 * 8;
    cursor_area[1].height = 16;
    cursor_area[1].digit = 5;
    cursor_area[1].point_location = 3;

    cursor_area[2].x = 70;
    cursor_area[2].y = 32;
    cursor_area[2].width = 5 * 8;
    cursor_area[2].height = 16;
    cursor_area[2].digit = 5;
    cursor_area[2].point_location = 3;

    cursor_area[3].x = 70;
    cursor_area[3].y = 48;
    cursor_area[3].width = 5 * 8;
    cursor_area[3].height = 16;
    cursor_area[3].digit = 5;
    cursor_area[3].point_location = 4;

    OLED_UI_Draw_Static();

    Buzzer_TIM_Init();
    BSP_WS2812_Init();
    BSP_Key_Init(&Key_Enable);
    
    BSP_Encoder_Init(&encoder1);

    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &cursor_index);
    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
    BSP_Encoder_Set_Range(&encoder1.unpressed, 0, CURSOR_AREA_NUM - 1);
}

void OLED_Ram_Update(void)
{
    if (dp.sys_state == FAULT_Lock)
    {
        return;
    }

    /* ========== 设置页面渲染 ========== */
    if (OLED_UI_Current_Page == page_settings)
    {
        OLED_UI_Draw_Settings();
        return;
    }

    /* ========== 设置入口页面 (cursor_index == 4) ========== */
    if (cursor_index == 4)
    {
        OLED_UI_Draw_SettingsEntry();
        return;
    }

    /* ========== 主界面渲染 ========== */

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
    if (dp.System_Enable_Flag)
    {
        OLED_ShowFloatNum_Nosigned(70, 17, mf_vout.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum_Nosigned(70, 32, mf_iout.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum_Nosigned(70, 48, mf_vout.filter_out * mf_iout.filter_out, 3, 1, OLED_8X16);
        if (dp.sys_state == CV)
        {
            OLED_Printf(2, 0, OLED_6X8, "CV  ");
        }
        else if (dp.sys_state == CC)
        {
            OLED_Printf(2, 0, OLED_6X8, "CC  ");
        }

        /* 效率显示 */
        if (power_in > 0.1f)
        {
            s32 eff = (s32)((power_out / (power_in)) * 100.0f + 0.5f);
            if (eff < 0)
                eff = 0;
            if (eff > 100)
                eff = 100;
            OLED_Printf(0, 9, OLED_6X8, "%3d", eff);
        }
        else
        {
            OLED_ShowString(0, 9, "---", OLED_6X8);
        }
    }
    else
    {
        OLED_ShowFloatNum_Nosigned(70, 17, dp.Vset / 100.0f, 2, 2, OLED_8X16);
        OLED_ShowFloatNum_Nosigned(70, 32, dp.Iset / 100.0f, 2, 2, OLED_8X16);
        OLED_ClearArea(70, 48, 40, 16);
        OLED_Printf(2, 0, OLED_6X8, "IDLE");
    }
    /* other things*/
    OLED_UI_Draw_Temperature(ADC_Value.Inductance_Temperature);
    OLED_UI_Animation();

    /* ========== 光标渲染（纯显示，不配置编码器） ========== */
    if (!sub_cursor)
    {
        OLED_ReverseArea(cursor->x, cursor->y, cursor->width, cursor->height);
    }
    else
    {
        OLED_ReverseArea(
            cursor->x + (cursor->width / cursor->digit) * (sub_cursor - 1),
            cursor->y,
            cursor->width / cursor->digit,
            cursor->height);
    }
}
