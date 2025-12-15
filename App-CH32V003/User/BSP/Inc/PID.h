#ifndef __PID_H__
#define __PID_H__

// PID控制结构体
typedef struct PID
{
    int Kp;             //  Proportional Const  P系数
    int Ki;             //  Integral Const      I系数
    int Kd;             //  Derivative Const    D系数
    int Target_val;     //  Target
    int Actual_val;     //  Actual
    int Previous_Error; //  Error[-2]
    int Last_Error;     //  Error[-1]
    int Current_Error;  //  Error[0 ]

    int SumError; //  Sums of Errors

    int Incremental;
    int output;

    int Error_Tolerance; // 误差容忍度
    int Integralmax;     // 积分项的最大值
    int outputmax;       
    int outputmin;       
} PID;

extern PID PID_Voltage,PID_Current;

void PID_Reset(PID *pid);
float PID_Incremental_Calc(PID *pid, float Target_val, float Actual_val);
float PID_Position_Calc(PID *pid, float Target_val, float Actual_val);

#endif // !__PID_H__
