/**
 * @file FloatEditor.h
 * @brief 浮点参数编辑状态机
 *
 * 分层架构中的中间层 —— 编辑状态机：
 *
 *   [ 编码器 ] --> [ FloatEditor ] --> [ OLED 显示渲染 ]
 *                       |
 *                 [ 参数数据层 (dp.Vset 等) ]
 *
 * cursor_pos 使用有符号整数表示相对于小数点的位权：
 *   - 正数 = 整数位（0=个位, 1=十位, 2=百位）
 *   - 负数 = 小数位（-1=十分位, -2=百分位, -3=千分位）
 *
 * 编辑值以缩放整数存储（如 dec_digits=2 时，3.14 存储为 314），
 * 消除 IEEE 754 浮点累加误差。仅在确认/取消时与 float 互转一次。
 */

#ifndef FLOATEDITOR_H
#define FLOATEDITOR_H

#include "debug.h"
#include <stdbool.h>

/** @brief 编辑状态机接收的按键事件 */
typedef enum {
    FE_BTN_LEFT,    /**< 光标向高位移动 */
    FE_BTN_RIGHT,   /**< 光标向低位移动 */
    FE_BTN_OK,      /**< 确认编辑，写回实际参数 */
    FE_BTN_CANCEL   /**< 取消编辑，恢复原始值 */
} FloatEditor_Button_t;

/** @brief 浮点参数编辑状态机
 *
 * 编辑值以缩放整数 edit_value_raw 存储（单位 = 10^{-dec_digits}），
 * 所有加减运算使用精确整数，仅在读/写 *target 时做一次 float 转换。
 */
typedef struct {
    float   *target;         /**< 指向实际参数的指针（确认时才写入） */
    s32      edit_value_raw; /**< 编辑中的临时副本（缩放整数） */
    s32      raw_min;        /**< 最小值的缩放整数表示 */
    s32      raw_max;        /**< 最大值的缩放整数表示 */
    s32      scale;          /**< 缩放因子 = 10^dec_digits */
    int8_t   cursor_pos;     /**< 当前编辑位：0=个位, 1=十位, -1=十分位, -2=百分位 */
    int8_t   max_cursor;     /**< 最高可编辑位（整数部分最高位） */
    int8_t   min_cursor;     /**< 最低可编辑位（小数部分最低位） */
    uint8_t  int_digits;     /**< 整数部分显示位数 */
    uint8_t  dec_digits;     /**< 小数部分显示位数 */
    bool     is_editing;     /**< 是否处于编辑模式 */
} FloatEditor_t;

/* ================================================================
 *  API
 * ================================================================ */

void  FloatEditor_Init(FloatEditor_t *e, float *target, float min, float max,
                       uint8_t int_digits, uint8_t dec_digits);
void  FloatEditor_StartEdit(FloatEditor_t *e);
void  FloatEditor_OnEncoder(FloatEditor_t *e, int8_t delta);
void  FloatEditor_OnButton(FloatEditor_t *e, FloatEditor_Button_t btn);
char* FloatEditor_Render(const FloatEditor_t *e, char *buf, uint8_t *cursor_char);
float FloatEditor_GetStep(const FloatEditor_t *e);
bool  FloatEditor_IsEditing(const FloatEditor_t *e);

/**
 * @brief 获取编辑值的 float 表示（仅供调试/外部读取）
 * @param e 编辑器实例指针（只读）
 * @return edit_value_raw 转换为 float
 */
float FloatEditor_GetEditValue(const FloatEditor_t *e);

#endif /* FLOATEDITOR_H */
