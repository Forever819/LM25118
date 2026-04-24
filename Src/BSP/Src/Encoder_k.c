#include "debug.h"
#include "Encoder_k.h"

// Ĭ�Ϲ��ص���ʱ������detachʱʹ�ã�
static s32 temp_unpressed = 0;
static s32 temp_pressed = 0;

// ȫ�ֱ���������,ָ��encoder1
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

    // Ĭ�Ϲ�����ʱ����
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

    // PC0��������A�ࣨ��ת����EXTI��
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    // PD0��������B�ࣨ�жϷ���
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    // PD4���������������룬�͵�ƽ=���£�
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    // EXTI Line0 �� PC0����תA�ࣩ
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
 * SysTick ÿ 1ms ���� ���� ��������״̬��
 *
 * ״̬ת��ͼ��
 *
 *  IDLE ����[����]����? PRESSING ����[��ת]������������������������������������? ROTATED
 *                      ��                                    ��
 *                   [��ʱ]                              [�ͷ�]��IDLE
 *                      ��
 *                   LONG_PRESS ����[�ͷ�]����? IDLE������LONG�¼���
 *                      ��
 *                   [�������£���ѡ��������]
 *
 *  PRESSING ����[�ͷ�������ת]����? IDLE������CLICK�¼���
 *
 * ========================================================= */
void BSP_Encoder_Tick (Encoder_t *enc) {
    // ��ȡ�������ţ��͵�ƽ=���£�
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
            // �ͷţ���û��ת�� �� ����
            if (!enc->rotated_while_pressed) {
                enc->key_event = KEY_EVENT_CLICK;
            }
            enc->key_state = KEY_STATE_IDLE;
        } else {
            enc->press_tick++;
            if (enc->press_tick >= enc->long_press_ms) {
                enc->key_state = KEY_STATE_LONG_PRESS;
                enc->key_event = KEY_EVENT_LONG_PRESS;  // �����״δ���
            }
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (!pressed) {
            enc->key_state = KEY_STATE_IDLE;
        }
        // ���賤���������������ڴ�ÿ��N ms��дһ�� KEY_EVENT_LONG_PRESS
        break;

    case KEY_STATE_ROTATED:
        // �����ڼ���ת�������ͷź�� IDLE�������������¼�
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
            // �ͷš� ����
            key->key_event = KEY_EVENT_CLICK;
            key->key_state = KEY_STATE_IDLE;
        } else {
            key->press_tick++;
            if (key->press_tick >= key->long_press_ms) {
                key->key_state = KEY_STATE_LONG_PRESS;
                key->key_event = KEY_EVENT_LONG_PRESS;  // �����״δ���
            }
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (!pressed) {
            key->key_state = KEY_STATE_IDLE;
        }
        // ���賤���������������ڴ�ÿ��N ms��дһ�� KEY_EVENT_LONG_PRESS
        break;

    default:
        key->key_state = KEY_STATE_IDLE;
        break;
    }
}

/* =========================================================
 * EXTI �жϣ�������ת
 * ���ж��е��� ���Ƽ���һ�жϺ�����xx_it.c�ļ��У�
 * ========================================================= */
void EXTI7_0_IRQHandler_Callback (void) {
    if (EXTI_GetITStatus (EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line0);

        if (g_enc == NULL)
            return;

        u8 key_down = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_4) == Bit_RESET);
        u8 dir_cw = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_0) == Bit_RESET);  // B���=˳ʱ��

        if (key_down) {
            // ---- ������ת�����ȼ��ߣ�----
            // ��ǣ����Ʊ��ΰ��µ� CLICK / LONG_PRESS �¼�
            g_enc->rotated_while_pressed = 1;
            g_enc->key_state = KEY_STATE_ROTATED;

            if (dir_cw)
                *g_enc->pressed.cnt += g_enc->pressed.step_val;
            else
                *g_enc->pressed.cnt -= g_enc->pressed.step_val;

            clamp_cnt (&g_enc->pressed);

        } else {
            // ---- δ������ת ----
            if (dir_cw)
                *g_enc->unpressed.cnt += g_enc->unpressed.step_val;
            else
                *g_enc->unpressed.cnt -= g_enc->unpressed.step_val;

            clamp_cnt (&g_enc->unpressed);
        }
    }
}

/* =========================================================
 * ��ѭ����ѯ��ȡ���¼���ȡ�����㣩
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
 * CNT �����ӿ�
 * ========================================================= */
s32 BSP_Encoder_Get_Cnt (Encoder_CNT_t *e) {
    return *e->cnt;
}

void BSP_Encoder_CNT_Attach (Encoder_CNT_t *e, s32 *value_addr) {
    e->cnt = value_addr;
}

void BSP_Encoder_CNT_Detach (Encoder_CNT_t *e) {
    e->cnt = &temp_unpressed;
}

void BSP_Encoder_Set_Step (Encoder_CNT_t *e, s32 step) {
    e->step_val = step;
}

void BSP_Encoder_Set_Range (Encoder_CNT_t *e, s32 min, s32 max) {
    e->min = min;
    e->max = max;
}

/*
main.c ʹ��ʾ��

#include "Encoder_k.h"

Encoder_t encoder1;
s32 volume   = 50;   // δ������ת����
s32 contrast = 128;  // ������ת����

int main(void) {
    SystemCoreClockUpdate();
    // ���� SysTick ������Ϊ 1ms

    BSP_Encoder_Init(&encoder1);

    // ����ʵ�ʱ���
    BSP_Encoder_CNT_Attach(&encoder1.unpressed, &volume);
    BSP_Encoder_Set_Range  (&encoder1.unpressed, 0, 100);
    BSP_Encoder_Set_Step   (&encoder1.unpressed, 1);

    BSP_Encoder_CNT_Attach(&encoder1.pressed, &contrast);
    BSP_Encoder_Set_Range  (&encoder1.pressed, 0, 255);
    BSP_Encoder_Set_Step   (&encoder1.pressed, 2);

    // ����ʱ���Ϊ 800ms
    encoder1.long_press_ms = 800;

    while (1) {
        Key_Event_t ev = BSP_Encoder_Get_Event(&encoder1);
        switch (ev) {
        case KEY_EVENT_CLICK:
            // �����¼�����
            break;
        case KEY_EVENT_LONG_PRESS:
            // �����¼�����
            break;
        default:
            break;
        }
        // ʹ�� volume / contrast ˢ�� UI ...
    }
}

// SysTick_Handler��ÿ 1ms��
void SysTick_Handler(void) {
    BSP_Encoder_Tick(&encoder1);
}
```

---

## ���Ҫ��˵��

**״̬�������߼���**
```
����
 ��
 ��
PRESSING ������ת��������? ROTATED �����ͷũ���? IDLE  (�ް����¼�)
 ��
 ���� �ͷ�(����ת) ����? IDLE  ���� CLICK
 ��
 ���� ��ʱ(500ms) ����? LONG_PRESS �����ͷũ���? IDLE  ���� LONG_PRESS

*/


/*

���ڶ������֧��

g_enc ��ָ�� encoder1 ��ָ�롣
Encoder_t encoder1;        // ʵ�ʵĶ����� main.c �ж���
static Encoder_t *g_enc;   // ָ�룬�� Encoder_k.c �ڲ���

// Init ʱ����������
void BSP_Encoder_Init(Encoder_t *enc) {
    g_enc = enc;   // g_enc ָ�� encoder1
}

// ���ã�
BSP_Encoder_Init(&encoder1);  // �˺� g_enc == &encoder1
ΪʲôҪ������ƣ�
��Ϊ EXTI7_0_IRQHandler ��Ӳ���жϣ�����ǩ���̶����޷����Σ���������Ҫ���� encoder1 �����ݡ������� g_enc ���ģ���ڲ���ȫ��ָ����Ϊ"����"��

void EXTI7_0_IRQHandler(void) {
    // ����д IRQHandler(Encoder_t *enc)
    // ֻ��ͨ�� g_enc ��������
    *g_enc->pressed.cnt += ...;
}

�����Ҫ��չ�����Ըĳ����飺
cstatic Encoder_t *g_enc[2] = {NULL, NULL};

// ������1 �� EXTI Line0
// ������2 �� EXTI Line1

void EXTI7_0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0)) {}
    if (EXTI_GetITStatus (EXTI_Line1)) {}
}
*/
