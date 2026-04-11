#include "debug.h"
#include "Encoder_k.h"

// 默认挂载的临时变量（detach时使用）
static s32 temp_unpressed = 0;
static s32 temp_pressed = 0;

// 全局编码器对象,指向encoder1
static Encoder_t *g_enc = NULL;
//
Encoder_t encoder1;
Key_t Key_Enable = {
    .GPIOX = GPIOD,
    .GPIO_Pin = GPIO_Pin_7,
};

static void clamp_cnt (Encoder_CNT_t *e) {
    if (*e->cnt > e->max)
        *e->cnt = e->max;
    if (*e->cnt < e->min)
        *e->cnt = e->min;
}

void BSP_Key_Init (Key_t *key) {

    RCC_PB2PeriphClockCmd (RCC_PB2Periph_GPIOD, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    key->key_state = KEY_STATE_IDLE;
    key->key_event = KEY_EVENT_NONE;
    key->press_tick = 0;
    key->long_press_ms = 500;
    key->key_valid_val=Bit_RESET;
}

void BSP_Encoder_Init (Encoder_t *enc) {
    g_enc = enc;

    // 默认挂载临时变量
    enc->unpressed.cnt = &temp_unpressed;
    enc->unpressed.step_val = 1;
    enc->unpressed.max = 0x7FFFFFFF;
    enc->unpressed.min = 0x80000000;

    enc->pressed.cnt = &temp_pressed;
    enc->pressed.step_val = 1;
    enc->pressed.max = 0x7FFFFFFF;
    enc->pressed.min = 0x80000000;

    enc->key_state = KEY_STATE_IDLE;
    enc->key_event = KEY_EVENT_NONE;
    enc->press_tick = 0;
    enc->long_press_ms = 800;
    enc->rotated_while_pressed = 0;

    // ---------- GPIO + EXTI + NVIC ----------
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_PB2PeriphClockCmd (RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOC | RCC_PB2Periph_GPIOD, ENABLE);

    // PC0：编码器A相（旋转触发EXTI）
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    // PD0：编码器B相（判断方向）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    // PD4：按键（上拉输入，低电平=按下）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    // EXTI Line0 → PC0（旋转A相）
    GPIO_EXTILineConfig (GPIO_PortSourceGPIOC, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_Init (&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init (&NVIC_InitStructure);
}

/* =========================================================
 * SysTick 每 1ms 调用 —— 驱动按键状态机
 *
 * 状态转移图：
 *
 *  IDLE ──[按下]──? PRESSING ──[旋转]──────────────────? ROTATED
 *                      │                                    │
 *                   [超时]                              [释放]→IDLE
 *                      │
 *                   LONG_PRESS ──[释放]──? IDLE（触发LONG事件）
 *                      │
 *                   [持续按下，可选持续触发]
 *
 *  PRESSING ──[释放且无旋转]──? IDLE（触发CLICK事件）
 *
 * ========================================================= */
void BSP_Encoder_Tick (Encoder_t *enc) {
    // 读取按键引脚（低电平=按下）
    u8 pressed = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_4) == Bit_RESET);

    switch (enc->key_state) {

    case KEY_STATE_IDLE:
        if (pressed) {
            enc->key_state = KEY_STATE_PRESSING;
            enc->press_tick = 0;
            enc->rotated_while_pressed = 0;
        }
        break;

    case KEY_STATE_PRESSING:
        if (!pressed) {
            // 释放：若没旋转过 → 单击
            if (!enc->rotated_while_pressed) {
                enc->key_event = KEY_EVENT_CLICK;
            }
            enc->key_state = KEY_STATE_IDLE;
        } else {
            enc->press_tick++;
            if (enc->press_tick >= enc->long_press_ms) {
                enc->key_state = KEY_STATE_LONG_PRESS;
                enc->key_event = KEY_EVENT_LONG_PRESS;  // 长按首次触发
            }
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (!pressed) {
            enc->key_state = KEY_STATE_IDLE;
        }
        // 如需长按持续触发，可在此每隔N ms再写一次 KEY_EVENT_LONG_PRESS
        break;

    case KEY_STATE_ROTATED:
        // 按下期间旋转过，等释放后回 IDLE，不产生按键事件
        if (!pressed) {
            enc->key_state = KEY_STATE_IDLE;
        }
        break;

    default:
        enc->key_state = KEY_STATE_IDLE;
        break;
    }
}

void BSP_Key_Tick (Key_t *key) {
    // u8 pressed = (GPIO_ReadInputDataBit (key->GPIOX, key->GPIO_Pin) == key->key_valid_val);
    u8 pressed = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_7) == Bit_RESET);


    switch (key->key_state) {
    case KEY_STATE_IDLE:
        if (pressed) {
            key->key_state = KEY_STATE_PRESSING;
            key->press_tick = 0;
        }
        break;

    case KEY_STATE_PRESSING:
        if (!pressed) {
            // 释放→ 单击
            key->key_event = KEY_EVENT_CLICK;
            key->key_state = KEY_STATE_IDLE;
        } else {
            key->press_tick++;
            if (key->press_tick >= key->long_press_ms) {
                key->key_state = KEY_STATE_LONG_PRESS;
                key->key_event = KEY_EVENT_LONG_PRESS;  // 长按首次触发
            }
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (!pressed) {
            key->key_state = KEY_STATE_IDLE;
        }
        // 如需长按持续触发，可在此每隔N ms再写一次 KEY_EVENT_LONG_PRESS
        break;

    default:
        key->key_state = KEY_STATE_IDLE;
        break;
    }
}

/* =========================================================
 * EXTI 中断：处理旋转
 * 在中断中调用 （推荐归一中断函数在xx_it.c文件中）
 * ========================================================= */
void EXTI7_0_IRQHandler_Callback (void) {
    if (EXTI_GetITStatus (EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line0);

        if (g_enc == NULL)
            return;

        u8 key_down = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_4) == Bit_RESET);
        u8 dir_cw = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_0) == Bit_RESET);  // B相低=顺时针

        if (key_down) {
            // ---- 按下旋转（优先级高）----
            // 标记：抑制本次按下的 CLICK / LONG_PRESS 事件
            g_enc->rotated_while_pressed = 1;
            g_enc->key_state = KEY_STATE_ROTATED;

            if (dir_cw)
                *g_enc->pressed.cnt += g_enc->pressed.step_val;
            else
                *g_enc->pressed.cnt -= g_enc->pressed.step_val;

            clamp_cnt (&g_enc->pressed);

        } else {
            // ---- 未按下旋转 ----
            if (dir_cw)
                *g_enc->unpressed.cnt += g_enc->unpressed.step_val;
            else
                *g_enc->unpressed.cnt -= g_enc->unpressed.step_val;

            clamp_cnt (&g_enc->unpressed);
        }
    }
}

/* =========================================================
 * 主循环轮询：取走事件（取后清零）
 * ========================================================= */
Key_Event_t BSP_Encoder_Get_Event (Encoder_t *enc) {
    Key_Event_t ev = enc->key_event;
    enc->key_event = KEY_EVENT_NONE;
    return ev;
}

Key_Event_t BSP_Key_Get_Event (Key_t *key) {
    Key_Event_t ev = key->key_event;
    key->key_event = KEY_EVENT_NONE;
    return ev;
}

/* =========================================================
 * CNT 操作接口
 * ========================================================= */
s32 BSP_Encoder_Get_Cnt (Encoder_CNT_t *e) {
    return *e->cnt;
}

void BSP_Encoder_CNT_Attach (Encoder_CNT_t *e, s32 *value_addr) {
    e->cnt = value_addr;
}

void BSP_Encoder_CNT_Detach (Encoder_CNT_t *e) {
    // 根据是 unpressed 还是 pressed 选不同 temp
    e->cnt = (e == /* caller knows */ NULL) ? &temp_unpressed : &temp_unpressed;
    // 实际使用时直接传对应 temp 地址即可，见下方宏
}

void BSP_Encoder_Set_Step (Encoder_CNT_t *e, s32 step) {
    e->step_val = step;
}

void BSP_Encoder_Set_Range (Encoder_CNT_t *e, s32 min, s32 max) {
    e->min = min;
    e->max = max;
}

/*
main.c 使用示例

#include "Encoder_k.h"

Encoder_t encoder1;
s32 volume   = 50;   // 未按下旋转控制
s32 contrast = 128;  // 按下旋转控制

int main(void) {
    SystemCoreClockUpdate();
    // 假设 SysTick 已配置为 1ms

    BSP_Encoder_Init(&encoder1);

    // 挂载实际变量
    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &volume);
    BSP_Encoder_Set_Range  (&encoder1.unpressed, 0, 100);
    BSP_Encoder_Set_Step   (&encoder1.unpressed, 1);

    BSP_Encoder_CNT_Attach(&encoder1.pressed, &contrast);
    BSP_Encoder_Set_Range  (&encoder1.pressed, 0, 255);
    BSP_Encoder_Set_Step   (&encoder1.pressed, 2);

    // 长按时间改为 800ms
    encoder1.long_press_ms = 800;

    while (1) {
        Key_Event_t ev = BSP_Encoder_Get_Event(&encoder1);
        switch (ev) {
        case KEY_EVENT_CLICK:
            // 单击事件处理
            break;
        case KEY_EVENT_LONG_PRESS:
            // 长按事件处理
            break;
        default:
            break;
        }
        // 使用 volume / contrast 刷新 UI ...
    }
}

// SysTick_Handler（每 1ms）
void SysTick_Handler(void) {
    BSP_Encoder_Tick(&encoder1);
}
```

---

## 设计要点说明

**状态机核心逻辑：**
```
按下
 │
 ▼
PRESSING ──旋转发生──? ROTATED ──释放──? IDLE  (无按键事件)
 │
 ├─ 释放(无旋转) ──? IDLE  触发 CLICK
 │
 └─ 超时(500ms) ──? LONG_PRESS ──释放──? IDLE  触发 LONG_PRESS

*/


/*

关于多编码器支持

g_enc 是指向 encoder1 的指针。
Encoder_t encoder1;        // 实际的对象，在 main.c 中定义
static Encoder_t *g_enc;   // 指针，在 Encoder_k.c 内部用

// Init 时建立关联：
void BSP_Encoder_Init(Encoder_t *enc) {
    g_enc = enc;   // g_enc 指向 encoder1
}

// 调用：
BSP_Encoder_Init(&encoder1);  // 此后 g_enc == &encoder1
为什么要这样设计？
因为 EXTI7_0_IRQHandler 是硬件中断，函数签名固定，无法传参，但它又需要访问 encoder1 的数据。所以用 g_enc 这个模块内部的全局指针作为"桥梁"。

void EXTI7_0_IRQHandler(void) {
    // 不能写 IRQHandler(Encoder_t *enc)
    // 只能通过 g_enc 访问数据
    *g_enc->pressed.cnt += ...;
}

如果需要扩展，可以改成数组：
cstatic Encoder_t *g_enc[2] = {NULL, NULL};

// 编码器1 → EXTI Line0
// 编码器2 → EXTI Line1

void EXTI7_0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0)) {}
    if (EXTI_GetITStatus (EXTI_Line1)) {}
}
*/
