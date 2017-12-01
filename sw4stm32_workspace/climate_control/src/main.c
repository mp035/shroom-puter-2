/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


//#include <eeprom.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32l0xx.h"
//#include "stm32l0xx_nucleo_32.h"
#include "gpio.h"
#include "widgets.h"
#include "eeprom.h"
#include "sht10.h"
#include "oled96.h"
#include "rtc.h"
#include "keypad.h"
#include "print.h"
#include "systick.h"
#include "sht10.h"
			

static const uint8_t water[34] = {16,16,     0,0,0,0,0,192,240,248,254,240,192,0,0,0,0,0,0,0,0,0,31,63,127,127,127,112,57,31,0,0,0,0,};
static const uint8_t cool[34] = {16,16,     0,72,140,16,96,164,72,126,72,164,96,16,140,72,0,0,1,37,99,17,13,75,36,252,36,75,13,17,99,37,1,0,};
static const uint8_t light_on[34] = {16,16,    0,0,0,112,140,2,33,65,33,65,33,2,140,112,0,0,0,0,0,0,1,2,126,213,212,213,126,2,1,0,0,0,};
static const uint8_t fan[34] = {16,16,    0,0,240,12,198,194,217,93,61,193,194,198,12,240,0,0,0,0,0,131,134,133,137,248,248,137,133,134,131,0,0,0,};
static const uint8_t heat[34] = {16,16,    0,128,130,4,192,240,240,248,251,248,240,240,192,4,130,128,0,0,32,16,1,7,7,15,111,15,7,7,1,16,32,0,};
static const uint8_t light_off[34] = {16,16,    0,0,0,248,204,2,3,129,193,225,115,58,158,254,7,3,0,96,112,56,31,14,127,243,241,240,124,14,3,0,0,0,};
static const uint8_t cross_black[34] = {16,16,0x02, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x40, 0x40, 0xa0, 0x50, 0x28, 0x14, 0x0a, 0x05, 0x02, 0x40, 0xa0, 0x50, 0x28, 0x14, 0x0a, 0x05, 0x02, 0x02, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x40};
static const uint8_t cross_white[34] = {16,16,0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f, 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe, 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe, 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};
static const uint8_t blank[34] = {16,16,    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

#define TEXTLINE(x) OledGotoXY(0, x * 16 )

typedef enum
{
    APPL_DRAW_HOME
    ,APPL_HOME
    ,APPL_DRAW_DOOR_OPEN
    ,APPL_DOOR_OPEN
    ,APPL_DRAW_MENU
    ,APPL_MENU
    ,APPL_DRAW_TIME_SET
    ,APPL_TIME_SET
    ,APPL_DRAW_TEMP_MENU
    ,APPL_TEMP_MENU
    ,APPL_DRAW_TEMP_SET
    ,APPL_TEMP_SET
    ,APPL_DRAW_TEMP_BAND_SET
    ,APPL_TEMP_BAND_SET
    ,APPL_DRAW_RH_MENU
    ,APPL_RH_MENU
    ,APPL_DRAW_HUMIDITY_SET
    ,APPL_HUMIDITY_SET
    ,APPL_DRAW_HUMIDITY_BAND_SET
    ,APPL_HUMIDITY_BAND_SET
    ,APPL_DRAW_LIGHT_MENU
    ,APPL_LIGHT_MENU
    ,APPL_DRAW_LIGHT_ON_SET
    ,APPL_LIGHT_ON_SET
    ,APPL_DRAW_LIGHT_OFF_SET
    ,APPL_LIGHT_OFF_SET
    ,APPL_DRAW_FAN_MENU
    ,APPL_FAN_MENU
    ,APPL_DRAW_FAN_RUN_SET
    ,APPL_FAN_RUN_SET
    ,APPL_DRAW_FAN_OFF_SET
    ,APPL_FAN_OFF_SET
    ,APPL_DRAW_DISABLE_MENU
    ,APPL_DISABLE_MENU
} app_state_e;

int32_t door_open_time_in_seconds;
int16_t temperatureset = 250;
int16_t temperatureband = 10;
int16_t humidityset = 950;
int16_t humidityband = 10;
int16_t startlight = 600;
int16_t stoplight = 1800;
volatile int16_t fanrunminutes = 15;
volatile int16_t fanidleminutes = 2;
uint64_t fancounter = 0;
uint32_t next_screen_update = 0;
uint32_t next_app_gate = 0;

// addresses for persistent variables
#define ADDR_temperatureset 0
#define ADDR_temperatureband 1
#define ADDR_humidityset 2
#define ADDR_humidityband 3
#define ADDR_startlight 4
#define ADDR_stoplight 5
#define ADDR_fanrunminutes 6
#define ADDR_fanidleminutes 7

// some macros which automatically handle the eeprom adresses of the variables
#define AUTO_EE_READ_INIT(variable, default_value) variable = eeprom_read(ADDR_##variable);\
													if (variable == 0) variable = default_value;\

#define AUTO_EE_WRITE(variable) eeprom_write(ADDR_##variable, variable);
#define AUTO_EE_READ(variable) variable = eeprom_read(ADDR_##variable);

// flags to hold state of each aspect
bool disable_humidity = false, disable_light = false, disable_vent = false;
volatile bool disable_temp = false;

// internal flags to hold status for display
uint8_t output_status = 0;
#define STATUS_HEATING 0x01
#define STATUS_COOLING 0x02
#define STATUS_HUMIDIFYING 0x04
#define STATUS_LIGHTING 0x08
#define STATUS_VENTILATING 0x10

//relays to control functions
#define HUMIDITY(x) //GPIO_WRITE(B, 4, x)
#define LIGHTING(x) //GPIO_WRITE(A, 11, x)
#define VENTILATION(x) //GPIO_WRITE(B, 5, x)
#define HEATING(x) peltier_heating = x;
#define COOLING(x) peltier_cooling = x;
/*
#define HEATING(x) heating(x)
void heating(uint16_t x)
{
	GPIO_CLR(B,3);// cooling pos
	GPIO_CLR(A,0);// cooling neg
	if (x)
	{
		GPIO_SET(A,1);// heating pos
		GPIO_SET(A,3);// heating neg
	}
	else
	{
		GPIO_CLR(A,1);// heating pos
		GPIO_CLR(A,3);// heating neg
	}
}

#define COOLING(x) cooling(x)
void cooling(uint16_t x)
{
	GPIO_CLR(A,1);// heating pos
	GPIO_CLR(A,3);// heating neg
	if (x)
	{
		GPIO_SET(B,3);// cooling pos
		GPIO_SET(A,0);// cooling neg
	}
	else
	{
		GPIO_CLR(B,3);// cooling pos
		GPIO_CLR(A,0);// cooling neg
	}
}
*/


void setup_decimal_widget(int16_t numeric_value, widget_t *widget)
{
    widget->widget_type = NUMCTRL;
    widget->xpos = 21;
    widget->ypos = 16;
    widget->digits = 3;
    if (numeric_value > 999) numeric_value = 999;
    widget->numeric_value = numeric_value;
    widget->decimal_position = 2;
    widget->cursor_position = 0;
    widget->return_value = WIDGET_DRAW_CANCEL;
    widget->zero_pad = 1;
}

void setup_3digit_integer_widget(int16_t numeric_value, widget_t *widget)
{
    widget->widget_type = NUMCTRL;
    widget->xpos = 21;
    widget->ypos = 16;
    widget->digits = 3;
    if (numeric_value > 999) numeric_value = 999;
    widget->numeric_value = numeric_value;
    widget->decimal_position = 0;
    widget->cursor_position = 0;
    widget->return_value = WIDGET_DRAW_CANCEL;
    widget->zero_pad = 1;
}

void setup_time_widget(int16_t time_value, widget_t *widget)
{
    widget->widget_type = TIMECTRL;
    widget->xpos = 21;
    widget->ypos = 16;
    if (time_value > 2359) time_value = 2359;
    widget->numeric_value = time_value;
    widget->cursor_position = 0;
    widget->return_value = WIDGET_DRAW_CANCEL;
}

void draw_setting_screen(char* title1, char* title2, widget_t *widget, char *units)
{
    OledCls();
    TEXTLINE(0);
    OledDisplayString(title1);
    int16_t posx,posy;
    if (title2 == 0)
    {
		TEXTLINE(1);
        OledDisplayString("            ");
        posx = 49;
        posy = 16;
        widget->xpos = 24;
        widget->ypos = posy;
    }
    else
    {
        OledDisplayString(title2);
        posx = 49;
        posy = 24;
        widget->xpos = 24;
        widget->ypos = posy;
    }

    OledGotoXY(posx,posy);
    OledDisplayString(units);

    OledGotoXY(0,48);
    if (widget->return_value == WIDGET_DRAW_CANCEL)
    {
        OledDisplayString("  OK  ->CANCEL");
    }
    else if (widget->return_value == WIDGET_DRAW_CONFIRM)
    {
        OledDisplayString("->OK    CANCEL");
    }
    else
    {
        OledDisplayString("              ");
    }

    draw_widget(widget);

    LCDRefresh(0);
}

void handle_settings_keys(uint8_t keyvalue, int16_t* temperature_parameter, uint16_t parameter_eeprom_address, app_state_e *applicaton_state, widget_t * widget, app_state_e draw_previous_screen_state, app_state_e draw_screen_state)
{
    if (keyvalue) // 100ms debounce
    {
        pass_key_to_widget(keyvalue, widget);
        switch(widget->return_value)
        {
        case WIDGET_CANCEL:
            *applicaton_state = draw_previous_screen_state;
            break;
        case WIDGET_CONFIRM:
            *temperature_parameter = widget->numeric_value;
            *applicaton_state = draw_previous_screen_state;
			eeprom_write(parameter_eeprom_address, *temperature_parameter);
            break;
        default:
            *applicaton_state = draw_screen_state;
        }
    }
}

int32_t rate_of_rise = 0;

int main (void)
{
    /* Initialize IO ports and peripherals */
    setup(1);

	// load eeprom values
	AUTO_EE_READ_INIT(temperatureset, 250);
	AUTO_EE_READ_INIT(temperatureband, 10);
	AUTO_EE_READ_INIT(humidityset, 950);
	AUTO_EE_READ_INIT(humidityband, 10);
	AUTO_EE_READ_INIT(startlight, 600);
	AUTO_EE_READ_INIT(stoplight, 1800);
	AUTO_EE_READ_INIT(fanrunminutes, 15);
	AUTO_EE_READ_INIT(fanidleminutes, 105);

    uint16_t trh_error = 0;
    app_state_e application_state = APPL_DRAW_HOME;

    widget_t numwidget;

	uint8_t keyvalue;
	HUMIDITY(0);
	VENTILATION(0);
	LIGHTING(0);
	HEATING(0);
	COOLING(0);

    while (1)
    {
        if (next_screen_update <= GetSystickMs())
{
        	next_screen_update += 5000;// check references to door_open_time_in_seconds if changing this value.
            trh_error = read_temp_rh_sensor();
            if (application_state == APPL_HOME)
                application_state = APPL_DRAW_HOME; // force home screen refresh if required.
            if (application_state == APPL_DOOR_OPEN)
            {
                door_open_time_in_seconds -= 5; // this should line up with the number of seconds that this loop is called.
                if (door_open_time_in_seconds <= 0)
                {
                    application_state = APPL_DRAW_HOME;
                }

                VENTILATION(0);
                HUMIDITY(0);
                HEATING(0);
                COOLING(0);
                LIGHTING(1);
            }
            else
            {
            	if ((GetSystickMs() - fancounter) < (fanrunminutes * 60000))
            	{
            		if (disable_vent)
            		{
            			output_status &= ~(STATUS_VENTILATING);
            			VENTILATION(0);
            		}
            		else
            		{
            			output_status |= STATUS_VENTILATING;
            			VENTILATION(1);
            		}
            	}
            	else if ((GetSystickMs() - fancounter)
            			< ((fanrunminutes + fanidleminutes) * 60000))
            	{
            		output_status &= ~(STATUS_VENTILATING);
            		VENTILATION(0);
            	}
            	else // fancounter has rolled over
            	{
            		fancounter += (fanrunminutes + fanidleminutes) * 60000;
            		output_status &= ~(STATUS_VENTILATING);
            		VENTILATION(0);
            	}

            	if ((humidity > humidityset) || disable_humidity)
            	{
            		output_status &= ~(STATUS_HUMIDIFYING);
            		HUMIDITY(0);
            	}
            	else if (humidity < (humidityset - humidityband))
            	{
            		output_status |= STATUS_HUMIDIFYING;
            		HUMIDITY(1);
            	}

            	// if the humidity set point is > 90% and humidity is above the setpoint then use the fan to lower humidity
            	if (
            			(humidityset > 900)
						&& (humidity > (humidityset + humidityband))
						&& ! disable_vent
            	)
            	{
            		output_status |= STATUS_VENTILATING;
            		VENTILATION(1);
            	}

            	uint16_t ltemp;
            	ltemp = hour * 100 + minute;
            	if (startlight < stoplight) // we are lighting through the day
            	{
            		if (ltemp < stoplight && ltemp >= startlight && ! disable_light)
            		{
            			output_status |= STATUS_LIGHTING;
            			LIGHTING(1);
            		}
            		else
            		{
            			output_status &= ~(STATUS_LIGHTING);
            			LIGHTING(0);
            		}
            	}
            	else if (startlight > stoplight) // we are lighting through midnight
            	{
            		if (ltemp >= stoplight && ltemp < startlight && ! disable_light)
            		{
            			output_status &= ~(STATUS_LIGHTING);
            			LIGHTING(0);
            		}
            		else
            		{
            			output_status |= STATUS_LIGHTING;
            			LIGHTING(1);
            		}
            	}
            	else // startlight == stoplight
            	{
            		output_status &= ~(STATUS_LIGHTING);
            		LIGHTING(0);
            	}

            	// Temperature control algorithms.

            	static int32_t last_temp = 255;// set last temp at the most common setpoint to start with.
            	//static int32_t rate_of_rise; // the number of degrees rise over 5 minutes
            	last_temp -= temperature;
            	rate_of_rise -= (rate_of_rise / 60); // remove one average reading
            	rate_of_rise += last_temp; // add this reading to the pool.

            	if (disable_temp)
            	{
            		output_status &= ~(STATUS_HEATING | STATUS_COOLING);
            	}
            	else
            	{
            		if (last_temp > 0) // temperature is falling
            		{
            			// if the temperature is falling and we are below the set point, then raise the temperature duty
            			if(temperature < temperatureset){
            				output_status |= STATUS_HEATING;
            				output_status &= ~(STATUS_COOLING);
            			}

            		}
            		else if (last_temp < 0) // temperature is rising
            		{
            			// if the temperature is rising and we are above the set point, then lower the temperature duty
            			if(temperature > temperatureset)
            			{
            				output_status |= STATUS_COOLING;
            				output_status &= ~(STATUS_HEATING);
            			}
            		}
            		else // temperature is stable
            		{
            			// when the temperature is stable, only adjust duty if we are outside band

            			if (temperature > (temperatureset + temperatureband))
            			{
            				output_status |= STATUS_COOLING;
            				output_status &= ~(STATUS_HEATING);
            			}
            			else if (temperature < (temperatureset - temperatureband))
            			{
            				output_status |= STATUS_HEATING;
            				output_status &= ~(STATUS_COOLING);
            			}
            			else
            			{
            				//stable temperature within band, do not adjust duty
            				output_status &= ~(STATUS_COOLING);
            				output_status &= ~(STATUS_HEATING);
            			}

            		}
            	}

            	last_temp = temperature;


            	if (output_status & STATUS_HEATING)
            	{
            		COOLING(0);
            		HEATING(1);
            	}
            	else if (output_status & STATUS_COOLING)
            	{
            		HEATING(0);
            		COOLING(1);
            	}
            	else
            	{
            		COOLING(0);
            		HEATING(0);
            	}
            }
        }

        /* wait until delay time has elapsed */
        keyvalue = scan_for_keycode();
        while (next_app_gate > GetSystickMs())
        {
            // ******************************* IDLE LOOP *********************************
            if (keyvalue == 0)
                keyvalue = scan_for_keycode(); // keep looking for keypresses while idle, but only if none have already been registered.
            LCDService();
        }
        next_app_gate += 125;
        ReadRtc(); // update time values

        // ************************ THIS IS THE MAIN STATE MACHINE FOR THE MENU/DISPLAY SYSTEM ***********************
        uint16_t temptime;
        switch (application_state)
        {
        case APPL_DRAW_HOME:
            if (trh_error)
            {
            	TEXTLINE(1);
                OledDisplayString("    SENSOR    ");
            	TEXTLINE(2);
                OledDisplayString("    ERROR     ");
            	TEXTLINE(3);
                OledDisplayString("              ");
            }
            else
            {
                // alternate between peltier duty and measured temp.
                // screen refreshes every 5 seconds, so work with that for now.
                TEXTLINE(1);
                OledDisplayString("TEMP:");
                printshort(temperature / 10, 3, 0);
                OledDisplayChar('.');
                printshort(temperature % 10, 1, 1);
                OledDisplayChar(126);
                OledDisplayChar('C');
                OledDisplayChar(' ');
                OledDisplayChar(' ');

                /*
                TEXTLINE(2);
                OledDisplayString("DUTY:");
                OledDisplayChar(' ');
                if (peltier_duty_setting < 0)
                {
                	OledDisplayChar('-');
                }
                else
                {
                	OledDisplayChar(' ');
                }
                printshort(abs(peltier_duty_setting), 3, 0);
                OledDisplayChar('%');
                OledDisplayChar(' ');
                OledDisplayChar(' ');
                OledDisplayChar(' ');
                */

                TEXTLINE(2);
                OledDisplayString(" R/H:");
                printshort(humidity / 10, 3, 0);
                OledDisplayChar('.');
                printshort(humidity % 10, 1, 1);
                OledDisplayChar('%');
                OledDisplayChar(' ');
                OledDisplayChar(' ');
                OledDisplayChar(' ');

                TEXTLINE(3);
                OledDisplayString("TIME: ");
                printshort(hour, 2, 1);
                OledDisplayChar(':');
                printshort(minute, 2, 1);

            }

            static uint8_t counter = 255;
            counter++;

			#define ICON_Y 0

            if (disable_humidity)
            {
            	OledOverlayBitmaps(water, cross_white, cross_black, 6, ICON_Y);
            }
            else if (output_status & STATUS_HUMIDIFYING) OledDisplayBitmap(water, 6, ICON_Y);
            else OledDisplayBitmap(blank, 6, ICON_Y);

            if (disable_temp)
            {
            	if (counter & 1)
					OledOverlayBitmaps(cool, cross_white, cross_black, 25, ICON_Y);
            	else
					OledOverlayBitmaps(heat, cross_white, cross_black, 25, ICON_Y);

            }
            else if (output_status & STATUS_COOLING) OledDisplayBitmap(cool, 25, ICON_Y);
            else if (output_status & STATUS_HEATING) OledDisplayBitmap(heat, 25, ICON_Y);
            else OledDisplayBitmap(blank, 25, ICON_Y);

            if (disable_light)
            {
            	OledOverlayBitmaps(light_on, cross_white, cross_black, 44, ICON_Y);
            }
            else if (output_status & STATUS_LIGHTING)OledDisplayBitmap(light_on, 44, ICON_Y);
            else OledDisplayBitmap(blank, 44, ICON_Y);

            if (disable_vent)
            {
            	OledOverlayBitmaps(fan, cross_white, cross_black, 63, ICON_Y);
            }
            else if (output_status & STATUS_VENTILATING) OledDisplayBitmap(fan, 63, ICON_Y);
            else OledDisplayBitmap(blank, 63, ICON_Y);

            LCDRefresh(0);
            application_state = APPL_HOME;
            break;
        case APPL_HOME:
            if (keyvalue) // 100ms debounce
            {
                if (keyvalue == '*')
                {
                    application_state = APPL_DRAW_DOOR_OPEN;
                }
                else if (keyvalue == '#')
                {
                	application_state = APPL_DRAW_DISABLE_MENU;
                }
                else
                {
                    application_state = APPL_DRAW_MENU;
                }
            }

            break;
        case APPL_DRAW_DOOR_OPEN:
            OledCls();
            TEXTLINE(0);
            OledDisplayString("  DOOR OPEN   ");
            TEXTLINE(1);
            OledDisplayString(" MODE ACTIVE. ");
            TEXTLINE(2);
            OledDisplayString("PRESS KNOB FOR");
            TEXTLINE(3);
            OledDisplayString(" NORMAL MODE  ");

            LCDRefresh(0);
            door_open_time_in_seconds = (15 * 60); // the door open state will automatically abort after 15 minutes
            application_state = APPL_DOOR_OPEN;
            break;
        case APPL_DOOR_OPEN:
            if (keyvalue)
            // 100ms debounce
            {
                application_state = APPL_DRAW_HOME;
            }
            break;
        case APPL_DRAW_MENU:
            OledCls();
            TEXTLINE(0);
            OledDisplayString("1-SET TIME    ");
            TEXTLINE(1);
            OledDisplayString("2-SET TEMP    ");
            TEXTLINE(2);
            OledDisplayString("3-SET R/H     ");
            TEXTLINE(3);
            OledDisplayString("4-SET LIGHT   ");
            //OledDisplayString("5-SET FAN     ");
            //OledDisplayString("*-EXIT MENU   ");

            //OledCls();
            //OledDisplayString("1-Light On  ");
            //OledDisplayString("2-Set Time  ");
            //OledDisplayString("3-Set Temp  ");
            //OledDisplayString("4-Set R/H   ");
            //OledDisplayString("5-Set Light ");
            //OledDisplayString("6-Set Fan   ");
            LCDRefresh(0);
            application_state = APPL_MENU;
            break;
        case APPL_MENU:
            switch (keyvalue)
            // 100ms debounce
            {
            case '1': // SET TIME
                setup_time_widget((hour * 100 + minute), &numwidget);
                application_state = APPL_DRAW_TIME_SET;
                break;
            case '2': // SET TEMP
                application_state = APPL_DRAW_TEMP_MENU;
                break;
            case '3': // SET RH
                application_state = APPL_DRAW_RH_MENU;
                break;
            case '4': // SET LIGHT
                application_state = APPL_DRAW_LIGHT_MENU;
                break;
            case '5': // SET FAN
                application_state = APPL_DRAW_FAN_MENU;
                break;
            case '*': // EXIT MENU
                application_state = APPL_DRAW_HOME;
                break;
            }
            break;
        case APPL_DRAW_TIME_SET:
            draw_setting_screen(" CURRENT TIME ", " (24HR TIME)  ", &numwidget,
                    "");
            application_state = APPL_TIME_SET;
            break;
        case APPL_TIME_SET:
            // the setting of the time is a special case because we do not have a single variable to hold the time value, and
            // interrupts must be disabled to change the time value outside the isr, so we use a temporary variable and transfer
            // the values if it changes.

            temptime = 0xFFFF;
            handle_settings_keys(keyvalue, (int16_t*)&temptime, 0xFFFF, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_TIME_SET);
            if (temptime != 0xFFFF)
            {
				// FIXME: We need to set the time on STM32
            	//uint16_t temp;
				//temp = SRbits.IPL;
				//SRbits.IPL = 0b111;
            	SetRtc(temptime/100, temptime%100, 0, 1,1,1,0);
                //hour = temptime / 100;
                //minute = temptime % 100;
                //second = 0;

				//SRbits.IPL = temp;

            }
            break;
        case APPL_DRAW_TEMP_MENU:
            OledCls();
            TEXTLINE(1);
            OledDisplayString("TEMP SET POINT");
            TEXTLINE(2);
            OledDisplayString("TEMP BAND     ");
            TEXTLINE(3);
            OledDisplayString("<- EXIT       ");
            LCDRefresh(0);
            application_state = APPL_TEMP_MENU;
            break;

        case APPL_TEMP_MENU:
            switch (keyvalue)
            // 100ms debounce
            {
            case '1': // TEMPERATURE SETPOINT
                setup_decimal_widget(temperatureset, &numwidget);
                application_state = APPL_DRAW_TEMP_SET;
                break;
            case '2': // TEMPERATURE BAND
                setup_decimal_widget(temperatureband, &numwidget);
                application_state = APPL_DRAW_TEMP_BAND_SET;
                break;
            case '#': // LAST MENU
                application_state = APPL_DRAW_MENU;
                break;
            case '*': // EXIT MENU
                application_state = APPL_DRAW_HOME;
                break;
            }
            break;
        case APPL_DRAW_TEMP_SET:
            draw_setting_screen(" TEMPERATURE  ", 0, &numwidget, "\x80" "C");
            application_state = APPL_TEMP_SET;
            break;
        case APPL_TEMP_SET:
            handle_settings_keys(keyvalue, &temperatureset, ADDR_temperatureset, &application_state,
                    &numwidget, APPL_DRAW_TEMP_MENU, APPL_DRAW_TEMP_SET);
            break;
        case APPL_DRAW_TEMP_BAND_SET:
            draw_setting_screen(" TEMPERATURE  ", "     BAND     " , &numwidget, "\x80" "C");
            application_state = APPL_TEMP_BAND_SET;
            break;
        case APPL_TEMP_BAND_SET:
            handle_settings_keys(keyvalue, &temperatureband, ADDR_temperatureband, &application_state,
                    &numwidget, APPL_DRAW_TEMP_MENU, APPL_DRAW_TEMP_BAND_SET);
            break;
        case APPL_DRAW_RH_MENU:
            OledCls();
            OledDisplayString("1-HUMIDITY    ");
            OledDisplayString("  SET POINT   ");
            OledDisplayString("2-HUMIDITY    ");
            OledDisplayString("  BAND        ");
            OledDisplayString("#-LAST MENU   ");
            OledDisplayString("*-EXIT MENU   ");
            LCDRefresh(0);
            application_state = APPL_RH_MENU;
            break;
        case APPL_RH_MENU:
            switch (keyvalue)
            // 100ms debounce
            {
            case '1': // HUMIDITY SETPOINT
                setup_decimal_widget(humidityset, &numwidget);
                application_state = APPL_DRAW_HUMIDITY_SET;
                break;
            case '2': //HUMIDITY BAND
                setup_decimal_widget(humidityband, &numwidget);
                application_state = APPL_DRAW_HUMIDITY_BAND_SET;
                break;
            case '#': // LAST MENU
                application_state = APPL_DRAW_MENU;
                break;
            case '*': // EXIT MENU
                application_state = APPL_DRAW_HOME;
                break;
            }
            break;
        case APPL_DRAW_HUMIDITY_SET:
            draw_setting_screen(" HUMIDITY SET ", 0, &numwidget, "%");
            application_state = APPL_HUMIDITY_SET;
            break;
        case APPL_HUMIDITY_SET:
            handle_settings_keys(keyvalue, &humidityset, ADDR_humidityset, &application_state,
                    &numwidget, APPL_DRAW_RH_MENU, APPL_DRAW_HUMIDITY_SET);
            break;
        case APPL_DRAW_HUMIDITY_BAND_SET:
            draw_setting_screen("HUMIDITY BAND ", 0, &numwidget,"%");
            application_state = APPL_HUMIDITY_BAND_SET;
            break;
        case APPL_HUMIDITY_BAND_SET:
            handle_settings_keys(keyvalue, &humidityband, ADDR_humidityband, &application_state, &numwidget, APPL_DRAW_RH_MENU, APPL_DRAW_HUMIDITY_BAND_SET);
            break;
        case APPL_DRAW_LIGHT_MENU:
            OledCls();
            OledDisplayString("1-LIGHT ON    ");
            OledDisplayString("              ");
            OledDisplayString("2-LIGHT OFF   ");
            OledDisplayString("              ");
            OledDisplayString("#-LAST MENU   ");
            OledDisplayString("*-EXIT MENU   ");
            LCDRefresh(0);
            application_state = APPL_LIGHT_MENU;
            break;
        case APPL_LIGHT_MENU:
            switch (keyvalue)
            // 100ms debounce
            {
            case '1': // LIGHT_ON
                setup_time_widget(startlight, &numwidget);
                application_state = APPL_DRAW_LIGHT_ON_SET;
                break;
            case '2': // LIGHT OFF
                setup_time_widget(stoplight, &numwidget);
                application_state = APPL_DRAW_LIGHT_OFF_SET;
                break;
            case '#': // LAST MENU
                application_state = APPL_DRAW_MENU;
                break;
            case '*': // EXIT MENU
                application_state = APPL_DRAW_HOME;
                break;
            }
            break;
        case APPL_DRAW_LIGHT_ON_SET:
            draw_setting_screen("   LIGHT ON   ", " (24HR TIME)  ", &numwidget,
                    "");
            application_state = APPL_LIGHT_ON_SET;
            break;
        case APPL_LIGHT_ON_SET:
            handle_settings_keys(keyvalue, &startlight, ADDR_startlight, &application_state,
                    &numwidget, APPL_DRAW_LIGHT_MENU, APPL_DRAW_LIGHT_ON_SET);
            break;
        case APPL_DRAW_LIGHT_OFF_SET:
            draw_setting_screen("  LIGHT OFF   ", " (24HR TIME)  ", &numwidget,
                    "");
            application_state = APPL_LIGHT_OFF_SET;
            break;
        case APPL_LIGHT_OFF_SET:
            handle_settings_keys(keyvalue, &stoplight, ADDR_stoplight, &application_state,
                    &numwidget, APPL_DRAW_LIGHT_MENU, APPL_DRAW_LIGHT_OFF_SET);
            break;
        case APPL_DRAW_FAN_MENU:
            OledCls();
            OledDisplayString("1-FAN RUN     ");
            OledDisplayString("  PERIOD      ");
            OledDisplayString("2-FAN IDLE    ");
            OledDisplayString("  PERIOD      ");
            OledDisplayString("#-LAST MENU   ");
            OledDisplayString("*-EXIT MENU   ");
            LCDRefresh(0);
            application_state = APPL_FAN_MENU;
            break;
        case APPL_FAN_MENU:
            switch (keyvalue)
            // 100ms debounce
            {
            case '1': // FAN RUN
                setup_3digit_integer_widget(fanrunminutes, &numwidget);
                application_state = APPL_DRAW_FAN_RUN_SET;
                break;
            case '2': //FAN OFF
                setup_3digit_integer_widget(fanidleminutes, &numwidget);
                application_state = APPL_DRAW_FAN_OFF_SET;
                break;
            case '#': // LAST MENU
                application_state = APPL_DRAW_MENU;
                break;
            case '*': // EXIT MENU
                application_state = APPL_DRAW_HOME;
                break;
            }
            break;
        case APPL_DRAW_FAN_RUN_SET:
            draw_setting_screen(" FAN RUN TIME ", " (IN MINUTES) ", &numwidget,
                    "min");
            application_state = APPL_FAN_RUN_SET;
            break;
        case APPL_FAN_RUN_SET:
            handle_settings_keys(keyvalue, &fanrunminutes, ADDR_fanrunminutes, &application_state,
                    &numwidget, APPL_DRAW_FAN_MENU, APPL_DRAW_FAN_RUN_SET);
            break;
        case APPL_DRAW_FAN_OFF_SET:
            draw_setting_screen("FAN IDLE TIME ", " (IN MINUTES) ", &numwidget,
                    "min");
            application_state = APPL_FAN_OFF_SET;
            break;
        case APPL_FAN_OFF_SET:
            handle_settings_keys(keyvalue, &fanidleminutes, ADDR_fanidleminutes, &application_state,
                    &numwidget, APPL_DRAW_FAN_MENU, APPL_DRAW_FAN_OFF_SET);
            break;
        case APPL_DRAW_DISABLE_MENU:
			OledCls();
			if (disable_temp && disable_humidity &&  disable_light && disable_vent)
				OledDisplayString("1-ENABLE ALL  ");
			else
				OledDisplayString("1-DISABLE ALL ");

			if (disable_temp)
				OledDisplayString("2-ENABLE TEMP ");
			else
				OledDisplayString("2-DISABLE TEMP");

			if (disable_humidity)
				OledDisplayString("3-ENABLE R/H  ");
			else
				OledDisplayString("3-DISABLE R/H ");

			if (disable_light)
				OledDisplayString("4-ENABLE LIGHT");
			else
				OledDisplayString("4-DISABL LIGHT");

			if (disable_vent)
				OledDisplayString("5-ENABLE FAN  ");
			else
				OledDisplayString("5-DISABLE FAN ");

			OledDisplayString("*-EXIT MENU   ");
			LCDRefresh(0);
			application_state = APPL_DISABLE_MENU;
			break;
		case APPL_DISABLE_MENU:
			switch (keyvalue)
			// 100ms debounce
			{
			case '1': // EN/DIS ALL
				if (disable_temp && disable_humidity &&  disable_light && disable_vent)
				{
					disable_temp = false;
					disable_humidity = false;
					disable_light = false;
					disable_vent = false;
				}
				else
				{
					disable_temp = true;
					disable_humidity = true;
					disable_light = true;
					disable_vent = true;
				}
				application_state = APPL_DRAW_DISABLE_MENU;
				break;
			case '2': // SET TEMP
				disable_temp = ! disable_temp;
				application_state = APPL_DRAW_DISABLE_MENU;
				break;
			case '3': // SET RH
				disable_humidity = ! disable_humidity;
				application_state = APPL_DRAW_DISABLE_MENU;
				break;
			case '4': // SET LIGHT
				disable_light = ! disable_light;
				application_state = APPL_DRAW_DISABLE_MENU;
				break;
			case '5': // SET FAN
				disable_vent = ! disable_vent;
				application_state = APPL_DRAW_DISABLE_MENU;
				break;
			case '*': // EXIT MENU
				application_state = APPL_DRAW_HOME;
				break;
			}
			break;

        } // switch application_state
    } // while (1)
} // int16_t main()
