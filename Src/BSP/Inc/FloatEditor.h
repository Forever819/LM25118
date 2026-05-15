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
 * 步长计算：step = lookup_table[cursor_pos + offset]
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

/** @brief 浮点参数编辑状态机 */
typedef struct {
    float   *target;         /**< 指向实际参数的指针（确认时才写入） */
    float    edit_value;     /**< 编辑中的临时副本 */
    float    min;            /**< 合法范围最小值 */
    float    max;            /**< 合法范围最大值 */
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

/**
 * @brief 初始化编辑状态机
 * @param e            编辑器实例指针
 * @param target       指向实际参数的指针
 * @param min          允许的最小值
 * @param max          允许的最大值
 * @param int_digits   整数显示位数
 * @param dec_digits   小数显示位数
 */
void FloatEditor_Init(FloatEditor_t *e, float *target, float min, float max,
                      uint8_t int_digits, uint8_t dec_digits);

/**
 * @brief 进入编辑模式
 * @note 会从 *target 同步当前值到 edit_value
 */
void FloatEditor_StartEdit(FloatEditor_t *e);

/**
 * @brief 处理编码器旋转
 * @param e     编辑器实例指针
 * @param delta  旋转增量（+1 顺时针, -1 逆时针）
 * @note 仅在 is_editing 时生效，按当前 cursor_pos 对应的步长修改 edit_value
 */
void FloatEditor_OnEncoder(FloatEditor_t *e, int8_t delta);

/**
 * @brief 处理按键事件
 * @param e   编辑器实例指针
 * @param btn 按键事件类型
 */
void FloatEditor_OnButton(FloatEditor_t *e, FloatEditor_Button_t btn);

/**
 * @brief 渲染编辑值为显示字符串，并计算光标反显位置
 * @param e            编辑器实例指针（只读）
 * @param buf          输出字符串缓冲区（调用者分配，至少 int_digits+dec_digits+3 字节）
 * @param cursor_char  输出：需要反显的字符在 buf 中的索引
 * @return buf 指针（方便链式调用）
 */
char* FloatEditor_Render(const FloatEditor_t *e, char *buf, uint8_t *cursor_char);

/**
 * @brief 获取当前 cursor_pos 对应的步长
 * @param e 编辑器实例指针（只读）
 * @return 步长值（float）
 */
float FloatEditor_GetStep(const FloatEditor_t *e);

/**
 * @brief 查询是否处于编辑模式
 * @param e 编辑器实例指针（只读）
 * @return true 表示正在编辑
 */
bool FloatEditor_IsEditing(const FloatEditor_t *e);

#endif /* FLOATEDITOR_H */
