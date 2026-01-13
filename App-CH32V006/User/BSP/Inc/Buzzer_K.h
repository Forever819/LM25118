#define BUZZER_QUEUE_SIZE 5
#define BUCK_MAX_PWM_PERIOD 100
#include "stdint.h"
typedef struct {
    uint32_t freq;
    uint32_t time_ms;
} BuzzerNode;


void Buzzer_Play (uint32_t freq, uint32_t ms);
void BSP_Buzzer_Task (void);
