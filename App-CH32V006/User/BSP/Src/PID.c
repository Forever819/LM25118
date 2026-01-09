#include "pid.h"

PID PID_Voltage = {
    .Kp = 0,
    .Ki = 1,
    .Kd = 0,
    .outputmax = 2400,
    .outputmin = 0,
    .SumErrormax = 2400,
};

PID PID_Current = {
    .Kp = 5,
    .Ki = 1,
    .Kd = 0,
    .outputmax = 19000,
    .outputmin = 0,
    .SumErrormax = 19000,
};

void PID_Reset (PID *pid) {
    pid->Current_Error = 0;
    pid->Last_Error = 0;
    pid->Previous_Error = 0;
    pid->SumError = 0;
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
    pid->output = pid->Kp * pid->Current_Error + pid->Ki * pid->SumError + pid->Kd * (pid->Current_Error - pid->Last_Error);
    pid->SumError += pid->Current_Error;
    pid->Last_Error = pid->Current_Error;
    if (pid->output > pid->outputmax)
        pid->output = pid->outputmax;
    else if (pid->output < pid->outputmin)
        pid->output = pid->outputmin;

    if (pid->SumError > pid->SumErrormax)
        pid->SumError = pid->SumErrormax;
    else if (pid->SumError < -pid->SumErrormax)
        pid->SumError = -pid->SumErrormax;
    return pid->output;
}