/*
 * rtc.h
 *
 *  Created on: 06/04/2014
 *      Author: mark
 */

#ifndef RTC_H_
#define RTC_H_

#include <stm32l0xx.h>

extern int second, minute, hour, day, month, year;

void ReadRtc(void);
void SetRtc(uint8_t hr, uint8_t min, uint8_t sec, uint8_t dayOfWeek, uint8_t date, uint8_t month, uint8_t year);
void RtcInit(void);

#endif /* RTC_H_ */
