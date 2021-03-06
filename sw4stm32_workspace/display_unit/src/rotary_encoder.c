/*
 * rotary_encoder.c
 *
 *  Created on: 1Dec.,2017
 *      Author: mark
 */

#include <stm32l0xx.h>
#include <stdint.h>
#include "main.h"
#include "gpio.h"
#include "systick.h"

int encoder_value = 0;
int button_pressed = 0;
uint8_t flags = 0;
uint8_t clearcount = 0;

#define FLAG_NEGATIVE 1
#define FLAG_POSITIVE 2
#define FLAG_BUTTON 4

int encoder_count = 0;
int encoder_trend = 0;

void RotaryEncoderInit()
{
	// Setup pins for Rotary Encoder
	GPIO_AS_INPUT(A,6);
	GPIO_AS_INPUT(A,7);
	GPIO_AS_INPUT(A,8);
	GPIO_ENABLE_PULLUP(A,6);
	GPIO_ENABLE_PULLUP(A,7);
	GPIO_ENABLE_PULLUP(A,8);
	// and interrupts
	EXTI->IMR = EXTI_IMR_IM6 | EXTI_IMR_IM7 | EXTI_IMR_IM8;
	// on falling edge
	EXTI->FTSR = EXTI_FTSR_FT6 | EXTI_FTSR_FT7 | EXTI_FTSR_FT8;
	// as well as rising edge (so we can detect contact release)
	//EXTI->RTSR = EXTI_RTSR_RT6 | EXTI_RTSR_RT7 | EXTI_RTSR_RT8;
	// ensure we select port A
	SYSCFG->EXTICR[2] = SYSCFG_EXTICR2_EXTI6_PA | SYSCFG_EXTICR2_EXTI7_PA;
	SYSCFG->EXTICR[3] = SYSCFG_EXTICR3_EXTI8_PA;
	// enable in the NVIC
	NVIC_SetPriority(EXTI4_15_IRQn,1);
	NVIC_EnableIRQ(EXTI4_15_IRQn);
}

int RotaryEncoderHasActivity()
{
	return button_pressed || encoder_value;
}

int RotaryEncoderGetValue()
{
	int temp;
	//NVIC_DisablRQ(EXTI4_15_IRQn);
	//__DSB(); //Data sync barrier
	//__ISB(); //Instruction sync barrier
	temp = encoder_value;
	encoder_value = 0;
	//NVIC_EnableIRQ(EXTI4_15_IRQn);
	return temp;
}

int RotaryEncoderGetPressed()
{
	int temp;
	//NVIC_DisableIRQ(EXTI4_15_IRQn);
	temp = button_pressed;
	button_pressed = 0;
	//NVIC_EnableIRQ(EXTI4_15_IRQn);
	return temp;
}

void EXTI4_15_IRQHandler()
{
	uint32_t itemp = EXTI->PR;
	EXTI->PR |= EXTI_PR_PIF6 | EXTI_PR_PIF7 ;
	uint32_t gptemp = GPIOA->IDR & GPIO_IDR_ID8;


	if (!flags){ // only one operation can be queued at a time.
		if (itemp & EXTI_PR_PIF6) {

			flags |= FLAG_BUTTON;

		} else if (itemp & EXTI_PR_PIF7){

			flags |= FLAG_POSITIVE;

		} else if (itemp & EXTI_PR_PIF8){

			flags |= FLAG_NEGATIVE;

		}
	}
}

// this needs to be called every 10ms for the encoder to work correctly.
void RotaryEncoderSystickService(){
#define GPIO_FLAGS (GPIO_IDR_ID8 | GPIO_IDR_ID7 | GPIO_IDR_ID6)
	uint32_t gptemp = GPIOA->IDR & GPIO_FLAGS;

	if ((gptemp == GPIO_FLAGS) && flags){
		clearcount++;
	} else {
		clearcount = 0;
	}

	if (clearcount > 2){
		// the encoder has released for at least 20ms.
		if (flags & FLAG_BUTTON){
			button_pressed = 1;
			flags = 0;
		} else if (flags & FLAG_POSITIVE) {
			encoder_value -= 1;
			flags = 0;
		} else if (flags & FLAG_NEGATIVE) {
			encoder_value += 1;
			flags = 0;
		}
		clearcount = 0;
	}
}
