#ifndef TIMER_H
#define TIMER_H

#define MAX_PWM_PERIOD 9600

void BSP_PWM_Init (void);
void BSP_PWM_Set_CCR (uint16_t ccr);

void BSP_TIM1_ISR_Callback (void);

#endif