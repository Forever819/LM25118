/**
 * @file LED_Responder.c
 * @brief LED (WS2812) 事件响应器 - 订阅系统状态/保护事件，更新 LED 颜色
 */

#include "debug.h"
#include "Event_Bus.h"
#include "WS2812.h"

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

// LED 颜色定义
#define W2812_CV_COLOR 0xff0000    // Red - DP_CV mode
#define W2812_CC_COLOR 0xFFD700    // Gold - DP_CC mode
#define W2812_IDLE_COLOR 0x001f00  // Dark green - DP_IDLE
#define W2812_FAULT_COLOR 0xFFFFFF // White - FAULT

/**
 * @brief LED 模式事件处理 - 根据系统模式设置 LED 颜色
 */
static void LED_Mode_Handler(PowerEvent_t *ev)
{
    switch (ev->type)
    {
    case EVENT_MODE_CV_ACTIVE:
        xprintf("[LED] DP_CV mode: red\r\n");
        BSP_WS2812_Set_Color(W2812_CV_COLOR);
        break;

    case EVENT_MODE_CC_ACTIVE:
        xprintf("[LED] DP_CC mode: gold\r\n");
        BSP_WS2812_Set_Color(W2812_CC_COLOR);
        break;

    case EVENT_MODE_IDLE:
        xprintf("[LED] DP_IDLE mode: dark green\r\n");
        BSP_WS2812_Set_Color(W2812_IDLE_COLOR);
        break;

    default:
        break;
    }
}

/**
 * @brief LED 保护事件处理 - 保护触发时设置 LED 为白色
 */
static void LED_Protection_Handler(PowerEvent_t *ev)
{
    switch (ev->type)
    {
    case EVENT_PROTECTION_OVP:
    case EVENT_PROTECTION_OCP:
    case EVENT_PROTECTION_SCP:
    case EVENT_PROTECTION_OTP:
    case EVENT_PROTECTION_OPP:
        xprintf("[LED] Protection event: white\r\n");
        BSP_WS2812_Set_Color(W2812_FAULT_COLOR);
        break;

    default:
        break;
    }
}

/**
 * @brief 初始化 LED 响应器 - 订阅模式转换与保护事件
 *
 * 调用位置: main() 初始化中，Event_Bus_Init() 之后
 *
 * Usage: LED_Responder_Init();
 */
void LED_Responder_Init(void)
{
    // 订阅模式事件
    Event_Bus_Subscribe(EVENT_MODE_CV_ACTIVE, LED_Mode_Handler);
    Event_Bus_Subscribe(EVENT_MODE_CC_ACTIVE, LED_Mode_Handler);
    Event_Bus_Subscribe(EVENT_MODE_IDLE, LED_Mode_Handler);

    // 订阅保护事件
    Event_Bus_Subscribe(EVENT_PROTECTION_OVP, LED_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OCP, LED_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_SCP, LED_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OTP, LED_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OPP, LED_Protection_Handler);

    // 初始状态: DP_IDLE
    BSP_WS2812_Set_Color(W2812_IDLE_COLOR);

    xprintf("[LED Responder] Initialized\r\n");
}

/*
 * Usage Example in main():
 *
 * int main(void) {
 *     // ...
 *     Event_Bus_Init();
 *     UI_Responder_Init();      // UI 响应器
 *     LED_Responder_Init();     // LED 响应器（此处）
 *     Buzzer_Responder_Init();  // 蜂鸣器响应器
 *     // ...
 * }
 */
