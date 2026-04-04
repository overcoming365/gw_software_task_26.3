#include "stm32f10x.h"                  // Device header
#include "pid.h"

static int PID_Integral = 0;
static int PID_LastError = 0;

const int Speed_PID[3] = {1, 0, 0};  // Kp, Ki, Kd

int Temp_To_Speed(u8 temp)
{
	int speed;
	if (temp < 20) return 0;
	if (temp > 40) return Max_PWM;
	speed = (int)(temp - 20) * 155 + 500;
	return speed;
}

int Speed_Control(int NowPoint, int SetPoint, int *TURN_PID)
{
	int NowError = SetPoint - NowPoint;

	PID_Integral += NowError;
	PID_Integral = Min_Max(PID_Integral, -INTEGRAL_MAX, INTEGRAL_MAX);

	int Out = TURN_PID[0] * NowError + TURN_PID[1] * PID_Integral + TURN_PID[2] * (NowError - PID_LastError);
	PID_LastError = NowError;
	return Out;
}

void Speed_PID_Reset(void)
{
	PID_Integral = 0;
	PID_LastError = 0;
}
