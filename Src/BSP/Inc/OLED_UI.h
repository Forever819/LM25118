#include <ch32v00X.h>
#include "OLED.h"

#define CURSOR_AREA_NUM 4

typedef struct
{
    s16 x;
    s16 y;
    u8 width;
    u8 height;
    u8 digit;
    u8 point_location;
} cursor_area_t;

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

/* ========== 交互接口（供 Key_Event_Proc 调用） ========== */

/**
 * @brief 编码器单击事件处理
 * - 无 sub_cursor 时：进入 sub_cursor 模式（选中某一位）
 * - 有 sub_cursor 时：退出 sub_cursor 模式，回到光标选择
 */
void OLED_UI_OnEncoderClick(void);

/**
 * @brief 进入设置页面（由 cursor_index == 0 时触发）
 */
void OLED_UI_EnterSettings(void);

/**
 * @brief 退出设置页面，返回主界面
 */
void OLED_UI_ExitSettings(void);

/**
 * @brief 判断当前是否在设置页面中
 */
u8 OLED_UI_IsInSettings(void);

extern OLED_UI_Context_t g_oled_ui;
