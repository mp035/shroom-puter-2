#ifndef RCC_H
#define RCC_H
#include <stdint.h>

#ifdef STM32L0

void SetClock16MhzInternal(void);
void EnableLseOscillator(void);

#else
//************************************************
void SetClockFullSpeedInternal();
// Sets the clock to run on High Speed Internal
// oscillator at maximum chip frequency
//************************************************

//************************************************
void SetClock8MhzInternal();
void SetClock12MhzInternal();
// Sets the clock to run on High Speed Internal
// oscillator at the given frequency
//************************************************
void SetClockXtal();
void SetClockXtalPll(uint32_t pllInDiv, uint32_t pllMul);
//!!IMPORTANT you must define the external crystal value
// in Hz similar to:
// #define HSE_VALUE    ((uint32_t)8000000)
// somewhere in your project headers, otherwise
// the SystemCoreClock functions will be incorrect.
//
// Sets the clock to run on external crystal with PLL.
// The crystal frequency is divided by pllInDiv, and
// then multiplied by pllMul to get the system
// clock frequency.
//
// use one of the following CMSIS constants as
// the PLL input divider(pllInDiv):
//
// RCC_CFGR2_PREDIV_DIV1 < PREDIV input clock not divided
// RCC_CFGR2_PREDIV_DIV2 < PREDIV input clock divided by 2
// RCC_CFGR2_PREDIV_DIV3 < PREDIV input clock divided by 3
// RCC_CFGR2_PREDIV_DIV4 < PREDIV input clock divided by 4
// RCC_CFGR2_PREDIV_DIV5 < PREDIV input clock divided by 5
// RCC_CFGR2_PREDIV_DIV6 < PREDIV input clock divided by 6
// RCC_CFGR2_PREDIV_DIV7 < PREDIV input clock divided by 7
// RCC_CFGR2_PREDIV_DIV8 < PREDIV input clock divided by 8
// RCC_CFGR2_PREDIV_DIV9 < PREDIV input clock divided by 9
// RCC_CFGR2_PREDIV_DIV10 < PREDIV input clock divided by 10
// RCC_CFGR2_PREDIV_DIV11 < PREDIV input clock divided by 11
// RCC_CFGR2_PREDIV_DIV12 < PREDIV input clock divided by 12
// RCC_CFGR2_PREDIV_DIV13 < PREDIV input clock divided by 13
// RCC_CFGR2_PREDIV_DIV14 < PREDIV input clock divided by 14
// RCC_CFGR2_PREDIV_DIV15 < PREDIV input clock divided by 15
// RCC_CFGR2_PREDIV_DIV16 < PREDIV input clock divided by 16
//
// use one of the following CMSIS constants as
// the pll multiplier (pllMul):
//
// RCC_CFGR_PLLMUL2 < PLL input clock*2
// RCC_CFGR_PLLMUL3 < PLL input clock*3
// RCC_CFGR_PLLMUL4 < PLL input clock*4
// RCC_CFGR_PLLMUL5 < PLL input clock*5
// RCC_CFGR_PLLMUL6 < PLL input clock*6
// RCC_CFGR_PLLMUL7 < PLL input clock*7
// RCC_CFGR_PLLMUL8 < PLL input clock*8
// RCC_CFGR_PLLMUL9 < PLL input clock*9
// RCC_CFGR_PLLMUL10 < PLL input clock10
// RCC_CFGR_PLLMUL11 < PLL input clock*11
// RCC_CFGR_PLLMUL12 < PLL input clock*12
// RCC_CFGR_PLLMUL13 < PLL input clock*13
// RCC_CFGR_PLLMUL14 < PLL input clock*14
// RCC_CFGR_PLLMUL15 < PLL input clock*15
// RCC_CFGR_PLLMUL16 < PLL input clock*16
//
//************************************************

// read the RCC register and calculate the AHB prescale value.
int GetAhbPrescaler(void);

// read the RCC register and calculate the APB prescale value.
int GetApbPrescaler(void);

#endif
#endif
