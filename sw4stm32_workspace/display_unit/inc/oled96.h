/*
 * oled96.h
 *
 *  Created on: 30 Jul 2015
 *      Author: Mark
 */

#ifndef OLED96_H_
#define OLED96_H_

#include <stdint.h>
#include "main.h"
#include "i2c_master.h"
#include "fonts.h" // for degrees symbol char code
typedef uint16_t uint;
typedef uint8_t uchar;

//#define Write_Address 0x78/*slave addresses with write*/
//#define Read_Address 0x79/*slave addresses with read*/
//#define SLAVE_ADDRESS 0x3C
#define SLAVE_ADDRESS 0x78
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define Start_column	0x00
#define Start_page		0x00
#define	StartLine_set	0x00

#define I2C_TRANSMIT(length, data) I2cMasterBlockingWrite(length, SLAVE_ADDRESS, data)
// the busy test is not required on STM32
#define I2C_NOTREADY() (0)

#define WRITE_I2C_DATA(length, data) I2cMasterBlockingWriteWithDC(length, SLAVE_ADDRESS, 0x40, data)

void OledTestLoop(void);
void OledOverlayBitmaps(const uint8_t base[], const uint8_t white_overlay[], const uint8_t black_overlay[], uint8_t xpos, uint8_t ypos);
void OledDisplayBitmap(const uint8_t bitmap[], uint8_t xpos, uint8_t ypos);
void OledInit(void);
void OledCls();
void OledAdjContrast(int8_t adjustment_amt); // plus or minus amt.
void OledDisplayString(const char *string);
void OledDisplayStringWithCursor(const char *string, int8_t cursorpos, int8_t cursorlength);
void OledDisplayDigit(uint8_t digit_value);
void OledDisplayChar(char character);
void OledGotoXY(uint8_t x, uint8_t y);

#define OLED_NUMERIC_OFFSET (48)

#endif /* OLED96_H_ */
