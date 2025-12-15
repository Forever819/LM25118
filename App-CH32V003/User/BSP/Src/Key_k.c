#include "debug.h"
#include "Key_k.h"

Key_t Key_Encoder = {
    .GPIOX = GPIOD,
    .GPIO_Pin = GPIO_Pin_4,
};

Key_t Key_Enable = {
    .GPIOX = GPIOD,
    .GPIO_Pin = GPIO_Pin_1,
};

static void Key_Init (Key_t *Key) {
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Pin = Key->GPIO_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (Key->GPIOX, &GPIO_InitStructure);

    // 初始化状态
    Key->cnt = 0;
    Key->state = Key_Idle;
    Key->pressed_flag = 0;
}

static void Key_Scan (Key_t *Key) {
    uint8_t key_val = GPIO_ReadInputDataBit (Key->GPIOX, Key->GPIO_Pin);

    switch (Key->state) {
    case Key_Idle:
        if (key_val == Bit_RESET)  // 按下
        {
            Key->cnt++;
            if (Key->cnt >= KEY_DEBOUNCE_MS) {
                Key->cnt = 0;
                Key->state = Key_Pressed;
            }
        } else {
            Key->cnt = 0;
        }
        break;

    case Key_Pressed:
        if (key_val == Bit_RESET) {
            Key->cnt++;

            if (Key->cnt >= KEY_LONG_MS) {
                Key->cnt = 0;
                Key->state = Key_LongPressed;
            }
        } else  // 松开
        {
            Key->cnt = 0;
            Key->state = Key_Released;
        }
        break;

    case Key_Released:
        if (key_val == Bit_RESET)  // 第二次按下 → 双击
        {
            Key->cnt++;
            if (Key->cnt >= KEY_DEBOUNCE_MS) {
                Key->cnt = 0;
                Key->state = Key_DoublePressed;
            }
        } else  // 仍然松开 → 等待是否超时
        {
            Key->cnt++;
            if (Key->cnt >= KEY_DOUBLE_MS) {
                Key->cnt = 0;
                Key->pressed_flag = 1;
                Key->state = Key_Idle;
            }
        }
        break;

    case Key_DoublePressed:
        if (key_val == Bit_SET)  // 第二次松开
        {
            Key->cnt++;
            if (Key->cnt >= KEY_DEBOUNCE_MS) {
                Key->cnt = 0;
                Key->double_pressed_flag = 1;
                Key->state = Key_Idle;
            }
        } else {
            Key->cnt = 0;
        }
        break;

    case Key_LongPressed:
        if (key_val == Bit_SET)  // 长按后松开
        {
            Key->cnt++;
            if (Key->cnt >= KEY_DEBOUNCE_MS) {
                Key->cnt = 0;
                Key->state = Key_Idle;
            }
        } else {
            Key->cnt = 0;
            Key->long_pressed_flag = 1;
        }
        break;
    }
}

void BSP_Key_Init (void) {
    Key_Init (&Key_Encoder);
    Key_Init (&Key_Enable);
}

void BSP_Key_Scan (void) {
    Key_Scan (&Key_Encoder);
    Key_Scan (&Key_Enable);
}
