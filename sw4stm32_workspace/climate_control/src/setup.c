/*
 * setup.c
 *
 *  Created on: 8 Feb 2017
 *      Author: mark
 */

#include <stm32l0xx.h>
#include "rcc.h"
#include "spi.h"
#include "gpio.h"
#include "systick.h"
#include "rtc.h"
#include "sht10.h"
#include "oled96.h"
#include "i2c_master.h"

void setup(int firsttime)
{
	SetClock16MhzInternal();
	ENABLE_GPIO_PORT(A);
	ENABLE_GPIO_PORT(B);
	//ENABLE_GPIO_PORT(C);
	SetupSysTick(100);

	EnableLseOscillator(); // to drive RTC
	RtcInit();

	// LIGHT
	GPIO_AS_OUTPUT(A,1);

	setup_sht10();
	read_temp_rh_sensor();

	// setup oled.  I2C must be inititalised separately.
	GPIO_AS_AF(A,9);
	GPIO_AS_AF(A,10);
	GPIO_SELECT_AF(A,9,1);
	GPIO_SELECT_AF(A,10,1);
	SetupI2cMaster(I2C1);
	//OledTestLoop();
	OledInit();
	OledCls();
}