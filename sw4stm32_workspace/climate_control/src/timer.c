/*
 * timer.c
 *
 *  Created on: 8 Jul 2016
 *      Author: mark
 */
#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#include "gpio.h"
#include "rcc.h"

void InitTimer1WithCompareOutput(uint16_t cycletime_ms)
{
	// fix zero cycle time
	cycletime_ms = (cycletime_ms > 1) ? cycletime_ms : 1;

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	// ensure the counter is disabled prior to configuration
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM1->CCER &=~TIM_CCER_CC1E;

	//TIM1->CR1  = TIM_CR1_ARPE;
	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

	TIM1->BDTR = TIM_BDTR_MOE | TIM_BDTR_AOE; // main output enable, automatic output enable.

	// set up for 50% duty pwm
	TIM1->ARR = cycletime_ms * 2 -1;
	TIM1->CCR1 = cycletime_ms;

	// AHB and APB prescalers are at 1 so SystemCoreClock is output direct to the timer.
	// This will give us 1/2ms per tick.
	TIM1->PSC = (SystemCoreClock / GetApbPrescaler() / 2000) - 1;

	// set up output on interrupt pin
	GPIO_AS_AF(A,8);
	GPIO_SELECT_AF(A,8,2);


	TIM1->CNT = TIM1->CCR1; // put the output in a low state

	// enable the counter now that configuration is complete
	TIM1->CCER |= TIM_CCER_CC1E;
	// TIM1->CR1 |= TIM_CR1_CEN; // don't start yet.

}

void StopAndResetTimer1(void)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM1->CNT = TIM1->CCR1;
}

void StartTimer1(void)
{
	TIM1->CNT = 0;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void InitTimer14WithCompareOutput(uint16_t cycletime_us)
{
	// fix zero cycle time
	cycletime_us = (cycletime_us > 1) ? cycletime_us : 1;

	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

	// ensure the counter is disabled prior to configuration
	TIM14->CR1 &= ~TIM_CR1_CEN;
	TIM14->CCER &=~TIM_CCER_CC1E;

	//TIM14->CR1  = TIM_CR1_ARPE;
	TIM14->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

	TIM14->BDTR = TIM_BDTR_MOE | TIM_BDTR_AOE; // main output enable, automatic output enable.

	// set up for 50% duty pwm
	TIM14->ARR = cycletime_us * 2 - 1;
	TIM14->CCR1 = cycletime_us;

	// AHB and APB prescalers are at 1 so SystemCoreClock is output direct to the timer.
	// This will give us 1/2us per tick.
	TIM14->PSC = (SystemCoreClock / GetApbPrescaler() / 2000000) - 1;

	// set up output on carrier pin
	GPIO_AS_AF(A,4);
	GPIO_SELECT_AF(A,4,4);


	TIM14->CNT = TIM14->CCR1; // put the output in a low state

	// enable the counter now that configuration is complete
	TIM14->CCER |= TIM_CCER_CC1E;
	// TIM14->CR1 |= TIM_CR1_CEN; // don't start yet.

}

void StopAndResetTimer14(void)
{
	TIM14->CR1 &= ~TIM_CR1_CEN;
	TIM14->CNT = TIM14->CCR1;
}

void StartTimer14(void)
{
	TIM14->CNT = 0;
	TIM14->CR1 |= TIM_CR1_CEN;
}

void InitTimer3ForAdcTrigger(uint16_t cycletime_us)
{
	// fix zero cycle time
	cycletime_us = (cycletime_us > 1) ? cycletime_us : 1;

	// ensure timer clock is enabled.
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	// ensure the counter is disabled prior to configuration
	TIM3->CR1 &= ~TIM_CR1_CEN;
	TIM3->CCER &=~TIM_CCER_CC1E;

	// enable auto-reload/preload so that we can trim the sample rate on the fly
	TIM3->CR1 |= TIM_CR1_ARPE;

	TIM3->CR2 = TIM_CR2_MMS_1; //MMS 010 = TRG0 = UEV (trigger on update event)

	TIM3->SMCR = 0; // defaults are fine for this register

	TIM3->DIER = 0; // interrupts and DMA are not required for the timer

	// get timer clock, see clock tree for conditional doubling of tclk
	uint32_t tclk = SystemCoreClock / GetApbPrescaler();
	if (GetApbPrescaler() != 1)
	{
		tclk *= 2;
	}

	// set up for correct cycletime
	TIM3->ARR = cycletime_us - 1;

	// AHB and APB prescalers are at 1 so SystemCoreClock is output direct to the timer.
	// This will give us 1us per tick.
	TIM3->PSC = (tclk /1000000UL) - 1;

	// set up output on interrupt pin
	GPIO_AS_AF(A,8);
	GPIO_SELECT_AF(A,8,2);


	TIM3->CNT = TIM1->CCR1; // put the output in a low state

	// enable the counter now that configuration is complete
	TIM3->CCER |= TIM_CCER_CC1E;
	// TIM3->CR1 |= TIM_CR1_CEN; // don't start yet.

}

void AdjustTimer3Interval(uint16_t new_interval_us)
{
	new_interval_us = (new_interval_us > 0) ? new_interval_us : 1;
	TIM3->ARR = new_interval_us - 1;
}

uint16_t GetTimer3IntervalUs()
{
	return TIM3->ARR + 1;
}

void StopAndResetTimer3(void)
{
	TIM3->CR1 &= ~TIM_CR1_CEN;
}

void StartTimer3(void)
{
	TIM3->CR1 |= TIM_CR1_CEN;
}

#endif
