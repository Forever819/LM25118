/*coding:utf-8*/
#include "debug.h"
#include "Encoder.h"


static s32 temp_unpressed = 0;
static s32 temp_pressed = 0;


static Encoder_t *g_enc = NULL;
//
Encoder_t encoder1;
Key_t Key_Enable = {
    .GPIOX = GPIOD,
    .GPIO_Pin = GPIO_Pin_1,
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
    GPIO_InitStructure.GPIO_Pin = key->GPIO_Pin;
    GPIO_Init (key->GPIOX, &GPIO_InitStructure);

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
 *  DP_IDLE ����[����]����? PRESSING ����[��ת]������������������������������������? ROTATED
 *                      ��                                    ��
 *                   [��ʱ]                              [�ͷ�]��DP_IDLE
 *                      ��
 *                   LONG_PRESS ����[�ͷ�]����? DP_IDLE������LONG�¼���
 *                      ��
 *                   [�������£���ѡ��������]
 *
 *  PRESSING ����[�ͷ�������ת]����? DP_IDLE������CLICK�¼���
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
            
            if (!enc->rotated_while_pressed) {
                enc->key_event = KEY_EVENT_CLICK;
            }
            enc->key_state = KEY_STATE_IDLE;
        } else {
            enc->press_tick++;
            if (enc->press_tick >= enc->long_press_ms) {
                enc->key_state = KEY_STATE_LONG_PRESS;
                enc->key_event = KEY_EVENT_LONG_PRESS;  
            }
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (!pressed) {
            enc->key_state = KEY_STATE_IDLE;
        }
        
        break;

    case KEY_STATE_ROTATED:
        
        if (!pressed) {
            enc->key_state = KEY_STATE_IDLE;
        }
        break;

    default:
        enc->key_state = KEY_STATE_IDLE;
        break;
    }
}

void BSP_Key_Tick_GPIO (Key_t *key) {
    u8 pressed = (GPIO_ReadInputDataBit (key->GPIOX, key->GPIO_Pin) == key->key_valid_val);
    switch (key->key_state) {
    case KEY_STATE_IDLE:
        if (pressed) {
            key->key_state = KEY_STATE_PRESSING;
            key->press_tick = 0;
        }
        break;

    case KEY_STATE_PRESSING:
        if (!pressed) {

            key->key_event = KEY_EVENT_CLICK;
            key->key_state = KEY_STATE_IDLE;
        } else {
            key->press_tick++;
            if (key->press_tick >= key->long_press_ms) {
                key->key_state = KEY_STATE_LONG_PRESS;
                key->key_event = KEY_EVENT_LONG_PRESS;  
            }
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (!pressed) {
            key->key_state = KEY_STATE_IDLE;
        }
        
        break;

    default:
        key->key_state = KEY_STATE_IDLE;
        break;
    }
}


void EXTI7_0_IRQHandler_Callback (void) {
    if (EXTI_GetITStatus (EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line0);

        if (g_enc == NULL)
            return;

        u8 key_down = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_4) == Bit_RESET);
        u8 dir_cw = (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_0) == Bit_RESET); 

        if (key_down) {
            g_enc->rotated_while_pressed = 1;
            g_enc->key_state = KEY_STATE_ROTATED;

            if (dir_cw)
                *g_enc->pressed.cnt += g_enc->pressed.step_val;
            else
                *g_enc->pressed.cnt -= g_enc->pressed.step_val;

            clamp_cnt (&g_enc->pressed);

        } else {
           
            if (dir_cw)
                *g_enc->unpressed.cnt += g_enc->unpressed.step_val;
            else
                *g_enc->unpressed.cnt -= g_enc->unpressed.step_val;

            clamp_cnt (&g_enc->unpressed);
        }
    }
}

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
