/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32l0xx.h"
#include "gpio.h"
#include "rcc.h"
#include "uart2.h"
#include "systick.h"

// status flag values.
#define STATUS_HEATING 0x01
#define STATUS_COOLING 0x02
#define STATUS_HUMIDIFYING 0x04
#define STATUS_LIGHTING 0x08
#define STATUS_VENTILATING 0x10

#define MINIMUM_COOL 60000 // the minimum amount of time to cycle the fridge compressor(60000 = 1 min on, 1 min off.)
uint64_t fridge_counter = 0;

void set_cool(int cool){
	if (cool ^ GPIO_READ(A,7)){
		// this requests a change of state
		uint64_t temp = GetSystickMs();
		if (temp - fridge_counter > MINIMUM_COOL)
		{
			if (cool){
				GPIO_SET(A,7);
			}
			else
			{
				GPIO_CLR(A,7);
			}
			fridge_counter = GetSystickMs();
		}
	}
}

void setup(void)
{
	SetClock16MhzInternal();
	ENABLE_GPIO_PORT(A);
	//ENABLE_GPIO_PORT(B);

	SetupSysTick(10);

	// setup Uart
	GPIO_AS_AF(A,3);
	GPIO_SELECT_AF(A,3,4);
	Uart2Init(9600, false, false);

	GPIO_AS_OUTPUT(A,6); //heat
	GPIO_AS_OUTPUT(A,7); //cool
	GPIO_AS_OUTPUT(A,8); //vent
	GPIO_AS_OUTPUT(A,1); //humidity
}

int main(void)
{
	setup();
	uint8_t rxdata;

	while(1){
		DelayMs(100);
		if (Uart2Rx(&rxdata, 1) == 1)
		{
			//operate relays based on received data.
			if (rxdata & STATUS_COOLING){
				GPIO_CLR(A,6);
				set_cool(1);
			}
			else if (rxdata & STATUS_HEATING){
				set_cool(0);
				GPIO_SET(A,6);
			}
			else
			{
				set_cool(0);
				GPIO_CLR(A,6);
			}

			if (rxdata & STATUS_HUMIDIFYING)
			{
				GPIO_SET(A,1);
			}
			else
			{
				GPIO_CLR(A,1);
			}

			if (rxdata & STATUS_VENTILATING)
			{
				GPIO_SET(A,8);
			}
			else
			{
				GPIO_CLR(A,8);
			}
		}
	}
}
