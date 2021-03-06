
#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#endif

#include <i2c_master.h>
#include "gpio.h"
#include <stdint.h>


I2C_TypeDef *I2C;

void SetupI2cMaster(I2C_TypeDef *argI2c)
{
	I2C = argI2c;

	// enable i2c clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	I2C->CR1 = 0;// reset value, ensures PE is not set during config.
	I2C->CR2 = I2C_CR2_AUTOEND; // automatically send stop bit after NBYTES

	I2C->OAR1 = 0;// reset value, ensures OA1EN is not set during config.

	// for 16MHz I2CCLK and standard mode 100Khz Master
	I2C->TIMINGR = 0x00503D5A;

	// enable peripheral after configuring
	I2C->CR1 |= I2C_CR1_PE;

}

int I2cMasterBlockingRead(uint8_t nbytes, uint16_t address, uint8_t *dataptr)
{

	I2C->CR2 &= ~(I2C_CR2_SADD|I2C_CR2_NBYTES); // clear any existing address, and numbytes
	I2C->CR2 |= address & I2C_CR2_SADD; // write the new slave address
	I2C->CR2 |= ((uint32_t)nbytes << I2C_CR2_NBYTES_Pos); // set the new number of bytes
	I2C->CR2 |= I2C_CR2_RD_WRN; // set direction as read.
	I2C->CR2 |= I2C_CR2_START; // start the transfer
	int i;
	for(i=0; i< nbytes; i++)
	{
		while(! (I2C->ISR & I2C_ISR_RXNE))
		{
			if (I2C->ISR & I2C_ISR_NACKF)
			{
				return i; // return current count on NACK.
			}
		}
		dataptr[i] = I2C->RXDR;
	}
	return i;
}


int I2cMasterBlockingWrite(uint8_t nbytes, uint16_t address, const uint8_t *dataptr)
{
I2C->CR2 &= ~(I2C_CR2_SADD|I2C_CR2_NBYTES); // clear any existing address, and numbytes
	I2C->CR2 |= address & I2C_CR2_SADD; // write the new slave address
	I2C->CR2 |= ((uint32_t)nbytes << I2C_CR2_NBYTES_Pos); // set the new number of bytes
	I2C->CR2 &= ~I2C_CR2_RD_WRN; // set direction as write.
	I2C->CR2 |= I2C_CR2_START; // start the transfer
	int i;
	for(i=0; i< nbytes; i++)
	{
		while(! (I2C->ISR & I2C_ISR_TXE))
		{
			if (I2C->ISR & I2C_ISR_NACKF)
			{
				return i; // return current count on NACK.
			}
		}
		I2C->TXDR = dataptr[i];
	}
	return i;
}

// for oled displays, writes a buffer with a data/control prefix.
int I2cMasterBlockingWriteWithDC(uint8_t nbytes, uint16_t address, uint8_t dc, const uint8_t *dataptr)
{
	nbytes += 1; // add the dc byte.

	I2C->CR2 &= ~(I2C_CR2_SADD|I2C_CR2_NBYTES); // clear any existing address, and numbytes
	I2C->CR2 |= address & I2C_CR2_SADD; // write the new slave address
	I2C->CR2 |= ((uint32_t)nbytes << I2C_CR2_NBYTES_Pos); // set the new number of bytes
	I2C->CR2 &= ~I2C_CR2_RD_WRN; // set direction as write.
	I2C->CR2 |= I2C_CR2_START; // start the transfer
	int i;
	for(i=0; i< nbytes; i++)
	{
		while(! (I2C->ISR & I2C_ISR_TXE))
		{
			if (I2C->ISR & I2C_ISR_NACKF)
			{
				return i; // return current count on NACK.
			}
		}
		if (i){
			I2C->TXDR = dataptr[i-1];
		}
		else
		{
			I2C->TXDR = dc;
		}
	}
	return i;
}

// for blocking writes byte-at-a-time
void I2CMasterBaatStart(uint8_t nbytes, uint16_t address)
{
	I2C->CR2 &= ~(I2C_CR2_SADD|I2C_CR2_NBYTES); // clear any existing address, and numbytes
	I2C->CR2 |= address & I2C_CR2_SADD; // write the new slave address
	I2C->CR2 |= ((uint32_t)nbytes << I2C_CR2_NBYTES_Pos); // set the new number of bytes
	I2C->CR2 &= ~I2C_CR2_RD_WRN; // set direction as write.
	I2C->CR2 |= I2C_CR2_START; // start the transfer
}

int I2cMasterBaatWrite(uint8_t data)
{
		if (I2C->ISR & I2C_ISR_NACKF) return 2; // return 2 if slave nack'd
		if(! (I2C->ISR & I2C_ISR_TXE)) return 1; // return 1 if not ready to write byte.
		I2C->TXDR = data;
		return 0;
}

