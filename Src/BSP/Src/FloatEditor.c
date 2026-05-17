//coding : utf-8
/**
 * @file FloatEditor.c
 * @brief 浮点参数编辑状态机实现
 *
 * 核心设计：编辑值以缩放整数存储（如 dec_digits=2 时 = 实际值 × 100），
 * 所有加减运算使用精确整数，彻底消除 IEEE 754 浮点累加误差。
 *
 * 步长计算：raw_step = scale × 10^cursor_pos
 *   cursor_pos >= 0: raw_step = scale * 10^cursor_pos  （整数乘法）
 *   cursor_pos <  0: raw_step = scale / 10^(-cursor_pos) （整数除法，始终整除）
 */
#include "FloatEditor.h"

/* ================================================================
 *  内部辅助
 * ================================================================ */

/** @brief 计算 10^n（n >= 0） */
static s32 pow10_int(int8_t n)
{
    s32 v = 1;
    while (n-- > 0) v *= 10;
    return v;
}

/** @brief 计算当前 cursor_pos 对应的原始步长（整数） */
static s32 raw_step_for(const FloatEditor_t *e)
{
    if (e->cursor_pos >= 0) {
        return e->scale * pow10_int(e->cursor_pos);
    } else {
        return e->scale / pow10_int(-e->cursor_pos);
    }
}

/* ================================================================
 *  公开 API 实现
 * ================================================================ */

/**
 * @brief 初始化编辑状态机
 */
void FloatEditor_Init(FloatEditor_t *e, float *target, float min, float max,
                      uint8_t int_digits, uint8_t dec_digits)
{
    s32 scale = pow10_int((int8_t)dec_digits);

    e->target         = target;
    e->scale          = scale;
    e->edit_value_raw = (s32)(*target * (float)scale + 0.5f);
    e->raw_min        = (s32)(min * (float)scale + 0.5f);
    e->raw_max        = (s32)(max * (float)scale + 0.5f);
    e->cursor_pos     = 0; //修改默认指向个位
    e->max_cursor     = (int8_t)int_digits - 1;
    e->min_cursor     = -(int8_t)dec_digits;
    e->int_digits     = int_digits;
    e->dec_digits     = dec_digits;
    e->is_editing     = false;
}

/**
 * @brief 进入编辑模式
 */
void FloatEditor_StartEdit(FloatEditor_t *e)
{
    e->is_editing = true;
}

/**
 * @brief 处理编码器旋转
 *
 * 使用精确整数运算，无浮点累加误差。
 */
void FloatEditor_OnEncoder(FloatEditor_t *e, int8_t delta)
{
    if (!e->is_editing) return;

    s32 raw_step = raw_step_for(e);
    e->edit_value_raw += (s32)delta * raw_step;

    if (e->edit_value_raw > e->raw_max) e->edit_value_raw = e->raw_max;
    if (e->edit_value_raw < e->raw_min) e->edit_value_raw = e->raw_min;
}

/**
 * @brief 处理按键事件
 */
void FloatEditor_OnButton(FloatEditor_t *e, FloatEditor_Button_t btn)
{
    switch (btn) {
    case FE_BTN_OK:
        *e->target = (float)e->edit_value_raw / (float)e->scale;
        e->is_editing = false;
        break;

    case FE_BTN_CANCEL:
        e->edit_value_raw = (s32)(*e->target * (float)e->scale + 0.5f);
        e->is_editing = false;
        break;

    case FE_BTN_LEFT:
        if (e->cursor_pos < e->max_cursor) {
            e->cursor_pos++;
        }
        break;

    case FE_BTN_RIGHT:
        if (e->cursor_pos > e->min_cursor) {
            e->cursor_pos--;
        }
        break;
    }
}

/**
 * @brief 渲染编辑值为显示字符串，并计算光标反显位置
 *
 * 直接从缩放整数提取各位数字，无需浮点运算。
 */
char* FloatEditor_Render(const FloatEditor_t *e, char *buf, uint8_t *cursor_char)
{
    s32 val = e->edit_value_raw;

    s32 int_part  = val / e->scale;
    s32 frac_part = val % e->scale;

    char *p = buf;

    /* 整数位（从高位到低位） */
    for (int8_t pos = e->max_cursor; pos >= 0; pos--) {
        s32 divisor = pow10_int(pos);
        *p++ = '0' + ((int_part / divisor) % 10);
    }

    /* 小数位（从十分位到最低位） */
    for (int8_t pos = -1; pos >= e->min_cursor; pos--) {
        s32 divisor = pow10_int((int8_t)e->dec_digits + pos);
        *p++ = '0' + ((frac_part / divisor) % 10);
    }
    *p = '\0';

    /* 映射 cursor_pos 到 buf 中的字符索引 */
    if (e->cursor_pos >= 0) {
        *cursor_char = (uint8_t)(e->max_cursor - e->cursor_pos);
    } else {
        *cursor_char = (uint8_t)(e->max_cursor + 1 + (-(e->cursor_pos + 1)));
    }

    return buf;
}

/**
 * @brief 获取当前 cursor_pos 对应的步长（float，供外部显示参考）
 */
float FloatEditor_GetStep(const FloatEditor_t *e)
{
    return (float)raw_step_for(e) / (float)e->scale;
}

/**
 * @brief 查询是否处于编辑模式
 */
bool FloatEditor_IsEditing(const FloatEditor_t *e)
{
    return e->is_editing;
}

/**
 * @brief 获取编辑值的 float 表示
 */
float FloatEditor_GetEditValue(const FloatEditor_t *e)
{
    return (float)e->edit_value_raw / (float)e->scale;
}

/*
 * ================================================================
 *  使用例程
 * ================================================================
 *
 * FloatEditor_t editor;
 * FloatEditor_Init(&editor, &dp.Vset, 0.0f, 45.0f, 2, 2);
 *
 * // ---- 主循环 ----
 * if (editor.is_editing) {
 *     FloatEditor_OnEncoder(&editor, delta);
 * }
 *
 * // ---- 按键 ----
 * void on_encoder_click(void) {
 *     if (editor.is_editing)
 *         FloatEditor_OnButton(&editor, FE_BTN_OK);
 *     else
 *         FloatEditor_StartEdit(&editor);
 * }
 *
 * void on_encoder_long_press(void) {
 *     if (editor.is_editing)
 *         FloatEditor_OnButton(&editor, FE_BTN_CANCEL);
 * }
 *
 * // ---- 渲染 ----
 * char buf[12]; uint8_t cursor_char;
 * FloatEditor_Render(&editor, buf, &cursor_char);
 * // int_digits=2: buf[0]=tens, buf[1]=ones, buf[2]=tenths, buf[3]=hundredths
 * for (int i = 0; i < editor.int_digits; i++)
 *     OLED_ShowChar(x + i * 8, y, buf[i], OLED_8X16);
 * OLED_ShowChar(x + editor.int_digits * 8, y, '.', OLED_8X16); // 小数点
 * for (int i = 0; i < editor.dec_digits; i++)
 *     OLED_ShowChar(x + (editor.int_digits + 1 + i) * 8, y,
 *                   buf[editor.int_digits + i], OLED_8X16);
 * if (editor.is_editing) {
 *     uint8_t cx = cursor_char;
 *     if (cursor_char >= editor.int_digits) cx++;
 *     OLED_ReverseArea(x + cx * 8, y, 8, 16);
 * }
 */
