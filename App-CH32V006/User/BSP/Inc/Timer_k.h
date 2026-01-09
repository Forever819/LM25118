#ifndef TIMER_K_H
#define TIMER_K_H


void BSP_TIM1_Init (void);
void BSP_TIM2_Init (void);

void Buzzer_Play (uint32_t freq, uint32_t ms);

void BSP_PWM_DAC_EN (uint8_t en);
void BSP_PWM_DAC_Set_CCR (uint16_t ccr);

void BSP_TIM1_IQR_Callback (void);

#endif