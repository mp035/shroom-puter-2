/*
 * spi.c
 *
 *  Created on: 8 Feb 2017
 *      Author: mark
 */

#include <stdint.h>
#include <spi.h>
#include <gpio.h>
#include "stm32l0xx.h"

void Spi1Init(void)
{
	// set CS high so the chip is not selected.
	GPIO_SET(A,4);

	// init gpio pins and af registers prior to calling this function.

	// start SPI clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	/* (1) Master selection, BR: Fpclk/256 (due to C27 on the board, SPI_CLK is
	set to the minimum) CPOL and CPHA at zero (rising first edge) */
	SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_MSTR | SPI_CR1_BR_1; /* (1) (Fpclk/8) */
	//SPI1->CR1 = SPI_CR1_MSTR; /* (1) (baud rate set to max (Fpclk/2) */
	/* (2) Slave select output enabled, RXNE IT, 8-bit Rx fifo */
	//SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_RXNEIE | SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; /* (2) */
#ifdef STM32L0
	SPI1->CR2 = SPI_CR2_SSOE; /* (2) with no interrupts */
#else
	SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; /* (2) with no interrupts */
#endif
	/* (3) Enable SPI1 */
	SPI1->CR1 |= SPI_CR1_SPE; /* (3) */

	// on initialisation, make sure the data register is empty.
	volatile uint8_t temp;
#ifdef SPI_SR_RXNE
	while(SPI1->SR & SPI_SR_RXNE)
#else
	while(SPI1->SR & SPI_SR_FRLVL)
#endif
	{
		temp = SPI1->DR;
	}
	(void) temp; // suppress warnings.

}

uint8_t Spi1(uint8_t data)
{
	// mp035: a note about the *(__IO uint8_t *)& magic in this function
	// the purpose of that magic is to ensure the compiler accesses the
	// SPIx->DR register using 8 bit access.  Essentially what we do is
	// dereference the address of the DR register which is cast to an unsigned
	// 8 bit volatile pointer.
	// If we fail to access the register using 8 bit access, then the SPI unit on the F0 series
	// will interpret our write as 2x 8bit transactions (the second transaction is
	// bits 8-15 of the access).

	// wait for tx fifo to be empty
#ifdef SPI_SR_TXE
	while(! (SPI1->SR & SPI_SR_TXE));
#else
	while(SPI1->SR & SPI_SR_FTLVL);
#endif

	*(__IO uint8_t *)&SPI1->DR = data;
	while( SPI1->SR & SPI_SR_BSY ); // wait for transmission to complete.

#ifdef SPI_SR_RXNE
	while(! (SPI1->SR & SPI_SR_RXNE)); // wait for character reception
#else
	while(! (SPI1->SR & SPI_SR_FRLVL)); // wait for character reception
#endif

	return *(__IO uint8_t *)&SPI1->DR;
}

void Spi1Cs(uint8_t enableCs)
{
	if(enableCs)
	{
		GPIO_CLR(A,4);
	}
	else
	{
		GPIO_SET(A,4);
	}
}
