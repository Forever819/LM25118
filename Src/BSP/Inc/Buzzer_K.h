#define BUZZER_QUEUE_SIZE 10
#define BUCK_MAX_PWM_PERIOD 100
#include "stdint.h"

typedef struct {
    uint32_t freq;
    uint32_t time_ms;
} BuzzerNode;

void Buzzer_TIM_Init(void);
void Buzzer_Play (uint32_t freq, uint32_t ms);
void BSP_Buzzer_Tick (void);
