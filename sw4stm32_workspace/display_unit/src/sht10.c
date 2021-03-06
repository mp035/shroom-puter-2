/*
 * sht10.c
 *
 *  Created on: 8 Feb 2017
 *      Author: mark
 */

#include <stdint.h>
#include "gpio.h"

int16_t temperature;
int16_t humidity;

// *** IF YOU CHANGE THESE, YOU ALSO NEED TO CHANGE THE PIN ASSIGNMENTS IN THE SETUP FUNCTION!
#define DTALO GPIO_CLR(A,12)
#define DTAHI GPIO_SET(A,12)
#define CLKLO GPIO_CLR(A,11)
#define CLKHI GPIO_SET(A,11)
#define DTAVAL GPIO_READ(A, 12)

void setup_sht10(void)
{
	// reset temp/rh sensor
		GPIO_AS_OUTPUT(A,11);
		GPIO_AS_OUTPUT(A,12);
		GPIO_AS_OPEN_DRAIN(A,11);
		GPIO_AS_OPEN_DRAIN(A,12);
		GPIO_ENABLE_PULLUP(A,11);
		GPIO_ENABLE_PULLUP(A,12);

		DTAHI;
		CLKLO;
}


#define DLY simple_delay();

void simple_delay(void)
{
	volatile int i;
	for (i=0; i<12; i++);
}

static int send_rh_byte(uint8_t value)
{
	DTAHI; // data should already be high, just reiterate it here.
	CLKLO; // likewise clock should already be low;

	int i;
	for(i=0; i<8; i++)
	{
		if (value & 0x80)
		{
			DTAHI;
		}
		else
		{
			DTALO;
		}
		DLY;
		CLKHI;
		DLY;
		CLKLO;
		DLY;
		value <<= 1;
	}
	// check ack
	int nack = 0;
	DTAHI; // release the data line, ready for input.
	CLKHI;
	DLY
	if (DTAVAL)
	{
		nack = 1;
	}
	CLKLO;
	DLY;
	return nack;

}

static uint8_t read_rh_byte(uint8_t ack)
{
	DTAHI; // this should already be the default state.
	CLKLO; // this should already be the default state.

	uint8_t value = 0;
	int i;
	for(i=0; i<8; i++)
	{
		CLKHI;
		DLY;
		value <<= 1;
		if (DTAVAL)
		{
			value |= 1;
		}
		CLKLO;
		DLY;
	}

	if (ack)
	{
		DTALO;
	}
	CLKHI;
	DLY;
	CLKLO;
	DLY;
	DTAHI;
	return value;
}

static void reset_interface()
{
	DTAHI;
	CLKLO;

	int i;
	for (i=0; i<10; i++)
	{
		DLY;
		CLKHI;
		DLY;
		CLKLO;
	}
	DLY;
}
static int send_command(uint8_t command)
{

	// start transmission
	DTAHI; // data should already be high, just reiterate it here.
	CLKLO; // likewise clock should already be low;

		CLKHI;
		DLY;
		DTALO;
		DLY;
		CLKLO;
		DLY;
		CLKHI;
		DLY;
		DTAHI;
		DLY;
		CLKLO;

		// address
		return send_rh_byte(command);

}

int read_temp_rh_sensor()
{
	reset_interface();

	if (send_command(3)) // 3 for temp, 5 for RH
	{
		return 1;
	}

	DLY;
	volatile int i;
	for (i = 0; i<3000000 && DTAVAL; i++); // wait 1 second for sensor to pull data line low;

	uint8_t byte0, byte1;
	byte1 = read_rh_byte(1);
	byte0 = read_rh_byte(0);

	int32_t dval = (byte1 << 8) + byte0;

	temperature = (-3970 + dval) / 10; // -39.7 + (0.01 * reading) = degrees celsius

	if (send_command(5))
	{
		return 2;
	}

	DLY;
	for (i = 0; i<3000000 && DTAVAL; i++); // wait 1 second for sensor to pull data line low;
	byte1 = read_rh_byte(1);
	byte0 = read_rh_byte(0);
	dval = (byte1 << 8) + byte0;

	float fdval = dval;
	fdval = -2.0468 + 0.0367 * fdval + (-1.5955E-6 * (fdval*fdval));

	humidity = fdval * 10;



	return 0;
}




