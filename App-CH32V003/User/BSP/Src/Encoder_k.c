#include "debug.h"
#include "Encoder_k.h"
#define LIMIT 500
static volatile int16_t encoder_cnt = 250;

void BSP_Encoder_Init() {
#if (ENCODER_MODE == ENCODER_EXTI_MODE)

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    GPIO_EXTILineConfig (GPIO_PortSourceGPIOC, GPIO_PinSource0);  // gpio组通过afio选择line0-7所以要使能afio
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

#elif (ENCODER_MODE == ENCODER_TIMER_MODE)

#endif
}

#if (ENCODER_MODE == ENCODER_EXTI_MODE)
void EXTI7_0_IRQHandler (void) __attribute__ ((interrupt));

void EXTI7_0_IRQHandler (void) {
    if (EXTI_GetITStatus (EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line0);
        if (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_0) == Bit_SET)
            encoder_cnt += 1;
        else
            encoder_cnt -= 1;

        if (encoder_cnt > LIMIT)
            encoder_cnt = LIMIT;
        else if (encoder_cnt < 0)
            encoder_cnt = 0;
    }
}

int16_t BSP_Encoder_Get_Cnt (void) {
    return encoder_cnt % LIMIT;
}

#elif (ENCODER_MODE == ENCODER_TIMER_MODE)

#endif