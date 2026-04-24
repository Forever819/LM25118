#include "OLED_UI.h"
#include "OLED.h"
#include "ADC_k.h"
#include "Encoder_k.h"
#include "Digital_Power.h"
#include "flash_param.h"
#include "bm_img.h"
cursor_area_t cursor_area[CURSOR_AREA_NUM], *cursor;
s32 cursor_index = 1; // defualt vset
s32 sub_cursor   = 0;

mean_filter_t mf_vin, mf_iin, mf_vout, mf_iout;
static u8 temp_anim_frame = 0;

static u32 my_pow(s32 x, u32 y)
{
    s32 val = 1;
    while (y--) {
        val *= x;
    }
    return val;
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
    const char anim_char[4] = {'|', '/', '-', '\\'};
    float temp_norm         = (temperature - 20.0f) / 80.0f; // 20~100C 映射到 0~100%
    if (temp_norm < 0.0f)
        temp_norm = 0.0f;
    if (temp_norm > 1.0f)
        temp_norm = 1.0f;

    OLED_ClearArea(x, y + 10, w, h);
    OLED_Printf(0, 56, OLED_6X8, "%2d", (int)(temperature + 0.5f));
    OLED_ShowChar(12, 56, 'C', OLED_6X8);
    OLED_ShowChar(120, 0, anim_char[temp_anim_frame & 0x03], OLED_6X8);
    temp_anim_frame++;

    OLED_DrawRectangle(bar_x, bar_y, bar_w, bar_h_total, OLED_UNFILLED);
    u8 bar_h       = (u8)((bar_h_total - 2) * temp_norm);
    s16 bar_bottom = bar_y + bar_h_total - 2;
    s16 bar_top    = bar_bottom - bar_h;
    if (bar_top < bar_y + 1)
        bar_top = bar_y + 1;
    OLED_DrawLine(bar_x + 1, bar_top, bar_x + 1, bar_bottom);
    OLED_DrawLine(bar_x + 2, bar_top, bar_x + 2, bar_bottom);
}

void OLED_UI_Init(void)
{

    Mean_Filter_Init(&mf_vin);
    Mean_Filter_Init(&mf_iin);
    Mean_Filter_Init(&mf_vout);
    Mean_Filter_Init(&mf_iout);

    cursor_area[0].x              = 0;
    cursor_area[0].y              = 9;
    cursor_area[0].width          = 4 * 6;
    cursor_area[0].height         = 8;
    cursor_area[0].digit          = 4;
    cursor_area[0].point_location = 3;

    cursor_area[1].x              = 70;
    cursor_area[1].y              = 17;
    cursor_area[1].width          = 5 * 8;
    cursor_area[1].height         = 16;
    cursor_area[1].digit          = 5;
    cursor_area[1].point_location = 3;

    cursor_area[2].x              = 70;
    cursor_area[2].y              = 32;
    cursor_area[2].width          = 5 * 8;
    cursor_area[2].height         = 16;
    cursor_area[2].digit          = 5;
    cursor_area[2].point_location = 3;

    cursor_area[3].x              = 70;
    cursor_area[3].y              = 48;
    cursor_area[3].width          = 5 * 8;
    cursor_area[3].height         = 16;
    cursor_area[3].digit          = 5;
    cursor_area[3].point_location = 4;

    OLED_UI_Draw_Static();
}

u8 Fault_trigger_flag1 = 0;
u8 Fault_trigger_flag2 = 0;

void OLED_UI_OVP(void)
{
    if (Fault_trigger_flag2)
        return;
    Fault_trigger_flag1 = 1;
    Fault_trigger_flag2 = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OVP DETECTED!", OLED_8X16);
}

void OLED_UI_OVT(void)
{
    if (Fault_trigger_flag2)
        return;
    Fault_trigger_flag1 = 1;
    Fault_trigger_flag2 = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OVER TEMP!", OLED_8X16);
}

void OLED_UI_OCP(void)
{
    if (Fault_trigger_flag2)
        return;
    Fault_trigger_flag1 = 1;
    Fault_trigger_flag2 = 1;
    OLED_Clear();
    OLED_Reverse();
    OLED_ShowString(0, 48, "OCP DETECTED!", OLED_8X16);
}

void OLED_UI_SCP(void)
{
    if (Fault_trigger_flag2)
        return;
    Fault_trigger_flag1 = 1;
    Fault_trigger_flag2 = 1;
    OLED_ShowImage(0, 0, 128, 64, IMG_SCP);
}

#define COOLINGFAN_POWER_CONSUMTION 2.0f

void OLED_Ram_Update(void)
{
    if (dp.sys_state == FAULT_Lock) {
        return;
    } else {
        if (Fault_trigger_flag1) {
            OLED_Clear();
            OLED_UI_Draw_Static();
            Fault_trigger_flag1 = 0;
            Fault_trigger_flag2 = 0;
        }
        Mean_Filter_Update(&mf_vin, ADC_Value.Vin);
        Mean_Filter_Update(&mf_iin, ADC_Value.Iin);
        Mean_Filter_Update(&mf_vout, ADC_Value.Vout);
        Mean_Filter_Update(&mf_iout, ADC_Value.Iout);

        float pin  = mf_vin.filter_out * mf_iin.filter_out;
        float pout = mf_vout.filter_out * mf_iout.filter_out;
        if (dp.System_Enable_Flag && pin > 0.1f) {
            s32 eff = (s32)((pout / (pin - COOLINGFAN_POWER_CONSUMTION)) * 100.0f + 0.5f);
            if (eff < 0)
                eff = 0;
            if (eff > 100)
                eff = 100;
            OLED_Printf(0, 9, OLED_6X8, "%3d", eff);
        } else
            OLED_ShowString(0, 9, "---", OLED_6X8);
        OLED_ShowFloatNum(20, 17, mf_vin.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum(20, 32, mf_iin.filter_out, 2, 2, OLED_8X16);
        OLED_ShowFloatNum(20, 48, mf_vin.filter_out * mf_iin.filter_out, 3, 1, OLED_8X16);
        OLED_UI_Draw_Temperature(ADC_Value.Inductance_Temperature);

        if (dp.System_Enable_Flag) {
            OLED_ShowFloatNum(70, 17, mf_vout.filter_out, 2, 2, OLED_8X16);
            OLED_ShowFloatNum(70, 32, mf_iout.filter_out, 2, 2, OLED_8X16);
            OLED_ShowFloatNum(70, 48, mf_vout.filter_out * mf_iout.filter_out, 3, 1, OLED_8X16);
            if (dp.sys_state == CV) {
                OLED_Printf(2, 0, OLED_6X8, "CV  ");
            } else if (dp.sys_state == CC) {
                OLED_Printf(2, 0, OLED_6X8, "CC  ");
            }
        } else {
            OLED_ShowFloatNum(70, 17, dp.Vset / 100.0f, 2, 2, OLED_8X16);
            OLED_ShowFloatNum(70, 32, dp.Iset / 100.0f, 2, 2, OLED_8X16);
            OLED_ClearArea(70, 48, 40, 16);
            OLED_Printf(2, 0, OLED_6X8, "IDLE");
        }

        if (!sub_cursor) {
            OLED_ReverseArea(cursor->x, cursor->y, cursor->width, cursor->height);
            BSP_Encoder_CNT_Detach(&encoder1.pressed);
        } else {
            OLED_ReverseArea(cursor->x + (cursor->width / cursor->digit) * (sub_cursor - 1), cursor->y, cursor->width / cursor->digit, cursor->height);
            BSP_Encoder_Set_Range(&encoder1.pressed, 1, cursor->digit);
            BSP_Encoder_CNT_Attach(&encoder1.pressed, &sub_cursor);
            switch (cursor_index) {
                case 0:
                    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &flash_data.cfg.oled_direction);
                    BSP_Encoder_Set_Range(&encoder1.unpressed, 0, 1);
                    BSP_Encoder_Set_Step(&encoder1.unpressed, 1);
                    OLED_Flip(flash_data.cfg.oled_direction);
                    return;
                    break;
                case 1:
                    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &dp.Vset);
                    BSP_Encoder_Set_Range(&encoder1.unpressed, 200, 4500);
                    break;
                case 2:

                    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &dp.Iset);
                    BSP_Encoder_Set_Range(&encoder1.unpressed, 0, 500);

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
            else if (sub_cursor > cursor->point_location) {
                step = my_pow(10, cursor->digit - sub_cursor + cursor->point_location - 3);
            } else
                step = 0;
            BSP_Encoder_Set_Step(&encoder1.unpressed, step);
        }
    }
}
