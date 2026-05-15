/**
 * @file Responders.h
 * @brief 事件响应器初始化接口汇总
 */

#ifndef __RESPONDERS_H__
#define __RESPONDERS_H__

/**
 * @brief 初始化 UI 响应器
 *
 * 订阅: 所有保护事件 (OVP/OCP/SCP/OVT)
 * 动作: 调用 OLED_UI_XXX() 更新显示
 */
void UI_Responder_Init(void);

/**
 * @brief 初始化 LED 响应器
 *
 * 订阅: 模式切换事件 (CV/CC/IDLE) + 保护事件
 * 动作: 调用 BSP_WS2812_Set_Color() 改变 LED 颜色
 */
void LED_Responder_Init(void);

/**
 * @brief 初始化 Buzzer 响应器
 *
 * 订阅: 模式切换事件 + 保护事件
 * 动作: 调用 Buzzer_Play() 播放不同的蜂鸣器模式
 */
void Buzzer_Responder_Init(void);

/**
 * @brief 初始化所有响应器 (便利函数)
 *
 * 调用位置: main() 初始化中，Event_Bus_Init() 之后
 *
 * Usage:
 *   Event_Bus_Init();
 *   Responders_Init_All();  // 一次性初始化所有响应器
 */
static inline void Responders_Init_All(void)
{
    UI_Responder_Init();
    LED_Responder_Init();
    Buzzer_Responder_Init();
}

#endif // __RESPONDERS_H__
