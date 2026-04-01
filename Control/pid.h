#ifndef __PID_H
#define __PID_H

#include "stm32f10x.h"

#define INTEGRAL_MAX  1000
#define Min_Max(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define Max_PWM       3600

int Temp_To_Speed(u8 temp);
int Speed_Control(int NowPoint, int SetPoint, int *TURN_PID);
void Speed_PID_Reset(void);

extern const int Speed_PID[3];

#endif
