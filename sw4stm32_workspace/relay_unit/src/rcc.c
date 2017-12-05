#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"


void SetClockPLLInternal(uint32_t pllmask)
{
	RCC->CFGR &= ~RCC_CFGR_SW; // clear clock selection bits, also sets system clock to HSI while we are playing around with the clock;
	RCC->CR &= ~RCC_CR_PLLON; // disable PLL
	RCC->CFGR &= ~RCC_CFGR_PLLMUL; // clear PLLMUL_BITS (also sets pll to X2 mode while we are playing around with it.)
	RCC->CFGR &= ~RCC_CFGR_PLLSRC; // select HSI/2 as PLL source
	RCC->CFGR &= ~RCC_CFGR_PPRE; // set PCLK (APB) prescaler to divide by 1
	RCC->CFGR &= ~RCC_CFGR_HPRE; // set HCKL (AHB) prescaler to divide by 1

	RCC->CFGR &= ~RCC_CFGR_PLLMUL; // clear pll bits
	RCC->CFGR |= pllmask; // wind pll up to required value
	RCC->CR |= RCC_CR_PLLON; // enable PLL

	// wait for PLL to lock to new settings, and HSI to be ready
	while (!((RCC->CR & RCC_CR_PLLRDY) && (RCC->CR & RCC_CR_HSIRDY)));

	RCC->CFGR |= RCC_CFGR_SW_PLL; // set system clock to PLL

	// wait for sytem clock to switch to PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

	// update SystemCoreClock variable
	SystemCoreClockUpdate();

}

// written for STM32F030C6T6, sets up system clock to full speed (48Mhz)
void SetClockFullSpeedInternal()
{
	SetClockPLLInternal(RCC_CFGR_PLLMUL12);
}

// written for STM32F030C6T6, sets up system clock to (12Mhz)
void SetClock12MhzInternal()
{
	SetClockPLLInternal(RCC_CFGR_PLLMUL3);
}

// written for STM32F030C6T6, sets up system clock to (8Mhz)
void SetClock8MhzInternal()
{
	// because we just run off of the 8mhz internal oscillator,
	// and that is power on default, do not do anything.
}


// written for STM32F030F4P6, sets up system clock to run from external crystal with PLL multiplier
void SetClockXtalPll(int pllInDiv, int pllMul)
{
	// first clear required values prior to setting.

	RCC->CFGR &= ~RCC_CFGR_SW; // clear clock selection bits, also sets system clock to HSI while we are playing around with the clock;
	RCC->CR &= ~RCC_CR_PLLON; // disable PLL
	RCC->CFGR &= ~RCC_CFGR_PLLMUL; // clear PLLMUL_BITS (also sets pll to X2 mode while we are playing around with it.)

	RCC->CFGR &= ~RCC_CFGR_PLLSRC; // select HSI/2 as PLL source
	RCC->CFGR &= ~RCC_CFGR_PPRE; // set PCLK (APB) prescaler to divide by 1
	RCC->CFGR &= ~RCC_CFGR_HPRE; // set HCKL (AHB) prescaler to divide by 1

	RCC->CFGR2 &= ~RCC_CFGR2_PREDIV; // clear pll prescale divider

	// now set required values and enable pll

	RCC->CR |= RCC_CR_HSEON; // enable the external oscillator
	while(!(RCC->CR & RCC_CR_HSERDY));

	RCC->CFGR2 |= pllInDiv; // set pll prescale divider
	RCC->CFGR |= pllMul; // set pll multiplier to the required value
	RCC->CFGR |= RCC_CFGR_PLLSRC; // select HSE/PREDIV as PLL source
	RCC->CR |= RCC_CR_PLLON; // enable PLL

	// wait for PLL to lock to new settings, and HSE to be ready
	while (!((RCC->CR & RCC_CR_PLLRDY) && (RCC->CR & RCC_CR_HSERDY)));

	RCC->CFGR |= RCC_CFGR_SW_PLL; // set system clock to PLL

	// wait for sytem clock to switch to PLL
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);


	// update SystemCoreClock variable
	SystemCoreClockUpdate();

}

// written for STM32F030F4P6, sets up system clock to run from external crystal with PLL multiplier
void SetClockXtal()
{
	// ensure HSI oscilator is running
	RCC->CR |= RCC_CR_HSION;
	while(!(RCC->CR & RCC_CR_HSIRDY));

	RCC->CFGR &= ~RCC_CFGR_SW; // clear clock selection bits, also sets system clock to HSI while we are playing around with the clock;
	RCC->CR &= ~RCC_CR_PLLON; // disable PLL
	RCC->CFGR &= ~RCC_CFGR_PLLMUL; // clear PLLMUL_BITS (also sets pll to X2 mode while we are playing around with it.)

	// start HSE oscillator
	RCC->CR |= RCC_CR_HSEON;

	// switch to HSE oscillator
	RCC->CFGR |= RCC_CFGR_SW_HSE;

	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);

	RCC->CFGR &= ~RCC_CFGR_PPRE; // set PCLK (APB) prescaler to divide by 1
	RCC->CFGR &= ~RCC_CFGR_HPRE; // set HCKL (AHB) prescaler to divide by 1

	// disable the HSI oscillator to save power
	RCC->CR &= ~RCC_CR_HSION;

	// update SystemCoreClock variable
	SystemCoreClockUpdate();

}

int GetApbPrescaler(void)
{
    uint32_t ppreVal = RCC->CFGR & RCC_CFGR_PPRE;
	if (! (ppreVal & RCC_CFGR_PPRE_0))
	{
		return 1;
	}

	switch (ppreVal)
	{
	case RCC_CFGR_PPRE_DIV2:
		return 2;
	case RCC_CFGR_PPRE_DIV4:
		return 4;
	case RCC_CFGR_PPRE_DIV8:
		return 8;
	case RCC_CFGR_PPRE_DIV16:
		return 16;
	default:
		return -1;
	}
}

int GetAhbPrescaler(void)
{
	uint32_t hpreVal = RCC->CFGR & RCC_CFGR_HPRE;
	if (! (hpreVal & RCC_CFGR_HPRE_0))
	{
		return 1;
	}

	switch (hpreVal)
	{
	case RCC_CFGR_HPRE_DIV2:
		return 2;
	case RCC_CFGR_HPRE_DIV4:
		return 4;
	case RCC_CFGR_HPRE_DIV8:
		return 8;
	case RCC_CFGR_HPRE_DIV16:
		return 16;
	case RCC_CFGR_HPRE_DIV64:
		return 64;
	case RCC_CFGR_HPRE_DIV128:
		return 128;
	case RCC_CFGR_HPRE_DIV256:
		return 256;
	case RCC_CFGR_HPRE_DIV512:
		return 512;
	default:
		return -1;
	}
}
#endif

void SetClock16MhzInternal()
{
	RCC->CR |= RCC_CR_HSION;
	while (! (RCC->CR & RCC_CR_HSIRDY));
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
	SystemCoreClockUpdate();
}

void EnableLseOscillator()
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN; // enable power controller clock
	PWR->CR |= PWR_CR_DBP;// enable access to rtc selection registers

	RCC->CSR |= RCC_CSR_LSEON;
	while (! (RCC->CSR & RCC_CSR_LSERDY));
}
