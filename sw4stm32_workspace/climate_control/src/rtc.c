/*
 * rtc.c
 *
 *  Created on: 06/04/2014
 *      Author: mark
 */
#include "main.h"
#include <stdint.h>
#include <stm32l0xx.h>

uint32_t subsecond_counter = 0;
int second = 0;
int minute = 0;
int hour = 0;
int day = 1;
int month = 1;
int year = 14;

static void UnlockRtc(void)
{
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
}

static void MaskInBcd(uint8_t value, uint8_t tensBits, uint8_t destinationShift, uint32_t *destination)
{
	uint8_t tens, ones, tensMask = 0;
	int i;
	for (i=0; i< tensBits; i++)
	{
		tensMask <<= 1;
		tensMask |= 1;
	}
	ones = value % 10;
	tens = value / 10;
	tens = tens & tensMask;

	uint32_t registerMask = ((tensMask << 4) | 0x0F) << destinationShift;
	uint32_t bcdValue = ((tens << 4) | ones) << destinationShift;

	*destination = (*destination) & ~registerMask;
	*destination = (*destination) | bcdValue;
}

static uint8_t ReadBcd(uint8_t tensBits, uint8_t sourceShift, uint32_t sourceValue)
{
	uint8_t tens, ones, tensMask = 0;
	int i;
	for (i=0; i< tensBits; i++)
	{
		tensMask <<= 1;
		tensMask |= 1;
	}

	ones = (sourceValue >> sourceShift) & 0x0F;
	tens = (sourceValue >> (sourceShift + 4)) & tensMask;

	return (tens * 10) + ones;
}

void ReadRtc(void)
{
	uint32_t tr = RTC->TR;
	uint32_t dr = RTC->DR;
	second = ReadBcd(3, RTC_TR_MNU_Pos, tr);
	minute = ReadBcd(3, RTC_TR_MNU_Pos, tr);
	hour = ReadBcd(2, RTC_TR_HU_Pos, tr);

	day = ReadBcd(2, RTC_DR_DU_Pos, dr);
	month = ReadBcd(1, RTC_DR_MU_Pos, dr);
	year = ReadBcd(4, RTC_DR_YU_Pos, dr);
}

void SetRtc(uint8_t hr, uint8_t min, uint8_t sec, uint8_t dayOfWeek, uint8_t date, uint8_t month, uint8_t year)
{
	uint32_t tr = RTC->TR;
	uint32_t dr = RTC->DR;

	UnlockRtc();

	// 1. Set INIT bit to 1 in the RTC_ISR register to enter initialization mode.
	// In this mode, the calendar counter is stopped and its value can be updated.
	RTC->ISR |= RTC_ISR_INIT;

	// 2. Poll INITF bit of in the RTC_ISR register. The initialization phase mode
	// is entered when INITF is set to 1. It takes around 2 RTCCLK clock cycles
	// (due to clock synchronization).
	while(!(RTC->ISR & RTC_ISR_INITF));

	// 3. To generate a 1 Hz clock for the calendar counter, program both the prescaler
	// factors in RTC_PRER register.
	RTC->PRER = 0x007F00FF; // just set them to power on default in case they have been altered.

	// 4. Load the initial time and date values in the shadow registers (RTC_TR and RTC_DR),
	// and configure the time format (12 or 24 hours) through the FMT bit in the RTC_CR register.
	MaskInBcd(hr, 2, RTC_TR_HU_Pos, &tr);
	MaskInBcd(min, 3, RTC_TR_MNU_Pos, &tr);
	MaskInBcd(sec, 3, RTC_TR_SU_Pos, &tr);
	MaskInBcd(dayOfWeek, 0, RTC_DR_WDU_Pos, &dr);
	dr &= ~RTC_DR_WDU;
	dr |= ((uint32_t)dayOfWeek & 0x07U) << RTC_DR_WDU_Pos;
	MaskInBcd(date, 2, RTC_DR_DU_Pos, &dr);
	MaskInBcd(month, 1, RTC_DR_MU_Pos, &dr);
	MaskInBcd(year, 4, RTC_DR_YU_Pos, &dr);

	RTC->DR = dr;
	RTC->TR = tr;


	// for our application, subseconds is irrelevant


	// 5. Exit the initialization mode by clearing the INIT bit. The actual calendar counter value
	// is then automatically loaded and the counting restarts after 4 RTCCLK clock cycles.
	RTC->ISR &= ~RTC_ISR_INIT;

	// lock RTC
	RTC->WPR = 0xFE;
	RTC->WPR = 0x64;


}

void RtcInit(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN; // enable power controller clock
	PWR->CR |= PWR_CR_DBP;// enable access to rtc selection registers

	RCC->CSR |= RCC_CSR_RTCSEL_LSE;

	RCC->CSR |= RCC_CSR_RTCEN;

}
