/**
 * @file Digital_Power_Core.c
 * @brief 电源系统业务逻辑核心实现 - 保护检查与状态转换
 */

#include "debug.h"
#include "Digital_Power_Core.h"
#include "Digital_Power.h"
#include "ADC_k.h"
#include "PID.h"

// #define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

#define SCP_VOLTAGE_MARGIN 0.3f
#define SCP_CURRENT_MARGIN 0.3f
#define OVP_MARGIN 0.1f
#define OCP_MARGIN 0.1f
#define OVT_THRESHOLD 60.0f


extern Digital_Power_Dev dp;

static u8 cc_flag = 1; // CC 模式转换标志

/**
 * @brief 初始化业务逻辑核心
 */
void Digital_Power_Core_Init(void)
{
    cc_flag = 1;
}

/**
 * @brief 保护检查 - 在 ADC 中断中调用，发布保护事件
 *
 * 检查顺序 (优先级递减):
 *   1. SCP (短路) - 电流过高 + 电压坍塌
 *   2. OCP (过流) - 电流超过阈值
 *   3. OVP (过压) - 电压超过阈值
 *   4. OVT (过温) - 温度超过阈值
 *
 * @return 1 = 已触发保护, 0 = 未触发
 */
u8 Protection_Check(void)
{
    if (!dp.System_Enable_Flag)
    {
        return 0;
    }

    PowerEvent_t ev = {EVENT_NONE};
    float vset_norm = dp.Vset / 100.0f;
    float iset_norm = dp.Iset / 100.0f;

    // SCP: 短路保护 - 电流过高 AND 电压坍塌
    if (ADC_Value.Iout > iset_norm * (1 + SCP_CURRENT_MARGIN) &&
        ADC_Value.Vout < vset_norm * (1 - SCP_VOLTAGE_MARGIN))
    {
        xprintf("protection: SCP (short circuit)\r\n");
        LM25118_DISABLE;
        dp.System_Enable_Flag = 0;
        dp.sys_state = SCP;
        ev.type = EVENT_PROTECTION_SCP;
        Event_Bus_Publish(&ev,event_trigger_in_loop);
        return 1;
    }

    // OCP: 过流保护
    if (ADC_Value.Iout > iset_norm * (1 + OCP_MARGIN))
    {
        xprintf("protection: OCP (over current)\r\n");
        LM25118_DISABLE;
        dp.System_Enable_Flag = 0;
        dp.sys_state = OCP;
        ev.type = EVENT_PROTECTION_OCP;
        Event_Bus_Publish(&ev,event_trigger_in_loop);
        return 1;
    }

    // OVP: 过压保护
    if (ADC_Value.Vout > vset_norm * (1 + OVP_MARGIN))
    {
        xprintf("protection: OVP (over voltage)\r\n");
        LM25118_DISABLE;
        dp.System_Enable_Flag = 0;
        dp.sys_state = OVP;
        ev.type = EVENT_PROTECTION_OVP;
        Event_Bus_Publish(&ev,event_trigger_in_loop);
        return 1;
    }

    // OVT: 过温保护
    if (ADC_Value.Inductance_Temperature > OVT_THRESHOLD)
    {
        xprintf("protection: OVT (over temperature)\r\n");
        LM25118_DISABLE;
        dp.System_Enable_Flag = 0;
        dp.sys_state = OVT;
        ev.type = EVENT_PROTECTION_OVT;
        Event_Bus_Publish(&ev,event_trigger_in_loop);
        return 1;
    }

    return 0;
}

/**
 * @brief 状态转换 - 在主循环中调用，发返回事件(不直接调用 UI/LED/Buzzer)
 *
 * 返回的事件由 main 循环发布到事件总线，由反应器处理
 *
 * @return 发生的事件 (EVENT_NONE 表示无事件)
 */
PowerEvent_t Digital_Power_State_Update(void)
{
    PowerEvent_t ev = {EVENT_NONE};

    // CV → CC 转换: 当启用且电流环输出为正(需要限流)
    if (dp.System_Enable_Flag && PID_Current.output && cc_flag)
    {
        cc_flag = 0;
        dp.sys_state = CC_Switch;
        ev.type = EVENT_MODE_CC_ACTIVE;
        return ev;
    }

    // CC → CV 转换: 当禁用且电流环已复位
    if (!dp.System_Enable_Flag && !cc_flag && !PID_Current.output)
    {
        dp.sys_state = CV_Switch;
        cc_flag = 1;
        return ev; // 无事件
    }

    // 纯状态切换逻辑 (Switch → Steady)
    switch (dp.sys_state)
    {
    case IDLE_Switch:
        dp.sys_state = IDLE;
        ev.type = EVENT_MODE_IDLE;
        break;

    case CC_Switch:
        dp.sys_state = CC;
        // 事件已在转换时发送
        break;

    case CV_Switch:
        dp.sys_state = CV;
        ev.type = EVENT_MODE_CV_ACTIVE;
        break;

    // 保护状态由 Protection_Check() 处理
    case OVP:
    case OCP:
    case SCP:
    case OVT:
        dp.sys_state = FAULT_Lock;
        break;

    default:
        break;
    }

    return ev;
}

/*
 * Usage Example:
 *
 * void ADC_DMA_TC_Callback(void) {
 *     BSP_ADC_Loop();
 *
 *     if (!Protection_Check()) {  // 未触发保护
 *         PID_Incremental_Calc(&PID_Voltage, ...);
 *         BSP_PWM_Set_CCR(PID_Voltage.output);
 *     }
 * }
 *
 * // In main loop:
 * PowerEvent_t ev = Digital_Power_State_Update();
 * if (ev.type != EVENT_NONE) {
 *     Event_Bus_Publish(&ev, event_trigger_in_loop);
 * }
 */
