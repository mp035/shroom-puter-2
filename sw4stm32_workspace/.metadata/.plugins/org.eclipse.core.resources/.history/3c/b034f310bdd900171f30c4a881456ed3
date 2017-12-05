#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#endif

#include <stdlib.h>
#include "rcc.h"
#include "gpio.h"
#include "main.h" // to control peltier temperature


// peltier duty cycle is handled in systick.c
uint8_t peltier_duty_count = 0;
volatile int8_t peltier_duty_setting = 0;// negative values for cooling, positive for heating
uint16_t peltier_ramp_counter = 0;
volatile uint8_t peltier_heating = 0;
volatile uint8_t peltier_cooling = 0;

volatile uint32_t DelayTickerMs = 0;
volatile uint32_t InterruptMs;
volatile uint64_t SystickMs = 0;


void heating(uint16_t x)
{
	return;
	GPIO_CLR(B,3);// cooling pos
	GPIO_CLR(A,0);// cooling neg
	if (x)
	{
		GPIO_SET(A,1);// heating pos
		GPIO_SET(A,3);// heating neg
	}
	else
	{
		GPIO_CLR(A,1);// heating pos
		GPIO_CLR(A,3);// heating neg
	}
}

void cooling(uint16_t x)
{
	return;
	GPIO_CLR(A,1);// heating pos
	GPIO_CLR(A,3);// heating neg
	if (x)
	{
		GPIO_SET(B,3);// cooling pos
		GPIO_SET(A,0);// cooling neg
	}
	else
	{
		GPIO_CLR(B,3);// cooling pos
		GPIO_CLR(A,0);// cooling neg
	}
}


void SysTick_Handler(void)
{
	// 100 tick period for peltier
	peltier_duty_count++;
	if (peltier_duty_count >= 100) peltier_duty_count = 0;

	if (peltier_duty_count < abs(peltier_duty_setting))
	{
		if (peltier_duty_setting < 0){ // we are cooling
			cooling(1);
		}
		if (peltier_duty_setting > 0){ // we are heating
			heating(1);
		}
	}
	else
	{
		cooling(0);
		heating(0);
	}

	// ramp duty as required
	peltier_ramp_counter++;
	if (peltier_ramp_counter >=5000){ // adjust duty every 5 seconds because that is how often the temperature value is updated.
		peltier_ramp_counter = 0;
		if (disable_temp) // ramp towards zero duty if temp is disabled
		{
			if (peltier_duty_setting > 0){
				peltier_duty_setting --;
			}
			if (peltier_duty_setting < 0){
				peltier_duty_setting++;
			}
		}
		else if(peltier_cooling)
		{
			if (peltier_duty_setting > -100){
				peltier_duty_setting--;
			}
		}
		else if(peltier_heating)
		{
			if (peltier_duty_setting < 100){
				peltier_duty_setting++;
			}
		}
	}


	DelayTickerMs += InterruptMs;
	SystickMs += InterruptMs;
}

// the interrupt interval only affects the resolution.  You can get
// 100ms delay resolution with 100ms interrupt interval
// smaller values will consume more resources due to increased
// context switching.
void SetupSysTick(uint32_t interrupt_interval_ms)
{
	int sysTickVal = SystemCoreClock / 1000 * interrupt_interval_ms;
	SysTick_Config(sysTickVal);
	InterruptMs = interrupt_interval_ms;
	NVIC_SetPriority(SysTick_IRQn,3); // 3 is lowest priority, 0 is highest
	NVIC_EnableIRQ(SysTick_IRQn); // enable the interrupt in NVIC
}

void DelayMs(uint32_t delay_ms)
{
	DelayTickerMs = 0;
	// if the resolution is too low to get the required delay, ensure that the
	// delay is longer than specified, rather than shorter.
	delay_ms = (delay_ms < InterruptMs * 2) ? InterruptMs * 2 : delay_ms;
	while (DelayTickerMs < delay_ms);
}

uint64_t GetSystickMs(void)
{
	// re-read SystickMs twice, and repeat as required
	// until two consecutive reads match to ensure that
	// the value was not changed by interrupt during
	// read because uint64_t reads are non-atomic on cortex

	uint64_t temp = SystickMs;
	while (temp != SystickMs)
	{
		temp = SystickMs;
	}
	return temp;
}
