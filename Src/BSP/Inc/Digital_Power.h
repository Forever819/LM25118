#include <ch32v00X.h>
#include "PID.h"
#include "Timer.h"
#include "Event_Bus.h"

#define LM25118_ENABLE GPIO_SetBits(GPIOC, GPIO_Pin_5);
#define LM25118_DISABLE GPIO_ResetBits(GPIOC, GPIO_Pin_5);

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
    OVT,
    FAULT_Lock
} Digital_Power_State;

typedef struct {
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
PowerEvent_t Digital_Power_State_Update(void);
u8 Protection_Check(void);

