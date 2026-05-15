//coding: utf-8
#include "Buzzer.h"
#include "debug.h"

#define USE_BUZZER_TIM_ISR 0

static BuzzerNode buzzer_queue[BUZZER_QUEUE_SIZE];
static volatile uint8_t buzzer_head = 0;
static volatile uint8_t buzzer_tail = 0;
static volatile uint8_t buzzer_playing = 0;
static volatile int32_t buzzer_ms = 0;
static volatile uint32_t buzzer_ccr = 0; // 改为 uint32_t

static void Buzzer_Stop_PWM(void)
{
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Disable);
}

static void Buzzer_Start_PWM(void)
{
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable);
}

static void Buzzer_PWM_Set_CCR(uint32_t c)
{
    TIM_SetCompare2(TIM2, c);
}

static void Buzzer_PWM_Set_ARR(uint32_t c)
{
    TIM_SetAutoreload(TIM2, c);
}

static uint8_t Buzzer_Enqueue(uint32_t freq, uint32_t ms)
{
    uint8_t next_tail = (buzzer_tail + 1) % BUZZER_QUEUE_SIZE;
    if (next_tail == buzzer_head)
        return 0; // 队列满

    buzzer_queue[buzzer_tail].freq = freq;
    buzzer_queue[buzzer_tail].time_ms = ms;
    buzzer_tail = next_tail;

    return 1;
}

static void Buzzer_StartNext(void)
{
    if (buzzer_head == buzzer_tail)
    {
        Buzzer_Stop_PWM();
        buzzer_playing = 0;
        return;
    }

    BuzzerNode *n = &buzzer_queue[buzzer_head];
    buzzer_head = (buzzer_head + 1) % BUZZER_QUEUE_SIZE;

    // 计算 ARR 值（假设 TIM2 时钟为 1MHz）
    if (n->freq == 0)
    {
        Buzzer_PWM_Set_ARR(1);
    }
    else
    {
        uint32_t arr = 1000000 / n->freq;
        Buzzer_PWM_Set_ARR(arr); // 设置周期
        buzzer_ccr = arr / 2;    // 50% 占空比
    }
    buzzer_ms = n->time_ms;
    buzzer_playing = 1;

    Buzzer_PWM_Set_CCR(buzzer_ccr);
    Buzzer_Start_PWM();
}

/*********************************************************************
 * @fn      Buzzer_Init
 *
 * @brief   TIM2 is initialized as a PWM output, driven buzzerr.
 *
 * @param   none
 *          
 *
 * @return  none
 */
void Buzzer_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOC, ENABLE);
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_TIM2, ENABLE);
    GPIO_PinRemapConfig(GPIO_PartialRemap3_TIM2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInitStructure.TIM_Period = 65535 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

#if USE_BUZZER_TIM_ISR

    NVIC_InitTypeDef nvic = {0};
    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

#endif

    TIM_Cmd(TIM2, ENABLE);
    Buzzer_Play(1000, 200);
}

#if USE_BUZZER_TIM_ISR
// 最好在 XXXX_it.c 中统一管理
void TIM2_IRQHandler(void) __attribute__((interrupt));
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        // 替代 BSP_Buzzer_Tick() 的毫秒计数
        // 需要分频，因为TIM2_UP中断频率很高
        // 可用计数器 1000 次 = 1ms
        static u32 div = 0;
        if (div++ > 1000)
        {
            BSP_Buzzer_Tick();
            div = 0;
        }
    }
}
#endif

/*********************************************************************
 * @fn      Buzzer_Play
 *
 * @brief   Play a tone.
 *
 * @param   freq - frequency of the tone
 * @param   ms - duration of the tone
 *
 * @return  none
 */
void Buzzer_Play(uint32_t freq, uint32_t ms)
{
    if (ms == 0)
        return;

    if (!Buzzer_Enqueue(freq, ms))
    {
        // 队列满，可以选择丢弃或等待
        return;
    }

    // 如果当前没有播放，立即启动
    if (!buzzer_playing)
    {
        Buzzer_StartNext();
    }
}

// require 1ms scan ，using  other timer isr or enable buzzer timer isr
void BSP_Buzzer_Tick(void)
{
    if (buzzer_playing && buzzer_ms > 0)
    {
        buzzer_ms--;
        if (buzzer_ms <= 0)
            Buzzer_StartNext();
    }
}
