/**
 * @file UI_Responder.c
 * @brief UI 层事件响应器 - 订阅保护/模式事件，更新 OLED 显示
 */

#include "debug.h"
#include "Event_Bus.h"
#include "OLED_UI.h"

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

/**
 * @brief UI 保护事件处理 - 保护触发时更新 OLED 显示
 */
static void OLED_UI_Handler(PowerEvent_t *ev)
{
    switch (ev->type)
    {
    case EVENT_PROTECTION_OVP:
        OLED_UI_OVP();
        break;

    case EVENT_PROTECTION_OCP:
        OLED_UI_OCP();
        break;

    case EVENT_PROTECTION_SCP:
        OLED_UI_SCP();
        break;

    case EVENT_PROTECTION_OTP:
        OLED_UI_OTP();
        break;
    case EVENT_PROTECTION_OPP:
        OLED_UI_OPP();
        break;

    case EVENT_MODE_IDLE:
        OLED_UI_ExitSettings();
        break;

    default:
        break;
    }
}

/**
 * @brief 初始化 UI 响应器 - 订阅所有需要 UI 反馈的事件
 *
 * 调用位置: main() 初始化中，Event_Bus_Init() 之后
 *
 * Usage: UI_Responder_Init();
 */
void UI_Responder_Init(void)
{
    Event_Bus_Subscribe(EVENT_PROTECTION_OVP, OLED_UI_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OCP, OLED_UI_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_SCP, OLED_UI_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OTP, OLED_UI_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OPP, OLED_UI_Handler);
    Event_Bus_Subscribe(EVENT_MODE_IDLE, OLED_UI_Handler);

    xprintf("[UI Responder] Initialized\r\n");
}

/*
 * Usage Example in main():
 *
 * int main(void) {
 *     // ...
 *     Event_Bus_Init();
 *     UI_Responder_Init();      // 初始化 UI 响应器
 *     LED_Responder_Init();     // 初始化 LED 响应器
 *     Buzzer_Responder_Init();  // 初始化蜂鸣器响应器
 *     // ...
 * }
 */
