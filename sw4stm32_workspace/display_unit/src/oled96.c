/*
 * oled96.c
 *
 *  Created on: 01/02/2015
 *      Author: Mark
 */

//----------------------------------------------------------------------
//EASTRISING TECHNOLOGY CO,.LTD.//
// Module    : ER-OLED0.96-1  0.96"
// Lanuage   : C51 Code
// Create    : JAVEN
// Date      : 2010-06-18
// Drive IC  : SSD1306
// INTERFACE : I2C
// MCU 		 : AT89LV52
// VDD		 : 3.3V   VBAT: 3.6V
// SA0 connected to VSS. Slave address:White:0x70 Read 0x71
//----------------------------------------------------------------------

#include "main.h"
#include "i2c_master.h"
#include "systick.h"
#include <stdint.h>
#include <string.h> // for memcpy


/*
#define Start i2c_start
#define Stop i2c_stop
#define SentByte i2c_write8
*/

#include "oled96.h"

uchar contrastLevel = 0x8F;

//void SentByte(unsigned char Byte);
//void Check_Ack(void); //Acknowledge
//void Stop(void);
//void Start(void);
//void Send_ACK(void);
//unsigned char ReceiveByte(void);

#include "fonts.h"

const uint8_t null[2] = {0x40, 0x00};

uint8_t oled_col = 0;
uint8_t oled_page = 0;

uint8_t scdata[4];
void send_command(uint8_t command)
{
	while(I2C_NOTREADY()); // wait for any pending transmissions, noting that they use our buffer.
	scdata[0] = 0x80;
	scdata[1] = command;
	I2C_TRANSMIT(2, scdata);
	return;
}

void send_2byte_command(uint8_t command, uint8_t arg)
{
	while(I2C_NOTREADY()); // wait for any pending transmissions, noting that they use our buffer.
	scdata[0] = 0x80;
	scdata[1] = command;
	scdata[2] = 0x80;
	scdata[3] = arg;
	I2C_TRANSMIT(4, scdata);
}

void send_null(){
	while(I2C_NOTREADY()); // wait for any pending transmissions
	I2C_TRANSMIT(2, null);
}

// Set page address 0~15
void set_page(unsigned char add)
{
	send_command(0xb0 | add);
	return;
}

void set_column(unsigned char add)
{
	send_command((0x10 | (add >> 4)));
	send_command((0x0f & add));
	return;
}

void set_contrast_control_register(unsigned char mod)
{
	uint8_t data[4];
	data[0] = 0x80;
	data[1] = 0x81;
	data[2] = 0x80;
	data[3] = mod;
	while(I2C_NOTREADY()); // wait for any pending transmissions
	I2C_TRANSMIT(4,data);
	return;
}


void OledGotoXY(uint8_t x, uint8_t y)
{
    // this sends the current memory position to the given pixel.
    oled_col = x;  // Column.
    oled_page = (y/8);  // Row pixel rounded down to nearest 8
}

void OledDisplayChar(char character)
{
	// this function is hard coded to use font8x16_doslike as the font map.

	set_page(oled_page);
	set_column(oled_col);
	while(I2C_NOTREADY()); // wait for any pending transmissions
	I2C_TRANSMIT(9, font8x16_doslike[(character-32)*2]);
	send_null();// one pixel of space after the char.

	set_page(oled_page+1);
	set_column(oled_col);
	while(I2C_NOTREADY()); // wait for any pending transmissions
	I2C_TRANSMIT(9, font8x16_doslike[(character-32)*2+1]);
	send_null();// one pixel of space after the char.


	// increment the column to make consecutive numbers easier
	oled_col += 9;
}

void OledDisplayDigit(uint8_t digit_value)
{
	OledDisplayChar(digit_value + OLED_NUMERIC_OFFSET);
}


void OledDisplayStringWithCursor(const char *string, int8_t cursorpos, int8_t cursorlength)
{
	set_page(oled_page);
	set_column(oled_col);
	int8_t strpos = 0;

	char *copy = string;
	while (*copy)
	{
			while(I2C_NOTREADY()); // wait for any pending transmissions
			I2C_TRANSMIT(9, font8x16_doslike[((*copy++)-32)*2]);
			send_null();// one pixel of space after the char.
	}

	set_page(oled_page+1);
	set_column(oled_col);
	while (*string)
	{
			while(I2C_NOTREADY()); // wait for any pending transmissions
			// draw the cursor if required
			if ((strpos >= cursorpos) && (strpos < (cursorpos+cursorlength)))
			{
				uint8_t block[9];
				memcpy(block, font8x16_doslike[((*string++)-32)*2 +1], 9);
				int i;
				for(i=1; i<9; i++)
				{
					block[i] |= 0x80;
				}
				I2C_TRANSMIT(9, block);
			}
			else
			{
				I2C_TRANSMIT(9, font8x16_doslike[((*string++)-32)*2 +1]);
			}
			strpos++;
			while(I2C_NOTREADY()); // wait for any pending transmissions
			send_null();// one pixel of space after the char.

			oled_col += 9;
	}

	// take care of any overruns.
	// NOTE: this function does NOT allow printing to roll over the edge of the screen!
	while (oled_col > OLED_WIDTH){
		oled_page += 2; // each character takes 2 pages in height.
		oled_col -= OLED_WIDTH;
	}
}

void OledDisplayString(const char *string)
{
	OledDisplayStringWithCursor(string, -1, -1);
}

/*
void display_contrastLevel(uchar number)
{
	uchar number1, number2, number3;
	number1 = number / 100;
	number2 = number % 100 / 10;
	number3 = number % 100 % 10;
	set_column(Start_column + 0 * 8);
	set_page(Start_page);
	Write_number(num, number1, 0);
	set_column(Start_column + 1 * 8);
	set_page(Start_page);
	Write_number(num, number2, 1);
	set_column(Start_column + 2 * 8);
	set_page(Start_page);
	Write_number(num, number3, 2);

}
*/
void OledAdjContrast(int8_t adjustment_amt) // plus or minus amt.
{
	contrastLevel += adjustment_amt;
	set_contrast_control_register(contrastLevel);
	//display_contrastLevel(contrastLevel);
}

/*
void Start(void)
{
	SDA = 1;
	SCL = 1;
	_nop_();
	SDA = 0;
	_nop_();
	SCL = 0;
}

void Stop(void)
{
	SCL = 0;
	SDA = 0;
	_nop_();
	SCL = 1;
	_nop_();
	SDA = 1;
}

void Check_Ack(void) //Acknowledge
{
	unsigned char ack = 1;
	SDA = 1;
	SCL = 1;
	_nop_();
	while (ack == 1)
	{
		ack = SDA;
	}
	_nop_();
	SCL = 0;
	return;
}

void SentByte(unsigned char Byte)
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		SCL = 0;
		if ((Byte & 0x80) == 0x80)
			SDA = 1;
		else
			SDA = 0;
		SCL = 1;
		_nop_();
		Byte = Byte << 1;
	}
	SCL = 0;
	Check_Ack();

}

unsigned char ReceiveByte(void)
{
	uchar i, rudata = 0;
	SCL = 0;
	SDA = 1;
	for (i = 0; i < 8; i++)
	{
		SCL = 1;
		_nop_();
		if (SDA == 1)
			rudata |= 0x01;
		else
			rudata |= 0x00;
		rudata = rudata << 1;
		SCL = 0;
		_nop_();
	}
	Send_ACK();
	return rudata;
}

void Send_ACK(void)
{
	SCL = 0;
	SDA = 0;
	_nop_();
	SCL = 1;
	SCL = 0;
}

*/


void clear_page(int page)
{
	uint8_t i;
	uint8_t cleardata[] =  {0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	set_page(page);
	set_column(0);
	for (i=0; i<16; i++)
	{
		I2C_TRANSMIT(9, cleardata);
	}
}

void OledCls()
{
	uint8_t j;
	for (j=0; j<8; j++)
	{
		clear_page(j);
	}
}

void OledInit(void)
{

	//Start();
	send_command(0xAE); //--turn off oled panel

	send_2byte_command(0xD9,0x22); // set pre-charge period to it's reset value.
	send_2byte_command(0xDB,0x20); // set vcomh deselect to it's reset value.

	send_2byte_command(0xd5,0x80); //--set display clock divide ratio/oscillator frequency
	send_2byte_command(0xa8, 0x3f); //--set multiplex ratio(to 64)
	send_2byte_command(0x8d,0x14); //--set Charge Pump enable (0x8d, 0x10 is disable)
	send_2byte_command(0xd3, 0x00); //-set display offset (0x00 = not offset) -- mp035 checked this is the POR value.
	send_command(0xB0); //--set start page address to 0 -- mp035 added.
	send_command(0x40); //--set start line address to 0 -- mp035 good.
	send_command(0xA6); // set normal display (0xA7 is inverse display) -- mp035 good.
	send_command(0xA4); //--disable entire display on, A5 turns all pixels on. -- mp035 good
	send_command(0xA1); //--set segment re-map 128 to 0 -- mp035 checked, I think this reverses horizontal addressing for this display.
	send_command(0xC8); //--Set COM Output Scan Direction 64 to 0 -- mp035 checked, but unsure exactly what this does.
	send_2byte_command(0xda, 0x12); //--set com pins hardware configuration -- mp035 - this seems like it would be INCORRECT
	send_2byte_command(0x81, contrastLevel); //--set contrast control register -- mp035 good.
	send_command(0xd9); //--set pre-charge period
	send_command(0xf1);
	send_command(0xdb); //--set vcomh
	send_command(0x40);

	send_2byte_command(0x20, 0x02); // set addressing mode to page addressing.
	send_command(0x00); // set lower nibble of the column start address to 0
	send_command(0x10); // set the upper nibble of the column start address to 0
	send_command(0x2E); // disable any scrolling command which may be in progress.

	send_command(0xAF); //--turn on oled panel
	DelayMs(100); // mp035 required, 100ms as per datasheet.

	//mp035 now clear the panel
	OledCls();

}


// This takes a bitmap and draws it on the LCD starting at the specified position (0,0 at the top left)
// the first 2 bytes in the bitmap are the image width, and height
void OledDisplayBitmap(const uint8_t bitmap[], uint8_t xpos, uint8_t ypos)
{
	uint8_t xsize = bitmap[0];
	uint8_t ysize = bitmap[1]/8;
	const uint8_t* my_array = bitmap + 2;
	OledGotoXY(xpos,ypos);
	int yrow;
	for(yrow = 0; yrow < ysize; yrow++)
	{
		set_page(oled_page + yrow);
		set_column(oled_col);
		while(I2C_NOTREADY()); // wait for any pending transmissions
		WRITE_I2C_DATA(xsize, &my_array[yrow*xsize]);
	}
}

void OledOverlayBitmaps(const uint8_t *base, const uint8_t *white_overlay, const uint8_t *black_overlay, uint8_t xpos, uint8_t ypos)
{
	uint8_t xsize = base[0];
	uint8_t ysize = base[1]/8;
	OledGotoXY(xpos,ypos);
	int xcol;
	int yrow;
	for(yrow = 0; yrow < ysize; yrow++)
	{
		set_page(oled_page + yrow);
		set_column(oled_col);
		for(xcol = 0; xcol<xsize; xcol++)
		{
			uint8_t display_data = base[yrow*xsize + xcol + 2];
			if (white_overlay)
				display_data &= white_overlay[yrow*xsize + xcol + 2];
			if (black_overlay)
				display_data |= black_overlay[yrow*xsize + xcol + 2];
			while(I2C_NOTREADY()); // wait for any pending transmissions
			WRITE_I2C_DATA(1, &display_data);
		}
	}
}

void OledTestLoop(void)
{
	OledInit();

	while (1)
	{
		uint8_t value;

        OledGotoXY(0,0);
		for (value = 0; value<10; value++)
		{
			OledDisplayDigit(value);
			DelayMs(500);
		}
        OledGotoXY(0,0);
        OledDisplayString ("Bender");
        DelayMs(500);
		OledDisplayChar ('.');
        DelayMs(500);
		OledDisplayChar ('.');
        DelayMs(500);
		OledDisplayChar ('.');
        DelayMs(500);
		OledDisplayChar ('.');
        DelayMs(500);
		OledDisplayChar ('.');
        DelayMs(500);
		OledDisplayChar ('.');
        DelayMs(500);

        OledGotoXY(0,16);
        OledDisplayStringWithCursor ("IS GREAT!", 1, 1);

        DelayMs(2000);

        static const uint8_t water[34] = {16,16,     0,0,0,0,0,192,240,248,254,240,192,0,0,0,0,0,0,0,0,0,31,63,127,127,127,112,57,31,0,0,0,0,};
        static const uint8_t cross_black[34] = {16,16,0x02, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x40, 0x40, 0xa0, 0x50, 0x28, 0x14, 0x0a, 0x05, 0x02, 0x40, 0xa0, 0x50, 0x28, 0x14, 0x0a, 0x05, 0x02, 0x02, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x40};
        static const uint8_t cross_white[34] = {16,16,0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f, 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe, 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe, 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};

        OledDisplayBitmap(water, 24,32);
        OledOverlayBitmaps(water, cross_white, cross_black, 67, 32);

        /*
		while(1)
		{
			LPM0;
		}
         */

		while(1)
		{
			// halt here.
		}
		/*
		Display_Picture(pic);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Picture(pic1);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Picture(pic2);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Picture(pic3);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Picture(pic4);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Chess(0x0f);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Chinese(font);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Display_Chinese(font);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		Display_Chinese_Column(font);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa7); //--set Inverse Display
		SentByte(0x00);
		Stop();
		Display_Chinese_Column(font);
		Delay(65000);

		Start();
		SentByte(Write_Address);
		SentByte(0x80);
		SentByte(0xa6); //--set normal display
		SentByte(0x00);
		Stop();
		*/
	}
}

