#ifndef ENCODER_K_H
#define ENCODER_K_H

#define ENCODER_EXTI_MODE   0
#define ENCODER_TIMER_MODE  1

#define ENCODER_MODE    ENCODER_EXTI_MODE

void BSP_Encoder_Init();
int16_t BSP_Encoder_Get_Cnt(void);

#endif
