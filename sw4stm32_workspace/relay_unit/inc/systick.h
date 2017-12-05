#ifndef _SYSTICK_H
#define _SYSTICK_H

#include <stdint.h>


// peltier duty cycle is handled in systick.c
extern volatile int8_t peltier_duty_setting; // negative values for cooling, positive for heating
extern volatile uint8_t peltier_heating;
extern volatile uint8_t peltier_cooling;

// This function sets up the SysTick Timer
// the interrupt interval only affects the resolution and accuracy.  You can get
// 100ms delay resolution & accuracy with 100ms interrupt interval
// smaller values will consume more resources due to increased
// context switching.
void SetupSysTick(uint32_t interrupt_interval_ms);

// This function executes a delay based on the systick timer
void DelayMs(uint32_t delay_ms);

// This function gets a 64 bit value which is the
// number of milliseconds since the system tick has
// been started.
uint64_t GetSystickMs(void);

#endif
