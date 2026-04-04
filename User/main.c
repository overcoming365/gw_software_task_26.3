#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "DHT11.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "Timer.h"
#include "pid.h"


#define MODE_IDLE      0
#define MODE_SINGLE    1
#define MODE_MOTOR     2
#define MODE_REALTIME  3

uint8_t system_mode = MODE_IDLE;
u8 humidity = 0, temperature = 0;
int target_speed = 0;
int pwm_output = 0;

void Process_Command(uint8_t cmd);
void Mode_Motor_Update(void);
void Mode_Realtime_Update(void);
void Mode_Reset(void);
void OLED_Show_TempHumi(void);

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	OLED_Init();
	Key_Init();
	DHT11_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	Timer_Init();

	OLED_Clear();
	OLED_ShowString(0, 0, "System Ready", OLED_8X16);
	OLED_Update();

	while (1)
	{
		uint8_t keyNum = Key_GetNum();
		if (keyNum != 0)
		{
			Process_Command(keyNum);
		}

		if (Serial_GetRxFlag())
		{
			uint8_t rxData = Serial_GetRxData();
			if (rxData >= '1' && rxData <= '4')
			{
				Process_Command(rxData - '0');
			}
		}

		if (system_mode == MODE_MOTOR && TIM2_100ms_Flag)
		{
			TIM2_100ms_Flag = 0;
			Mode_Motor_Update();
		}

		if (system_mode == MODE_REALTIME && TIM2_1000ms_Flag)
		{
			TIM2_1000ms_Flag = 0;
			Mode_Realtime_Update();
		}
	}
}

void Process_Command(uint8_t cmd)
{
	switch (cmd)
	{
		case 1:
			system_mode = MODE_SINGLE;
			Motor_SetPWM_L(0);
			DHT11_Read_Data(&humidity, &temperature);
			OLED_Show_TempHumi();
			Serial_Printf("Temp:%d Humi:%d\r\n", temperature, humidity);
			break;

		case 2:
			system_mode = MODE_MOTOR;
			DHT11_Read_Data(&humidity, &temperature);
			target_speed = Temp_To_Speed(temperature);
			Speed_PID_Reset();
			Encoder_Get();	
//			Motor_SetPWM_L(3600);	
			OLED_Clear();
			OLED_Printf(0, 0, OLED_8X16, "Motor Mode");
			OLED_Printf(0, 16, OLED_8X16, "Temp:%d", temperature);
			OLED_Printf(0, 32, OLED_8X16, "Target:%d", target_speed);
			OLED_Update();
			Serial_Printf("Motor ON T:%d S:%d\r\n", temperature, target_speed);
			break;

		case 3:
			system_mode = MODE_REALTIME;
			Motor_SetPWM_L(0);
			OLED_Clear();
			OLED_ShowString(0, 0, "Realtime Mode", OLED_8X16);
			OLED_Update();
			Serial_Printf("Realtime ON\r\n");
			break;

		case 4:
			Mode_Reset();
			Serial_Printf("Reset OK\r\n");
			break;
	}
}

void Mode_Motor_Update(void)
{
	static uint8_t display_cnt = 0;
	static int16_t encoder_speed = 0;

	encoder_speed = Encoder_Get();
	pwm_output = Speed_Control(encoder_speed, target_speed, (int *)Speed_PID);
	pwm_output = Min_Max(pwm_output, 0, Max_PWM);
	if (target_speed > 0 && pwm_output < 500)
		pwm_output = 500;
	Motor_SetPWM_L(pwm_output);

	display_cnt++;
	if (display_cnt >= 5)
	{
		display_cnt = 0;
		DHT11_Read_Data(&humidity, &temperature);
		target_speed = Temp_To_Speed(temperature);
		OLED_Clear();
		OLED_Printf(0, 0, OLED_8X16, "Temp:%d", temperature);
		OLED_Printf(0, 16, OLED_8X16, "Target:%d", target_speed);
		OLED_Printf(0, 32, OLED_8X16, "PWM:%d", pwm_output);
		OLED_Printf(0, 48, OLED_8X16, "curr:%d%%", encoder_speed);
		OLED_Update();

		Serial_Printf("T:%d H:%d S:%d PWM:%d\r\n", temperature, humidity, target_speed, pwm_output);
	}
}

void Mode_Realtime_Update(void)
{
	DHT11_Read_Data(&humidity, &temperature);
	OLED_Show_TempHumi();
	Serial_Printf("Temp:%d Humi:%d\r\n", temperature, humidity);
}

void Mode_Reset(void)
{
	system_mode = MODE_IDLE;
	temperature = 0;
	humidity = 0;
	target_speed = 0;
	pwm_output = 0;

	Motor_SetPWM_L(0);
	Motor_SetPWM_R(0);
	Speed_PID_Reset();

	OLED_Clear();
	OLED_ShowString(0, 0, "System Ready", OLED_8X16);
	OLED_Update();
}

void OLED_Show_TempHumi(void)
{
	OLED_Clear();
	OLED_Printf(0, 0, OLED_8X16, "Humidity:%d%%", humidity);
	OLED_Printf(0, 24, OLED_8X16, "Temp:%dC", temperature);
	OLED_Update();
}

