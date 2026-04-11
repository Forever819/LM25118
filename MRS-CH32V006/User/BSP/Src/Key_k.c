// #include "debug.h"
// #include "Key_k.h"
// #include "flash_param.h"

// xKey_t Key_Enable = {
//     .GPIOX = GPIOD,
//     .GPIO_Pin = GPIO_Pin_7,
// };

// static void Key_Init (xKey_t *Key) {
//     RCC_PB2PeriphClockCmd (RCC_PB2Periph_GPIOD, ENABLE);

//     GPIO_InitTypeDef GPIO_InitStructure = {0};
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
//     GPIO_InitStructure.GPIO_Pin = Key->GPIO_Pin;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
//     GPIO_Init (Key->GPIOX, &GPIO_InitStructure);

//     // 初始化状态
//     Key->cnt = 0;
//     Key->state = xKey_Idle;
// }

// static void Key_Scan (xKey_t *Key) {
//     uint8_t key_val = GPIO_ReadInputDataBit (Key->GPIOX, Key->GPIO_Pin);

//     switch (Key->state) {
//     case xKey_Idle:
//         if (key_val == Bit_RESET)  // 按下
//         {
//             Key->cnt++;
//             if (Key->cnt >= KEY_DEBOUNCE_MS) {
//                 Key->cnt = 0;
//                 Key->state = xKey_Pressed;
//             }
//         } else {
//             Key->cnt = 0;
//         }
//         break;

//     case xKey_Pressed:
//         if (key_val == Bit_RESET) {
//             Key->cnt++;

//             if (Key->cnt >= KEY_LONG_MS) {
//                 Key->cnt = 0;
//                 Key->state = xKey_LongPressed;
//             }
//         } else  // 松开
//         {
//             Key->cnt = 0;
//             Key->state = xKey_Released;
//         }
//         break;

//     case xKey_Released:
//         if (key_val == Bit_RESET)  // 第二次按下 → 双击
//         {
//             Key->cnt++;
//             if (Key->cnt >= KEY_DEBOUNCE_MS) {
//                 Key->cnt = 0;
//                 Key->state = xKey_DoublePressed;
//             }
//         } else  // 仍然松开 → 等待是否超时
//         {
//             Key->cnt++;
//             if (Key->cnt >= KEY_DOUBLE_MS) {
//                 Key->cnt = 0;
//                 Key->event = xKEY_EVENT_CLICK;
//                 Key->state = xKey_Idle;
//             }
//         }
//         break;

//     case xKey_DoublePressed:
//         if (key_val == Bit_SET)  // 第二次松开
//         {
//             Key->cnt++;
//             if (Key->cnt >= KEY_DEBOUNCE_MS) {
//                 Key->cnt = 0;
//                 Key->event = xKEY_EVENT_DOUBLE_PRESS;
//                 Key->state = xKey_Idle;
//             }
//         } else {
//             Key->cnt = 0;
//         }
//         break;

//     case xKey_LongPressed:
//         if (key_val == Bit_SET)  // 长按后松开
//         {
//             Key->cnt++;
//             if (Key->cnt >= KEY_DEBOUNCE_MS) {
//                 Key->cnt = 0;
//                 Key->state = xKey_Idle;
//             }
//         } else {
//             Key->cnt = 0;
//             Key->event = xKEY_EVENT_LONG_PRESS;
//         }
//         break;
//     }
// }

// xKey_Event_t BSP_Key_Get_Event (xKey_t *key) {
//     xKey_Event_t ev = key->event;
//     key->event = xKEY_EVENT_NONE;
//     return ev;
// }

// void BSP_Key_Init (void) {
//     Key_Init (&Key_Enable);

//     if (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_4) == Bit_RESET) {
//         flash_data.cfg.oled_direction = (flash_data.cfg.oled_direction + 1) % 2;
//     }
// }

// void BSP_Key_Scan (void) {
//     Key_Scan (&Key_Enable);
// }
