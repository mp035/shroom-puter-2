#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#endif

#include <stdlib.h>
#include "rcc.h"
#include "gpio.h"

volatile uint32_t DelayTickerMs = 0;
volatile uint32_t InterruptMs;
volatile uint64_t SystickMs = 0;

void SysTick_Handler(void)
{
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
