/**
 * @file Buzzer_Responder.c
 * @brief Buzzer 事件响应器 - 订阅系统事件，播放不同的蜂鸣器模式
 */

#include "debug.h"
#include "Event_Bus.h"
#include "Buzzer.h"


#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

/**
 * @brief Buzzer 模式事件处理 - 模式转换时的蜂鸣
 */
static void Buzzer_Mode_Handler(PowerEvent_t *ev)
{
    switch (ev->type) {
    case EVENT_MODE_CC_ACTIVE:
        // CC 模式: 两次 3300Hz 音
        xprintf("[Buzzer] CC mode: 3300Hz x2\r\n");
        Buzzer_Play(3300, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(3300, 100);
        break;

    case EVENT_MODE_CV_ACTIVE:
        // CV 模式: 两次 3000Hz 音
        xprintf("[Buzzer] CV mode: 3000Hz x2\r\n");
        Buzzer_Play(3000, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(3000, 100);
        break;

    case EVENT_MODE_IDLE:
        // IDLE 模式: 静音
        xprintf("[Buzzer] IDLE mode: silent\r\n");
        break;

    default:
        break;
    }
}

/**
 * @brief Buzzer 保护事件处理 - 保护触发时的警报音
 */
static void Buzzer_Protection_Handler(PowerEvent_t *ev)
{
    switch (ev->type) {
    case EVENT_PROTECTION_OVP:
        // OVP: 2200Hz x2
        xprintf("[Buzzer] OVP alarm: 2200Hz x2\r\n");
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        break;

    case EVENT_PROTECTION_OCP:
        // OCP: 2200Hz x2
        xprintf("[Buzzer] OCP alarm: 2200Hz x2\r\n");
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        break;

    case EVENT_PROTECTION_SCP:
        // SCP: 2200Hz x4 (最严重，次数最多)
        xprintf("[Buzzer] SCP alarm: 2200Hz x4\r\n");
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        break;

    case EVENT_PROTECTION_OVT:
        // OVT: 2200Hz x2
        xprintf("[Buzzer] OVT alarm: 2200Hz x2\r\n");
        Buzzer_Play(2200, 100);
        Buzzer_Play(0, 100);
        Buzzer_Play(2200, 100);
        break;

    default:
        break;
    }
}

/**
 * @brief 初始化 Buzzer 响应器 - 订阅模式转换与保护事件
 *
 * 调用位置: main() 初始化中，Event_Bus_Init() 之后
 *
 * Usage: Buzzer_Responder_Init();
 */
void Buzzer_Responder_Init(void)
{
    // 订阅模式事件
    Event_Bus_Subscribe(EVENT_MODE_CV_ACTIVE, Buzzer_Mode_Handler);
    Event_Bus_Subscribe(EVENT_MODE_CC_ACTIVE, Buzzer_Mode_Handler);
    Event_Bus_Subscribe(EVENT_MODE_IDLE, Buzzer_Mode_Handler);

    // 订阅保护事件
    Event_Bus_Subscribe(EVENT_PROTECTION_OVP, Buzzer_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OCP, Buzzer_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_SCP, Buzzer_Protection_Handler);
    Event_Bus_Subscribe(EVENT_PROTECTION_OVT, Buzzer_Protection_Handler);

    xprintf("[Buzzer Responder] Initialized\r\n");
}

/*
 * Usage Example in main():
 *
 * int main(void) {
 *     // ...
 *     Event_Bus_Init();
 *     UI_Responder_Init();      // UI 响应器
 *     LED_Responder_Init();     // LED 响应器
 *     Buzzer_Responder_Init();  // 蜂鸣器响应器（此处）
 *     // ...
 * }
 */
