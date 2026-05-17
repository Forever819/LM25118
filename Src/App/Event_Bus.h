/**
 * @file Event_Bus.h
 * @brief 事件发布-订阅框架，用于解耦业务逻辑与 UI 反应层
 */

#ifndef __EVENT_BUS_H__
#define __EVENT_BUS_H__

#include <ch32v00X.h>

/**
 * @enum PowerEvent_Type_t
 * @brief 电源系统事件类型
 */
typedef enum {
    EVENT_NONE = 0,
    EVENT_PROTECTION_SCP,      ///< 短路保护触发
    EVENT_PROTECTION_OCP,      ///< 过流保护触发
    EVENT_PROTECTION_OVP,      ///< 过压保护触发
    EVENT_PROTECTION_OTP,      ///< 过温保护触发
    EVENT_PROTECTION_OPP,      ///< 过功率保护触发
    EVENT_MODE_CC_ACTIVE,      ///< 进入恒流(DP_CC)模式
    EVENT_MODE_CV_ACTIVE,      ///< 进入恒压(DP_CV)模式
    EVENT_MODE_IDLE,           ///< 进入待机(DP_IDLE)模式
    EVENT_SYSTEM_ENABLE,       ///< 系统输出启用
    EVENT_SYSTEM_DISABLE,      ///< 系统输出禁用
} PowerEvent_Type_t;

/**
 * @brief 事件触发模式
 * 
 */
typedef enum
{
    event_trigger_immediately = 0,
    event_trigger_in_loop,
} EventTriggerMode_t;

/**
 * @struct PowerEvent_t
 * @brief 电源事件数据包
 */
typedef struct {
    PowerEvent_Type_t type;    ///< 事件类型
    u32 timestamp;             ///< 事件时间戳(ms)
} PowerEvent_t;

/// 事件处理器函数指针类型
typedef void (*EventHandler_t)(PowerEvent_t *event);

/**
 * @brief 初始化事件总线
 *
 * Usage: Event_Bus_Init(); // 在 main() 初始化中调用
 */
void Event_Bus_Init(void);

/**
 * @brief 订阅事件处理器
 *
 * @param type 要订阅的事件类型
 * @param handler 事件处理回调函数
 *
 * Usage: Event_Bus_Subscribe(EVENT_PROTECTION_OVP, LED_OVP_Handler);
 */
void Event_Bus_Subscribe(PowerEvent_Type_t type, EventHandler_t handler);

/**
 * @brief 发布事件
 *
 * event_trigger_immediately: 立即同步调用所有订阅者（主循环中模式切换用）
 * event_trigger_in_loop: 入队，等待 Event_Bus_Flush() 消费（ISR 中保护事件用）
 *
 * @param event 指向事件数据的指针
 * @param mode 事件触发模式
 *
 * Usage:
 *   PowerEvent_t ev = {EVENT_MODE_CC_ACTIVE};
 *   Event_Bus_Publish(&ev, event_trigger_immediately);
 */
void Event_Bus_Publish(PowerEvent_t *event, EventTriggerMode_t mode);

/**
 * @brief 消费队列中的延迟事件（在主循环中周期性调用）
 *
 * 将所有 event_trigger_in_loop 入队的事件一次性分发到订阅者。
 * 调用位置: main while(1) 循环中，每次迭代调用一次。
 *
 * Usage: Event_Bus_Flush();
 */
void Event_Bus_Flush(void);

#endif // __EVENT_BUS_H__
