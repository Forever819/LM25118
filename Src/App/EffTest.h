/**
 * @file EffTest.h
 * @brief 效率测试模块 —— 独立的状态机实现
 *
 * 通过 EFF_TEST_ENABLE 宏控制编译开关。
 * 关闭时所有 API 展开为空桩，零开销。
 */

#ifndef __EFF_TEST_H__
#define __EFF_TEST_H__

#include <stdint.h>
#include "ch32v00x.h"

/* ========== 编译开关 ========== */
#define EFF_TEST_ENABLE   1

/* ========== 配置参数 ========== */

/** @brief 电压起始值 (V) */
#define EFF_VSTART         2.0f
/** @brief 电压终止值 (V) */
#define EFF_VEND          45.0f
/** @brief 步进 (V) */
#define EFF_VSTEP          0.5f
/** @brief 每步静置时间 (主循环周期数, 每周期 ~10ms) */
#define EFF_SETTLE_TICKS   50
/** @brief 每步超时 (主循环周期数) */
#define EFF_TIMEOUT_TICKS  500
/** @brief FINISHED 停留时间后自动返回 (周期数) */
#define EFF_FINISH_TICKS   200

/** @brief 总步数 = (45-2)/0.5 + 1 = 87 */
#define EFF_TOTAL_STEPS    87

#if EFF_TEST_ENABLE

/* ========== 类型定义 ========== */

/** @brief 效率测试状态 */
typedef enum
{
    EFF_TEST_IDLE = 0,      /**< 空闲，未启动 */
    EFF_TEST_WARMING,       /**< 等待用户确认 (外接电子负载) */
    EFF_TEST_RUNNING,       /**< 步进测试中 */
    EFF_TEST_FINISHED,      /**< 测试完成 */
} EffTest_State_t;

/** @brief 效率测试控制器 */
typedef struct
{
    EffTest_State_t state;  /**< 当前状态 */
    float current_vout;     /**< 当前步进电压 (V) */
    float efficiency;       /**< 最新记录效率 (%) */
    uint16_t step_count;    /**< 已完成步数 (从0开始) */
    uint16_t step_settle;   /**< 静置计数 (等待电源稳定) */
    uint16_t step_timeout;  /**< 当前步进超时计数 (每10ms+1) */
} EffTest_t;

/* ========== API ========== */

/**
 * @brief 效率测试状态机 — 在主循环中每 10ms 调用一次
 *
 * 状态流转:
 *   WARMING → (EffTest_OnKeyClick) → RUNNING
 *     → 对每个 Vout (2.0V~45.0V, 0.5V) 等待稳定后记录
 *     → FINISHED → (超时自动) → IDLE
 */
void EffTest_Proc(void);

/**
 * @brief 单击事件处理
 *   - WARMING: 启动测试进入 RUNNING
 *   - RUNNING: 中断测试返回 IDLE
 */
void EffTest_OnKeyClick(void);

/**
 * @brief 长按事件处理 — WARMING 状态时退出
 */
void EffTest_OnKeyLongPress(void);

/**
 * @brief 从设置菜单启动测试
 */
void EffTest_Start(void);

/**
 * @brief 测试中是否需要强制风扇开启
 * @return 1 = 强制开启, 0 = 由温控决定
 */
uint8_t EffTest_IsFanForced(void);

/**
 * @brief 渲染效率测试页面
 */
void EffTest_Draw(void);

/** @brief 全局效率测试控制器 */
extern EffTest_t g_eff_test;

#else /* !EFF_TEST_ENABLE */

/* ========== 空桩 ========== */
#define EffTest_Proc()            ((void)0)
#define EffTest_OnKeyClick()      ((void)0)
#define EffTest_OnKeyLongPress()  ((void)0)
#define EffTest_Start()           ((void)0)
#define EffTest_IsFanForced()     0
#define EffTest_Draw()            ((void)0)

#endif /* EFF_TEST_ENABLE */

#endif /* __EFF_TEST_H__ */
