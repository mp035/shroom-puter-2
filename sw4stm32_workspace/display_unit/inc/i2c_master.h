#ifndef I2C_H
#define I2C_H

#ifdef STM32L0
#include "stm32l0xx.h"
#else
#include "stm32f0xx.h"
#endif


void SetupI2cMaster(I2C_TypeDef *argI2c);
int I2cMasterBlockingRead(uint8_t nbytes, uint16_t address, uint8_t *dataptr);
int I2cMasterBlockingWrite(uint8_t nbytes, uint16_t address, const uint8_t *dataptr);

// for oled displays, writes a buffer with a data/control prefix.
int I2cMasterBlockingWriteWithDC(uint8_t nbytes, uint16_t address, uint8_t dc, const uint8_t *dataptr);

// byte-at-a-time (BAAT) write functions.
void I2CMasterBaatStart(uint8_t nbytes, uint16_t address);
int I2cMasterBaatWrite(uint8_t data);

#endif
