#include "debug.h"

#include "Timer_k.h"
#include "Key_k.h"
#include "Encoder_k.h"
#include "WS2812_k.h"
#include "OLED.h"
#include "ADC_k.h"
#include "PID.h"
#include "Buzzer_K.h"

typedef enum {
    IDLE_Switch = 0,
    IDLE,
    CC_Switch,
    CC,
    CV_Switch,
    CV,
    OVP,
    OCP,
    SCP,
    FAULT_Lock
} Digital_Power_State;

typedef struct {
    Digital_Power_State sys_state;
    u8 System_Enable_Flag;

    float OVP_Thredhold;
    float OCP_Thredhold;

    s32 Vset;
    s32 Iset;
    s32 Pset;

} Digital_Power_Dev;

extern Digital_Power_Dev dp;
