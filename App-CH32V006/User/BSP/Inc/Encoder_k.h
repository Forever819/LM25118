#ifndef ENCODER_K_H
#define ENCODER_K_H

#include "debug.h"

// 按键状态机状态
typedef enum {
    KEY_STATE_IDLE = 0,    // 空闲
    KEY_STATE_PRESSING,    // 按下中（等待判断）
    KEY_STATE_LONG_PRESS,  // 长按确认
    KEY_STATE_ROTATED,     // 按下期间发生了旋转（屏蔽按键事件）
} Key_State_t;

// 按键事件（由用户轮询获取）
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_CLICK,       // 单击
    KEY_EVENT_DOUBLE_PRESS,
    KEY_EVENT_LONG_PRESS,  // 长按（持续触发或单次，见配置）
} Key_Event_t;

typedef struct {
    s32 *cnt;      // 指向被修改的值
    s32 step_val;  // 步进值
    s32 max;       // 上限
    s32 min;       // 下限
} Encoder_CNT_t;

// 完整编码器+按键对象
typedef struct {
    Encoder_CNT_t unpressed;  // 未按下旋转配置
    Encoder_CNT_t pressed;    // 按下旋转配置

    // 按键状态机（在SysTick/定时器中更新）
    Key_State_t key_state;
    Key_Event_t key_event;     // 待取走的事件

    u32 press_tick;            // 按下时刻tick
    u32 long_press_ms;         // 长按判定时间（ms），默认500

    u8 rotated_while_pressed;  // 按下期间是否旋转过
} Encoder_t;

typedef struct {
    GPIO_TypeDef *GPIOX;
    uint16_t GPIO_Pin;
    uint8_t key_valid_val;
    // 按键状态机（在SysTick/定时器中更新）
    Key_State_t key_state;
    Key_Event_t key_event;  // 待取走的事件

    u32 press_tick;         // 按下时刻tick
    u32 long_press_ms;      // 长按判定时间（ms），默认500
} Key_t;

// ---- API ----
void BSP_Encoder_Init (Encoder_t *enc);
void BSP_Key_Init (Key_t *key);
void BSP_Encoder_Tick (Encoder_t *enc);              // 放入SysTick_Handler，每1ms调用
void BSP_Key_Tick (Key_t *key);

Key_Event_t BSP_Encoder_Get_Event (Encoder_t *enc);  // 主循环轮询
Key_Event_t BSP_Key_Get_Event (Key_t *key);

void BSP_Encoder_CNT_Attach (Encoder_CNT_t *e, s32 *value_addr);
void BSP_Encoder_CNT_Detach (Encoder_CNT_t *e);
void BSP_Encoder_Set_Step (Encoder_CNT_t *e, s32 step);
void BSP_Encoder_Set_Range (Encoder_CNT_t *e, s32 min, s32 max);
s32 BSP_Encoder_Get_Cnt (Encoder_CNT_t *e);
void EXTI7_0_IRQHandler_Callback (void);

extern Key_t Key_Enable;
extern Encoder_t encoder1;

#endif