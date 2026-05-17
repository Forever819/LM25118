/**
 * @file Digital_Power_Core.h
 * @brief 电源系统业务逻辑核心 (状态转换、保护检查、事件发射)
 */

#ifndef __DIGITAL_POWER_CORE_H__
#define __DIGITAL_POWER_CORE_H__

#include <ch32v00X.h>
#include "Event_Bus.h"


/**
 * @brief 初始化业务逻辑核心
 *
 * 在 main() 中，Digital_Power_Init() 后调用
 *
 * Usage: Digital_Power_Core_Init();
 */
void Digital_Power_Core_Init(void);

/**
 * @brief 在 ADC DMA 中断回调中执行 - 检查所有保护条件
 *
 * 调用位置: ADC_DMA_TC_Callback() 的开头
 * 返回值: 0 = 未触发保护(继续 PID 控制), 非 0 = 已触发保护(阻止 PID 更新)
 *
 * Usage:
 *   void ADC_DMA_TC_Callback(void) {
 *       BSP_ADC_Loop();
 *       if (!Protection_Check()) {
 *           // 继续执行 PID 和 PWM 更新
 *           PID_Incremental_Calc(...);
 *           BSP_PWM_Set_CCR(...);
 *       }
 *   }
 */
u8 Protection_Check(void);

/**
 * @brief 在主循环中执行 - 处理状态转换逻辑，返回发生的事件
 *
 * 调用位置: main 循环中，Key_Event_Proc() 之后
 * 返回值: 发生的事件（可能为 EVENT_NONE）
 *
 * Usage:
 *   PowerEvent_t ev = Digital_Power_State_Event();
 *   if (ev.type != EVENT_NONE) {
 *     Event_Bus_Publish(&ev, event_trigger_in_loop);
 *   }
 */
PowerEvent_t Digital_Power_State_Event(void);

#endif // __DIGITAL_POWER_CORE_H__
