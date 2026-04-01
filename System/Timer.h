#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

void Timer_Init(void);

extern volatile uint8_t TIM2_100ms_Flag;
extern volatile uint8_t TIM2_1000ms_Flag;

#endif
