
#include "stm32l0xx.h"
#define FLASH_PEKEY1               ((uint32_t)0x89ABCDEFU)
#define FLASH_PEKEY2               ((uint32_t)0x02030405U)

#include <eeprom.h>
#include <stdint.h> // for uintxx_t
#include <stdlib.h> // for malloc and free
#include <stdbool.h> // for true and false

#if defined(STM32L0)
	#define EEPROM_SIZE 0x400
	#define EEPROM_START_ADDRESS 0x08080000
#endif

// **************************************************************************************************************
// MODIFIED HAL FLASH FUNCTIONS TO TAKE CARE OF LOW LEVEL PROGRAMMING
// **************************************************************************************************************

int _eepromWaitForLastOperation()
{
  uint32_t limit = SystemCoreClock; // HAL waits for 50 seconds, depending on the
  	  	  	  	  	  	  	  	  	  // number of operations in our loop we will wait for around 10 seconds.
  uint32_t i;
  for (i = 0; (i<limit) && (FLASH->SR & FLASH_SR_BSY); i++);

  if (i >= limit)
  {
	  return 2; // timeout.
  }

  /* Check FLASH End of Operation flag  */
  if (FLASH->SR & FLASH_SR_EOP)
  {
    /* Clear FLASH End of Operation pending bit */
	FLASH->SR &= ~FLASH_SR_EOP;
  }

  if ( FLASH->SR & (
		  FLASH_SR_WRPERR | FLASH_SR_PGAERR |
		  FLASH_SR_SIZERR | FLASH_SR_OPTVERR |
		  FLASH_SR_RDERR | FLASH_SR_FWWERR |
		  FLASH_SR_NOTZEROERR ))
  {
    return 1;
  }

  /* If there is no error flag set */
  return 0;

}

static void _eepromUnlock(void)
{
	/* (1) Wait till no operation is on going */
	/* (2) Check if the PELOCK is unlocked */
	/* (3) Perform unlock sequence */
	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
	/* For robust implementation, add here time-out management */
	}
	if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0) /* (2) */
	{
	FLASH->PEKEYR = FLASH_PEKEY1; /* (3) */
	FLASH->PEKEYR = FLASH_PEKEY2;
	}
}

static void _eepromLock(void)
{
	/* (1) Wait till no operation is on going */
	/* (2) Locks the NVM by setting PELOCK in PECR */
	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
	/* For robust implementation, add here time-out management */
	}
	FLASH->PECR |= FLASH_PECR_PELOCK; /* (2) */
}

// **************************************************************************************************************

uint32_t eeprom_read(uint16_t id)
{
	if (id >= EEPROM_SIZE) id = 0; // only allow access within the eeprom range.
	uint32_t address = id * 4 + EEPROM_START_ADDRESS;
	return *(__IO uint32_t *)address;
}

uint32_t eeprom_write(uint16_t id, uint32_t data)
{
	_eepromUnlock();
	if (id >= EEPROM_SIZE) id = 0; // only allow access within the eeprom range.
	uint32_t address = id * 4 + EEPROM_START_ADDRESS;
	*(__IO uint32_t *)address = data;
	uint32_t response = _eepromWaitForLastOperation();
	_eepromLock();
	return response;
}

