#ifndef GPIO_H
#define GPIO_H

#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#endif

// provide double indirection on these functions so that defined pin names can be used.
#define PASTE(x, y) x ## y
#define PASTE2(x, y, z) x ## y ## z

// enable GPIO CLocks
#ifdef STM32L0
#define ENABLE_GPIO_PORT(portletter) (RCC->IOPENR |= RCC_IOPENR_GPIO##portletter##EN)
#else
#define ENABLE_GPIO_PORT(portletter) (RCC->AHBENR |= RCC_AHBENR_GPIO##portletter##EN)
#endif

// set a single pin to GP Input
#define GPIO_AS_INPUT(portletter, pinnumber) (PASTE(GPIO,portletter)->MODER &= PASTE(~GPIO_MODER_MODE ,pinnumber))

// set a single pin to GP Output.
#define GPIO_AS_OUTPUT(portletter, pinnumber) GPIO_AS_INPUT(portletter,pinnumber);(PASTE(GPIO,portletter)->MODER |= PASTE2(GPIO_MODER_MODE, pinnumber, _0))

// set a single pin to Open Drain Output.
#ifdef STM32L0
#define GPIO_AS_OPEN_DRAIN(portletter, pinnumber) (PASTE(GPIO,portletter)->OTYPER |= (1 << pinnumber))
#else
#define GPIO_AS_OPEN_DRAIN(portletter, pinnumber) (PASTE(GPIO,portletter)->OTYPER |= PASTE(GPIO_OTYPER_OT, pinnumber))
#endif

// set a single pin to Push/Pull Output.
#define GPIO_AS_PUSH_PULL(portletter, pinnumber) (PASTE(GPIO,portletter)->OTYPER &= ~PASTE(GPIO_OTYPER_OT, pinnumber))

// set a single pin to AF mode
#define GPIO_AS_AF(portletter, pinnumber) GPIO_AS_INPUT(portletter,pinnumber);(PASTE(GPIO,portletter)->MODER |= PASTE2(GPIO_MODER_MODE, pinnumber, _1))

// set a single pin to Analog mode
#define GPIO_AS_ANALOG(portletter, pinnumber) (PASTE(GPIO,portletter)->MODER |= (PASTE2(GPIO_MODER_MODER, pinnumber, _1) | PASTE2(GPIO_MODER_MODER, pinnumber, _0)))

// set or clear the output of a GPIO pin
#define GPIO_SET(portletter, bitnum) (PASTE(GPIO,portletter)->BSRR = PASTE(GPIO_BSRR_BS_,bitnum))
#define GPIO_CLR(portletter, bitnum) (PASTE(GPIO,portletter)->BSRR = PASTE(GPIO_BSRR_BR_,bitnum))

#define GPIO_WRITE(portletter, bitnum, value) 	if(value)\
												{\
													PASTE(GPIO,portletter)->BSRR = PASTE(GPIO_BSRR_BS_,bitnum);\
												}\
												else\
												{\
													PASTE(GPIO,portletter)->BSRR = PASTE(GPIO_BSRR_BR_,bitnum);\
												}

#define GPIO_ENABLE_PULLUP(portletter, pinnumber) (PASTE(GPIO,portletter)->PUPDR |= PASTE2(GPIO_PUPDR_PUPD, pinnumber,_0))
#define GPIO_ENABLE_PULLDOWN(portletter, pinnumber) (PASTE(GPIO,portletter)->PUPDR |= PASTE2(GPIO_PUPDR_PUPD, pinnumber,_1))


// set the alternate function of the GPIO pin via the GPIO_AFH and GPIO_AFL register
void GpioSelectAf(GPIO_TypeDef *GPIO, int pinnumber, int function);

// a macro for the function above, so the API is consistent
#define GPIO_SELECT_AF(portletter, pinnumber, function) GpioSelectAf(GPIO##portletter, pinnumber, function)

#define GPIO_READ(portletter, pinnumber) (PASTE(GPIO, portletter)->IDR & PASTE(GPIO_IDR_ID , pinnumber))

#endif
