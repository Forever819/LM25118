#include <ch32v00X.h>
#include "PID.h"
#include "Timer.h"
#include "Event_Bus.h"

#define LM25118_ENABLE GPIO_SetBits(GPIOC, GPIO_Pin_5);
#define LM25118_DISABLE                    \
    {                                      \
        GPIO_ResetBits(GPIOC, GPIO_Pin_5); \
        dp.System_Enable_Flag = 0;         \
        PID_Reset(&PID_Voltage);           \
        PID_Reset(&PID_Current);           \
    }

typedef enum
{
    DP_IDLE_Switch = 0,
    DP_IDLE,
    DP_CC_Switch,
    DP_CC,
    DP_CV_Switch,
    DP_CV,
    DP_OVP,
    DP_OCP,
    DP_SCP,
    DP_OTP,
    DP_OPP,
    DP_FAULT_LOCK
} Digital_Power_State;

typedef struct
{
    Digital_Power_State sys_state;
    u8 System_Enable_Flag;

    float OVP_Thredhold;
    float OCP_Thredhold;

    float Vset;
    float Iset;
    float Pset;

} Digital_Power_Dev;

extern Digital_Power_Dev dp;

void Key_Event_Proc(void);
void Digital_Power_Init(void);
PowerEvent_t Digital_Power_State_Event(void);
u8 Protection_Check(void);
