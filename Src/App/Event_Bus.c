/**
 * @file Event_Bus.c
 * @brief 事件总线实现 - 解耦业务逻辑与 UI/LED/Buzzer 响应层
 */

#include "Event_Bus.h"

#define MAX_HANDLERS_PER_TYPE 10
#define EVENT_QUEUE_SIZE 8

/// 事件订阅表项
typedef struct
{
    EventHandler_t handlers[MAX_HANDLERS_PER_TYPE]; ///< 该事件类型的处理器数组
    u8 count;                                       ///< 已注册的处理器数量
} EventSubscription_t;

/// 全局事件订阅表 (按事件类型索引)
static EventSubscription_t subscription_table[EVENT_SYSTEM_DISABLE + 1] = {0};

/// 延迟事件环形队列
static PowerEvent_t event_queue[EVENT_QUEUE_SIZE];
static volatile u8 queue_head = 0;
static volatile u8 queue_tail = 0;

/**
 * @brief 初始化事件总线
 */
void Event_Bus_Init(void)
{
    for (u8 i = 0; i < (EVENT_SYSTEM_DISABLE + 1); i++)
    {
        subscription_table[i].count = 0;
        for (u8 j = 0; j < MAX_HANDLERS_PER_TYPE; j++)
        {
            subscription_table[i].handlers[j] = NULL;
        }
    }
    queue_head = 0;
    queue_tail = 0;
}

/**
 * @brief 订阅事件处理器
 */
void Event_Bus_Subscribe(PowerEvent_Type_t type, EventHandler_t handler)
{
    if (type >= EVENT_SYSTEM_DISABLE + 1 || handler == NULL)
    {
        return;
    }

    EventSubscription_t *sub = &subscription_table[type];
    if (sub->count >= MAX_HANDLERS_PER_TYPE)
    {
        printf("[EventBus] Warning: max handlers reached for event %d\r\n", type);
        return;
    }

    sub->handlers[sub->count] = handler;
    sub->count++;
}

/**
 * @brief 发布事件
 *
 * - event_trigger_immediately: 同步调用所有订阅者
 * - event_trigger_in_loop: 入队，等待 Event_Bus_Flush() 消费
 */
void Event_Bus_Publish(PowerEvent_t *event, EventTriggerMode_t mode)
{
    if (event == NULL || event->type >= EVENT_SYSTEM_DISABLE + 1)
    {
        return;
    }

    if (mode == event_trigger_immediately)
    {
        EventSubscription_t *sub = &subscription_table[event->type];
        for (u8 i = 0; i < sub->count; i++)
        {
            if (sub->handlers[i] != NULL)
            {
                sub->handlers[i](event);
            }
        }
    }
    else
    {
        // 入队延迟事件
        u8 next = (queue_head + 1) % EVENT_QUEUE_SIZE;
        if (next != queue_tail)
        {
            event_queue[queue_head] = *event;
            queue_head = next;
        }
    }
}

/**
 * @brief 消费队列中的所有延迟事件
 */
void Event_Bus_Flush(void)
{
    while (queue_tail != queue_head)
    {
        PowerEvent_t *ev = &event_queue[queue_tail];
        EventSubscription_t *sub = &subscription_table[ev->type];
        for (u8 i = 0; i < sub->count; i++)
        {
            if (sub->handlers[i] != NULL)
            {
                sub->handlers[i](ev);
            }
        }
        queue_tail = (queue_tail + 1) % EVENT_QUEUE_SIZE;
    }
}

/*
 * Usage Example:
 *
 * // 1. 定义处理器
 * static void MyLED_Handler(PowerEvent_t *ev) {
 *     switch (ev->type) {
 *         case EVENT_PROTECTION_OVP:
 *             BSP_WS2812_Set_Color(0xFF0000);
 *             break;
 *     }
 * }
 *
 * // 2. 初始化并订阅
 * void LED_Responder_Init(void) {
 *     Event_Bus_Subscribe(EVENT_PROTECTION_OVP, MyLED_Handler);
 * }
 *
 * // 3. 主循环中使用
 * int main(void) {
 *     Event_Bus_Init();
 *     LED_Responder_Init();
 *     while (1) {
 *         Event_Bus_Flush();  // 消费 ISR 中的保护事件
 *         // 模式切换用立即触发
 *         PowerEvent_t ev = {EVENT_MODE_CV_ACTIVE};
 *         Event_Bus_Publish(&ev, event_trigger_immediately);
 *     }
 * }
 */
