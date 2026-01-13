#ifndef ENCODER_K_H
#define ENCODER_K_H

#define ENCODER_EXTI_MODE 0
#define ENCODER_TIMER_MODE 1

#define ENCODER_MODE ENCODER_EXTI_MODE

void BSP_Encoder_Init();

float BSP_Encoder_Get_Cnt (void);
void BSP_Encoder_Set_Cnt (volatile float *value);

void BSP_Encoder_Set_Step_Value (float step);
float BSP_Encoder_Get_Step_Value();


void BSP_EncoderCNT_Set_Range (float min, float max);
#endif
