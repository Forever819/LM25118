#include "debug.h"
#include "Timer_k.h"

volatile u8 buzzer_ms = 0;

void BSP_TIM1_Init (void) {
    RCC_PB2PeriphClockCmd (RCC_PB2Periph_GPIOC, ENABLE);
    RCC_PB2PeriphClockCmd (RCC_PB2Periph_TIM1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits (GPIOC, GPIO_Pin_5);  // Inactive low

    TIM_TimeBaseStructInit (&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 20 - 1; //1Khz
    TIM_TimeBaseInitStructure.TIM_Period = 2400 - 1;  // 20kHz
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

void BSP_TIM2_Init (void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};

    RCC_PB2PeriphClockCmd (RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOC, ENABLE);
    RCC_PB1PeriphClockCmd (RCC_PB1Periph_TIM2, ENABLE);
    GPIO_PinRemapConfig (GPIO_PartialRemap3_TIM2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInitStructure.TIM_Period = 2400 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;
    TIM_TimeBaseInit (TIM2, &TIM_TimeBaseInitStructure);

    TIM_OCStructInit (&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 1200;
    TIM_OC2Init (TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig (TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig (TIM2, ENABLE);

    TIM_Cmd (TIM2, ENABLE);
    Buzzer_Play (900, 100);
}

void Buzzer_Play (uint32_t freq, uint32_t ms) {
    buzzer_ms = ms;
    TIM2->PSC = 20000.0f / freq;
    TIM_Cmd (TIM2, ENABLE);
}

void BSP_PWM_DAC_Set_CCR (uint16_t ccr) {
    TIM_SetCompare3 (TIM1, ccr);
}

void TIM1_UP_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler (void) {
    TIM_ClearITPendingBit (TIM1, TIM_IT_Update);
    if (!buzzer_ms) {
        TIM_Cmd (TIM2, DISABLE);
        buzzer_ms = 0;
    } else {
        buzzer_ms--;
    }
    BSP_TIM1_IQR_Callback();
}

__weak_symbol void BSP_TIM1_IQR_Callback (void) {
}