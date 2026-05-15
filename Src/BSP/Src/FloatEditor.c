/**
 * @file FloatEditor.c
 * @brief 浮点参数编辑状态机实现
 *
 * 使用静态查找表替代 powf()，避免 RV32EC 无 FPU 时的软浮点库调用开销。
 * cursor_pos 范围受 max_cursor/min_cursor 约束，查表偏移量确保不越界。
 */

#include "FloatEditor.h"

/* ================================================================
 *  步长查找表
 *
 *  索引 = cursor_pos + STEP_TABLE_OFFSET
 *
 *  cursor_pos:  -4     -3     -2     -1      0      1      2
 *  step:      0.0001  0.001  0.01   0.1     1.0   10.0  100.0
 * ================================================================ */

static const float step_table[] = {
    0.0001f,  /* cursor_pos = -4 */
    0.001f,   /* cursor_pos = -3 */
    0.01f,    /* cursor_pos = -2 */
    0.1f,     /* cursor_pos = -1 */
    1.0f,     /* cursor_pos =  0 */
    10.0f,    /* cursor_pos =  1 */
    100.0f,   /* cursor_pos =  2 */
};

#define STEP_TABLE_OFFSET  4  /**< cursor_pos 为 0 时的表索引 */
#define STEP_TABLE_MAX     6  /**< 最大有效索引 */

/** @brief 安全查表，cursor_pos 越界时返回最近的有效步长 */
static float step_lookup(int8_t cursor_pos)
{
    int idx = (int)cursor_pos + STEP_TABLE_OFFSET;
    if (idx < 0) idx = 0;
    if (idx > STEP_TABLE_MAX) idx = STEP_TABLE_MAX;
    return step_table[idx];
}

/* ================================================================
 *  公开 API 实现
 * ================================================================ */

/**
 * @brief 初始化编辑状态机
 *
 * 绑定目标参数，设定合法范围和显示格式。
 * 初始状态为非编辑模式，cursor_pos 设为零（个位）。
 */
void FloatEditor_Init(FloatEditor_t *e, float *target, float min, float max,
                      uint8_t int_digits, uint8_t dec_digits)
{
    e->target       = target;
    e->edit_value   = *target;
    e->min          = min;
    e->max          = max;
    e->cursor_pos   = 0;
    e->max_cursor   = (int8_t)int_digits - 1;
    e->min_cursor   = -(int8_t)dec_digits;
    e->int_digits   = int_digits;
    e->dec_digits   = dec_digits;
    e->is_editing   = false;
}

/**
 * @brief 进入编辑模式
 *
 * 从实际参数同步当前值，光标定位到最低位（小数末位），
 * 便于逐位向上调整。
 */
void FloatEditor_StartEdit(FloatEditor_t *e)
{
    e->edit_value = *e->target;
    e->cursor_pos = e->min_cursor;
    e->is_editing = true;
}

/**
 * @brief 处理编码器旋转
 *
 * 按 cursor_pos 对应的步长修改 edit_value，并钳位到 [min, max]。
 */
void FloatEditor_OnEncoder(FloatEditor_t *e, int8_t delta)
{
    if (!e->is_editing) return;

    float step = step_lookup(e->cursor_pos);
    e->edit_value += (float)delta * step;

    if (e->edit_value > e->max) e->edit_value = e->max;
    if (e->edit_value < e->min) e->edit_value = e->min;
}

/**
 * @brief 处理按键事件
 *
 * - FE_BTN_OK:     确认编辑，将 edit_value 写回 *target，退出编辑模式
 * - FE_BTN_CANCEL: 取消编辑，恢复 *target 原值，退出编辑模式
 * - FE_BTN_LEFT:   光标向高位移动（cursor_pos++）
 * - FE_BTN_RIGHT:  光标向低位移动（cursor_pos--）
 */
void FloatEditor_OnButton(FloatEditor_t *e, FloatEditor_Button_t btn)
{
    switch (btn) {
    case FE_BTN_OK:
        *e->target = e->edit_value;
        e->is_editing = false;
        break;

    case FE_BTN_CANCEL:
        e->edit_value = *e->target;
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
 * 不使用 sprintf/printf，用整数运算逐位提取数字，
 * 节省栈空间和代码体积。
 *
 * 显示格式（以 int_digits=2, dec_digits=2, edit_value=3.14 为例）：
 *
 *   buf 内容:  "0314"
 *   buf 索引:   0123
 *   cursor_pos:  1  0 -1 -2  (对应十位/个位/十分位/百分位)
 *
 * 小数点在 OLED 上由调用者根据 int_digits 确定绘制位置，
 * 不在 buf 中占用字符。
 *
 * @param e           编辑器实例指针（只读）
 * @param buf         输出缓冲区，长度至少 int_digits + dec_digits + 1
 * @param cursor_char 输出：当前 cursor_pos 在 buf 中对应的字符索引
 * @return buf 指针
 */
char* FloatEditor_Render(const FloatEditor_t *e, char *buf, uint8_t *cursor_char)
{
    float val = e->edit_value;
    if (val < 0.0f) val = 0.0f;
    if (val > e->max) val = e->max;

    /* 分离整数和小数部分 */
    int int_part = (int)val;
    float frac = val - (float)int_part;

    /* 生成整数位数字（从高位到低位） */
    char *p = buf;
    for (int8_t pos = e->max_cursor; pos >= 0; pos--) {
        int divisor = 1;
        for (int8_t i = 0; i < pos; i++) {
            divisor *= 10;
        }
        *p++ = '0' + ((int_part / divisor) % 10);
    }

    /* 生成小数位数字（从十分位到最低位） */
    for (int8_t pos = -1; pos >= e->min_cursor; pos--) {
        int multiplier = 1;
        for (int8_t i = -1; i >= pos; i--) {
            multiplier *= 10;
        }
        int digit = ((int)(frac * (float)multiplier + 0.5f)) % 10;
        *p++ = '0' + digit;
    }
    *p = '\0';

    /* 映射 cursor_pos 到 buf 中的字符索引 */
    if (e->cursor_pos >= 0) {
        *cursor_char = (uint8_t)(e->max_cursor - e->cursor_pos);
    } else {
        /* 小数位：跳过所有整数位 */
        *cursor_char = (uint8_t)(e->max_cursor + 1 + (-(e->cursor_pos + 1)));
    }

    return buf;
}

/**
 * @brief 获取当前 cursor_pos 对应的步长
 */
float FloatEditor_GetStep(const FloatEditor_t *e)
{
    return step_lookup(e->cursor_pos);
}

/**
 * @brief 查询是否处于编辑模式
 */
bool FloatEditor_IsEditing(const FloatEditor_t *e)
{
    return e->is_editing;
}

/*
 * ================================================================
 *  使用例程
 * ================================================================
 *
 * // ----- 初始化 -----
 * FloatEditor_t editor;
 * FloatEditor_Init(&editor, &dp.Vset, 0.0f, 45.0f, 2, 2);
 *
 * // ----- 主循环中处理编码器 -----
 * // 读取编码器增量（由 ISR 写入 scratch 变量）
 * s32 delta_val = read_and_clear_delta();
 *
 * if (editor.is_editing) {
 *     FloatEditor_OnEncoder(&editor, (int8_t)delta_val);
 * } else {
 *     // 导航模式：delta 用于切换参数索引
 *     g_param_index += delta_val;
 *     // ... 切换到新参数时重新 Init ...
 * }
 *
 * // ----- 按键处理 -----
 * void on_encoder_click(void) {
 *     if (editor.is_editing)
 *         FloatEditor_OnButton(&editor, FE_BTN_OK);    // 确认
 *     else
 *         FloatEditor_StartEdit(&editor);              // 进入编辑
 * }
 *
 * void on_encoder_long_press(void) {
 *     if (editor.is_editing)
 *         FloatEditor_OnButton(&editor, FE_BTN_CANCEL); // 取消
 *     // ... 切换输出使能 ...
 * }
 *
 * void on_encoder_rotate_while_pressed(int8_t delta) {
 *     if (editor.is_editing)
 *         FloatEditor_OnButton(&editor,
 *             (delta > 0) ? FE_BTN_RIGHT : FE_BTN_LEFT);
 * }
 *
 * // ----- OLED 渲染 -----
 * char buf[12];
 * uint8_t cursor_char;
 * FloatEditor_Render(&editor, buf, &cursor_char);
 *
 * // 绘制数字和小数点
 * // int_digits=2, dec_digits=2: "0314"
 * //   [0]='0' [1]='3' 小数点 [2]='1' [3]='4'
 * for (int i = 0; i < editor.int_digits; i++) {
 *     OLED_ShowChar(x + i * 8, y, buf[i], OLED_8X16);
 * }
 * OLED_ShowChar(x + editor.int_digits * 8, y, '.', OLED_8X16);
 * for (int i = 0; i < editor.dec_digits; i++) {
 *     OLED_ShowChar(x + (editor.int_digits + 1 + i) * 8, y,
 *                   buf[editor.int_digits + i], OLED_8X16);
 * }
 *
 * // 反显当前编辑位
 * if (editor.is_editing) {
 *     uint8_t cx;
 *     if (cursor_char >= editor.int_digits) {
 *         // 小数位，跳过小数点字符
 *         cx = cursor_char + 1;
 *     } else {
 *         cx = cursor_char;
 *     }
 *     OLED_ReverseArea(x + cx * 8, y, 8, 16);
 * }
 */
