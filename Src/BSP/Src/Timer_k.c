#include <ch32v00X.h>
#include "Timer_k.h"

void BSP_PWM_Init (void) {
    RCC_PB2PeriphClockCmd (RCC_PB2Periph_GPIOC, ENABLE);
    RCC_PB2PeriphClockCmd (RCC_PB2Periph_TIM1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    //LM25118 Enable pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits (GPIOC, GPIO_Pin_5);  // Inactive low

    TIM_TimeBaseStructInit (&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 5 - 1; //1Khz
    TIM_TimeBaseInitStructure.TIM_Period = MAX_PWM_PERIOD - 1;  // 5kHz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;
    TIM_TimeBaseInit (TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCStructInit (&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 250;
    TIM_OC3Init (TIM1, &TIM_OCInitStructure);

    TIM_OC3PreloadConfig (TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig (TIM1, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init (&NVIC_InitStructure);

    TIM_ITConfig (TIM1, TIM_IT_Update, ENABLE);
    TIM_SelectOutputTrigger (TIM1, TIM_TRGOSource_Update);
    TIM_CtrlPWMOutputs (TIM1, ENABLE);
    TIM_Cmd (TIM1, ENABLE);
}

void BSP_PWM_Set_CCR (uint16_t ccr) {
    TIM_SetCompare3 (TIM1, ccr);
}

__weak_symbol void BSP_TIM1_ISR_Callback (void) {
}
