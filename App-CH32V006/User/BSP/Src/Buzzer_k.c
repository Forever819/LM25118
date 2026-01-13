#include "Buzzer_K.h"
#include "debug.h"

static BuzzerNode buzzer_queue[BUZZER_QUEUE_SIZE];
static volatile uint8_t buzzer_head = 0;
static volatile uint8_t buzzer_tail = 0;
static volatile uint8_t buzzer_playing = 0;
static volatile int32_t buzzer_ms = 0;
static volatile uint32_t buzzer_ccr = 0;  // 改为 uint32_t

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
        return 0;  // 队列满
        
    buzzer_queue[buzzer_tail].freq = freq;
    buzzer_queue[buzzer_tail].time_ms = ms;
    buzzer_tail = next_tail;
    
    return 1;
}

static void Buzzer_StartNext(void) 
{
    if (buzzer_head == buzzer_tail) {
        Buzzer_Stop_PWM();
        buzzer_playing = 0;
        return;
    }
    
    BuzzerNode *n = &buzzer_queue[buzzer_head];
    buzzer_head = (buzzer_head + 1) % BUZZER_QUEUE_SIZE;
    
    // 计算 ARR 值（假设 TIM2 时钟为 1MHz）
    uint32_t arr = 1000000 / n->freq;
    Buzzer_PWM_Set_ARR(arr);  // 设置周期
    
    buzzer_ccr = arr / 2;  // 50% 占空比
    buzzer_ms = n->time_ms;
    buzzer_playing = 1;
    
    Buzzer_PWM_Set_CCR(buzzer_ccr);
    Buzzer_Start_PWM();
}

void Buzzer_Play(uint32_t freq, uint32_t ms) 
{
    if (freq == 0 || ms == 0)
        return;
    
    if (!Buzzer_Enqueue(freq, ms)) {
        // 队列满，可以选择丢弃或等待
        return;
    }
    
    // 如果当前没有播放，立即启动
    if (!buzzer_playing) {
        Buzzer_StartNext();
    }
}

// require 1ms scan
void BSP_Buzzer_Task(void) 
{
    if (!buzzer_playing) {
        return;  // 没有播放任务，直接返回
    }
    
    buzzer_ms--;
    if (buzzer_ms <= 0) {
        Buzzer_StartNext();  // 当前音符结束，播放下一个
    }
}