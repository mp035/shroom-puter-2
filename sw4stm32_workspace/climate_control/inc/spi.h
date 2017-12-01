/*
 * sph.h
 *
 *  Created on: 8 Feb 2017
 *      Author: mark
 */

#ifndef SPH_H_
#define SPH_H_

#include <stdint.h>

void Spi1Init(void);

// performs a blocking SPI transfer, sending data on MOSI, and returning the result read from MISO
uint8_t Spi1(uint8_t data);

// set argument to 1 to select default device and enable SPI, or 0 to deselect it and disable SPI
void Spi1Cs(uint8_t enableCs);

#endif /* SPH_H_ */
