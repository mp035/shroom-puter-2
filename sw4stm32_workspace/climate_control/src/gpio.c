#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#endif

void GpioSelectAf(GPIO_TypeDef *GPIO, int pinnumber, int function)
{
	// mask function, just in case.
	function &= 0xf;

	if (pinnumber > 7) // use upper register
	{
		// set up  gpio
		GPIO->AFR[1] &= ~(0xF << ((pinnumber - 8) * 4));
		GPIO->AFR[1] |= (function << ((pinnumber - 8) * 4));
	}
	else
	{
		// set up  gpio
		GPIO->AFR[0] &= ~(0xF << (pinnumber * 4));
		GPIO->AFR[0] |= (function << (pinnumber * 4));
	}
}
