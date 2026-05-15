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

/** @brief 设置页面中的设置项数量 */
#define SETTINGS_COUNT 2

/** @brief 当前显示的页面 */
typedef enum
{
    page_main = 0,
    page_settings = 1,
} oled_page_e;

/* ========== UI 生命周期 ========== */
void OLED_UI_Init(void);
void OLED_Ram_Update(void);

/* ========== 保护事件显示 ========== */
void OLED_UI_OVP(void);
void OLED_UI_OCP(void);
void OLED_UI_SCP(void);
void OLED_UI_OVT(void);

/* ========== 设置页面 ========== */
void OLED_UI_EnterSettings(void);
void OLED_UI_ExitSettings(void);
u8 OLED_UI_IsInSettings(void);

/* ========== 全局状态（供 main.c 访问） ========== */

/** @brief 全局编辑器实例 */
extern FloatEditor_t g_editor;

/** @brief 当前选中的参数索引：0=Vset, 1=Iset, 2=Pset, 3=设置入口 */
extern uint8_t g_param_index;

/** @brief 设置页面的光标位置 */
extern uint8_t g_settings_cursor;

#endif /* OLED_UI_H */
