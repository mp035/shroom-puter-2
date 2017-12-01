/*
 * timer.h
 *
 *  Created on: 8 Jul 2016
 *      Author: mark
 */

#ifndef TIMER_H_
#define TIMER_H_

void InitTimer1WithCompareOutput(uint16_t cycletime_ms);
void StopAndResetTimer1(void);
void StartTimer1(void);

void InitTimer14WithCompareOutput(uint16_t cycletime_ms);
void StopAndResetTimer14(void);
void StartTimer14(void);

void InitTimer3ForAdcTrigger(uint16_t cycletime_us);
void StopAndResetTimer3(void);
void StartTimer3(void);
void AdjustTimer3Interval(uint16_t new_interval_us);
uint16_t GetTimer3IntervalUs();


#endif /* TIMER_H_ */
