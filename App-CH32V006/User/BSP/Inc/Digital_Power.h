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
    IDLE = 0,
    CC,
    CV,
    OVP,
    OCP
} Digital_Power_State;

typedef struct {
    Digital_Power_State sys_state;
    u8 System_Enable_Flag;

    float Vset;
    float Iset;

    float current_input_limit;
    float current_output_limit;
    float voltage_intput_limit;
    float voltage_output_limit;

} Digital_Power_Dev;
