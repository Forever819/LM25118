#ifndef LED_K_H
#define LED_K_H

#define LED_GPIO_MODE 0
#define LED_TIMER_MODE 1

#define LED_MODE LED_GPIO_MODE

void BSP_LED_Init (void);

#if (LED_MODE == LED_GPIO_MODE)
void BSP_LED_Toggle (void);
#elif (LED_MODE == LED_TIMER_MODE)
void BSP_LED_SetBrigtness (uint16_t ccr);
void BSP_LED_Breathe (FunctionalState state);

#endif

#endif
