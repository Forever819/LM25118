#include "debug.h"
#include "Encoder_k.h"

static volatile float *Encoder_CNT;
static volatile float Encoder_CNT_MAX, Encoder_CNT_MIN;
static volatile float Encoder_Step_Value;

void BSP_Encoder_Init() {
#if (ENCODER_MODE == ENCODER_EXTI_MODE)

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_PB2PeriphClockCmd (RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOC | RCC_PB2Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    GPIO_EXTILineConfig (GPIO_PortSourceGPIOC, GPIO_PinSource0);  // gpio组通过afio选择line0-7所以要使能afio
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_Init (&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init (&NVIC_InitStructure);
    *Encoder_CNT = 0;
    Encoder_Step_Value = 1.0f;
#elif (ENCODER_MODE == ENCODER_TIMER_MODE)

#endif
}

#if (ENCODER_MODE == ENCODER_EXTI_MODE)
void EXTI7_0_IRQHandler (void) __attribute__ ((interrupt));

void EXTI7_0_IRQHandler (void) {
    if (EXTI_GetITStatus (EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line0);
        if (GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_0) == Bit_RESET)
            *Encoder_CNT += Encoder_Step_Value;
        else
            *Encoder_CNT -= Encoder_Step_Value;
        //限幅
        if (*Encoder_CNT > Encoder_CNT_MAX) {
            *Encoder_CNT = Encoder_CNT_MAX;
        } else if (*Encoder_CNT < Encoder_CNT_MIN)
            *Encoder_CNT = Encoder_CNT_MIN;
    }
}

//获取当前值
float BSP_Encoder_Get_Cnt (void) {
    return *Encoder_CNT;
}

//设置修改值
void BSP_Encoder_Set_Cnt (volatile float *value) {
    Encoder_CNT = value;
}

//设置步进值
void BSP_Encoder_Set_Step_Value (float step) {
    Encoder_Step_Value = step;
}

//获取步进值
float BSP_Encoder_Get_Step_Value() {
    return Encoder_Step_Value;
}

void BSP_EncoderCNT_Set_Range (float min, float max) {
    Encoder_CNT_MAX = max;
    Encoder_CNT_MIN = min;
}
#elif (ENCODER_MODE == ENCODER_TIMER_MODE)

#endif