/**
 * @file OLED_UI.h
 * @brief OLED 用户界面 —— 显示渲染层
 *
 * 分层架构中的最上层，纯渲染逻辑：
 *
 *   [ 编码器 ] --> [ FloatEditor ] --> [ OLED_UI 显示渲染 ]
 *                        |
 *                  [ 参数数据层 ]
 *
 * 显示层只读取 FloatEditor 状态，不做任何编辑逻辑。
 */

#ifndef OLED_UI_H
#define OLED_UI_H

#include <ch32v00X.h>
#include "OLED.h"
#include "FloatEditor.h"

/** @brief 参数显示描述符 —— 将参数映射到屏幕上的位置和格式 */
typedef struct
{
    uint8_t x;          /**< OLED X 坐标 (像素) */
    uint8_t y;          /**< OLED Y 坐标 (像素) */
    uint8_t font_size;  /**< 字体大小: OLED_8X16 或 OLED_6X8 */
    uint8_t int_digits; /**< 整数显示位数 */
    uint8_t dec_digits; /**< 小数显示位数 */
} ParamDisplay_t;

/** @brief 设置页面中的设置项数量（含返回主页入口） */
#define SETTINGS_COUNT 7

/** @brief 设置页面一屏可见行数（标题 8x16 + 4 行 6x8） */
#define SETTINGS_VISIBLE_ROWS 4

/** @brief 当前显示的页面 */
typedef enum
{
    page_main = 0,
    page_settings = 1,
    page_fault_lock
} oled_page_e;

/* ========== UI ========= */
void OLED_UI_Init(void);
void OLED_UI_Reander(void);
void OLED_UI_Trigger_Page_Switch(oled_page_e page);

/* ========== 保护事件显示 ========== */
void OLED_UI_OVP(void);
void OLED_UI_OCP(void);
void OLED_UI_SCP(void);
void OLED_UI_OTP(void);
void OLED_UI_OPP(void);

/* ========== 设置页面 ========== */
void OLED_UI_EnterSettings(void);
void OLED_UI_ExitSettings(void);
u8 OLED_UI_IsInSettings(void);

/* ========== 全局状态（供 main.c 访问） ========== */

/** @brief 全局编辑器实例 */
extern FloatEditor_t g_param_editor;

/** @brief 当前选中的参数索引：0=Vset, 1=Iset, 2=Pset, 3=设置入口 */
extern uint8_t g_param_index;

/** @brief 设置页面的光标位置 */
extern uint8_t g_settings_cursor;

/** @brief 设置页面 FloatEditor 实例（编辑 slope 参数） */
extern FloatEditor_t g_settings_editor;

/** @brief 设置页面是否处于编辑模式 */
extern bool g_settings_is_editing;

/** @brief 设置项索引 2-5 对应的 slope 目标指针数组 */
extern float *g_slope_targets[4];

#endif /* OLED_UI_H */
