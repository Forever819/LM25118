#include "pid.h"
#include "stdint.h"
#include "Timer.h"
PID PID_Voltage = {
    .Kp = 10,
    .Ki = 5.88,
    .Kd = 0,
    .outputmax = MAX_PWM_PERIOD,
    .outputmin = 0,
};

PID PID_Current = {
    .Kp = 20,
    .Ki = 10,
    .Kd = 0,
    .outputmax = 0,
    .outputmin = -45,
};


void PID_Reset (PID *pid) {
    pid->Current_Error = 0;
    pid->Last_Error = 0;
    pid->Previous_Error = 0;
    pid->SumError = 0;
    pid->Incremental = 0;
    pid->output = 0;
}

// 增量式PID
float PID_Incremental_Calc (PID *pid, float Target_val, float Actual_val) {
    pid->Actual_val = Actual_val;
    pid->Target_val = Target_val;
    pid->Current_Error = pid->Target_val - pid->Actual_val;
    pid->Incremental = pid->Kp * (pid->Current_Error - pid->Last_Error) + pid->Ki * pid->Current_Error + pid->Kd * (pid->Current_Error - 2 * pid->Last_Error + pid->Previous_Error);
    pid->output += pid->Incremental;
    pid->Previous_Error = pid->Last_Error;
    pid->Last_Error = pid->Current_Error;
    if (pid->output > pid->outputmax)
        pid->output = pid->outputmax;
    else if (pid->output < pid->outputmin)
        pid->output = pid->outputmin;


    return pid->output;
}

float PID_Position_Calc (PID *pid, float Target_val, float Actual_val) {
    pid->Actual_val = Actual_val;
    pid->Target_val = Target_val;
    pid->Current_Error = pid->Target_val - pid->Actual_val;
    pid->SumError += pid->Current_Error;

    //累积误差限幅
    if (pid->SumError > pid->SumErrormax)
        pid->SumError = pid->SumErrormax;
    else if (pid->SumError < -pid->SumErrormax)
        pid->SumError = -pid->SumErrormax;

    pid->output = pid->Kp * pid->Current_Error + pid->Ki * pid->SumError + pid->Kd * (pid->Current_Error - pid->Last_Error);
    //输出限幅
    if (pid->output > pid->outputmax) {
        pid->output = pid->outputmax;
        if (pid->Current_Error > 0)
            pid->SumError -= pid->Current_Error;
    } else if (pid->output < pid->outputmin) {
        pid->output = pid->outputmin;
        if (pid->Current_Error < 0)
            pid->SumError -= pid->Current_Error;
    }

    pid->Last_Error = pid->Current_Error;

    return pid->output;
}
