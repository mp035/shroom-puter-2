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
#include "print.h"
#include "systick.h"
#include "sht10.h"
#include "rotary_encoder.h"
#include "menu.h"
#include "uart2.h"
			

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



int32_t door_open_time_in_seconds;
int16_t temperatureset = 250;
int16_t temperatureband = 10;
int16_t humidityset = 950;
int16_t humidityband = 10;
int16_t startlight = 600;
int16_t stoplight = 1800;
volatile int16_t fanrunminutes = 15;
volatile int16_t fanidleminutes = 2;
uint64_t fanstart = 0;
uint64_t fanstop = 0;
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
													if (variable == 0 || variable == 0xFFFFFFFF) variable = default_value;\

#define AUTO_EE_WRITE(variable) eeprom_write(ADDR_##variable, variable);
#define AUTO_EE_READ(variable) variable = eeprom_read(ADDR_##variable);

// flags to hold state of each aspect
bool disable_humidity = false, disable_light = false, disable_vent = false;
volatile bool disable_temp = false;

// accumulate the encoder value outside the isr
uint8_t my_enc_value = 0;

// internal flags to hold status for display
uint8_t output_status = 0;
#define STATUS_HEATING 0x01
#define STATUS_COOLING 0x02
#define STATUS_HUMIDIFYING 0x04
#define STATUS_LIGHTING 0x08
#define STATUS_VENTILATING 0x10

//relays to control functions
#define HUMIDITY(x) //GPIO_WRITE(B, 4, x)
#define LIGHTING(x) GPIO_WRITE(A, 1, x)
#define VENTILATION(x) //GPIO_WRITE(B, 5, x)
#define HEATING(x)
#define COOLING(x)
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
    widget->max_value = 999;
    widget->min_value = 0;
    if (numeric_value > 999) numeric_value = 235; // for safety set temperature to 23.5C on error.
    widget->numeric_value = numeric_value;
    widget->decimal_position = 2;
    widget->cursor_position = 0;
    widget->return_value = WIDGET_NOTHING;
    widget->zero_pad = 1;
}

void setup_3digit_integer_widget(int16_t numeric_value, widget_t *widget)
{
    widget->widget_type = NUMCTRL;
    widget->xpos = 21;
    widget->ypos = 16;
    widget->digits = 3;
    widget->max_value = 999;
    widget->min_value = 0;
    if (numeric_value > 999) numeric_value = 999;
    widget->numeric_value = numeric_value;
    widget->decimal_position = 0;
    widget->cursor_position = 0;
    widget->return_value = WIDGET_NOTHING;
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
    widget->return_value = WIDGET_NOTHING;
}

void draw_setting_screen(char* title1, char* title2, widget_t *widget, char *units)
{
    OledCls();
    TEXTLINE(0);
    OledDisplayString(title1);
    int16_t posx,posy;
    TEXTLINE(1);
    if (title2 == 0)
    {
        OledDisplayString("            ");
        posx = 60;
        posy = 16;
        widget->xpos = 24;
        widget->ypos = posy;
    }
    else
    {
        OledDisplayString(title2);
        posx = 60;
        posy = 32;
        widget->xpos = 24;
        widget->ypos = posy;
    }

    OledGotoXY(posx,posy);
    OledDisplayString(units);

    draw_widget(widget);
}

void update_setting_screen(widget_t *widget)
{
    TEXTLINE(3);
    if (widget->return_value == WIDGET_DRAW_CANCEL)
    {
        OledDisplayString("   ->CANCEL   ");
    }
    else if (widget->return_value == WIDGET_DRAW_CONFIRM)
    {
        OledDisplayString("    ->SAVE    ");
    }
    else if (widget->return_value == WIDGET_DRAW_BACK)
    {
        OledDisplayString("    ->BACK    ");
    }
    else
    {
        OledDisplayString("              ");
    }

    draw_widget(widget);
}

void reset_vent_settings(){
	fanstop = GetSystickMs + (fanidleminutes + fanrunminutes) * 60000;
	fanstart = fanstop - fanrunminutes * 60000;
}

void handle_settings_keys(int16_t* temperature_parameter, uint16_t parameter_eeprom_address, app_state_e *applicaton_state, widget_t * widget, app_state_e draw_previous_screen_state, app_state_e draw_screen_state, void (*reset_function)(void))
{
    if (RotaryEncoderHasActivity())
    {
        pass_encoder_to_widget(RotaryEncoderGetValue(), RotaryEncoderGetPressed(), widget);
        switch(widget->return_value)
        {
        case WIDGET_CANCEL:
            *applicaton_state = draw_previous_screen_state;
            break;
        case WIDGET_CONFIRM:
            *temperature_parameter = widget->numeric_value;
            *applicaton_state = draw_previous_screen_state;
			eeprom_write(parameter_eeprom_address, *temperature_parameter);
			if (reset_function) reset_function();
            break;
        default:
            *applicaton_state = draw_screen_state;
        }
    }
}


int main (void)
{
    /* Initialize IO ports and peripherals */
    setup(1);

	// load eeprom values
	AUTO_EE_READ_INIT(temperatureset, 235);
	AUTO_EE_READ_INIT(temperatureband, 10);
	AUTO_EE_READ_INIT(humidityset, 950);
	AUTO_EE_READ_INIT(humidityband, 10);
	AUTO_EE_READ_INIT(startlight, 600);
	AUTO_EE_READ_INIT(stoplight, 1800);
	AUTO_EE_READ_INIT(fanrunminutes, 15);
	AUTO_EE_READ_INIT(fanidleminutes, 105);

    uint16_t trh_error = 0;
    app_state_e application_state = APPL_SETUP_HOME;

    widget_t numwidget;

	HUMIDITY(0);
	VENTILATION(0);
	LIGHTING(0);
	HEATING(0);
	COOLING(0);


	// create menus
	menu_t mainMenu;
	menu_t tempMenu;
	menu_t rhMenu;
	menu_t lightMenu;
	menu_t fanMenu;
	menu_t disableMenu;

	menuItem_t tempMenuItems[4] = {
			{"GO BACK       ", APPL_MENU, &mainMenu},
			{"TEMP SET POINT", APPL_SETUP_TEMP_SET, 0},
			{"TEMP BAND     ", APPL_SETUP_TEMP_BAND_SET, 0},
			{"EXIT MENU     ", APPL_SETUP_HOME, 0}
	};
	tempMenu.currentItem = 0;
	tempMenu.items = tempMenuItems;
	tempMenu.numItems = 4;

	menuItem_t rhMenuItems[] = {
			{"GO BACK       ", APPL_MENU, &mainMenu},
			{"HUMIDITY SET  ", APPL_SETUP_HUMIDITY_SET, 0},
			{"HUMIDITY  BAND", APPL_SETUP_HUMIDITY_BAND_SET, 0},
			{"EXIT MENU     ", APPL_SETUP_HOME, 0}
	};
	rhMenu.currentItem = 0;
	rhMenu.items = rhMenuItems;
	rhMenu.numItems = 4;

	menuItem_t lightMenuItems[] = {
			{"GO BACK       ", APPL_MENU, &mainMenu},
			{"LIGHT ON      ", APPL_SETUP_LIGHT_ON_SET, 0},
			{"LIGHT OFF     ", APPL_SETUP_LIGHT_OFF_SET, 0},
			{"EXIT MENU     ", APPL_SETUP_HOME, 0}
	};
	lightMenu.currentItem = 0;
	lightMenu.items = lightMenuItems;
	lightMenu.numItems = 4;

	menuItem_t fanMenuItems[] = {
			{"GO BACK       ", APPL_MENU, &mainMenu},
			{"VENT RUN TIME ", APPL_SETUP_FAN_RUN_SET, 0},
			{"VENT IDLE TIME", APPL_SETUP_FAN_OFF_SET, 0},
			{"EXIT MENU     ", APPL_SETUP_HOME, 0}
	};
	fanMenu.currentItem = 0;
	fanMenu.items = fanMenuItems;
	fanMenu.numItems = 4;


	menuItem_t mainMenuItems[] = {
			{"EXIT MENU     ", APPL_SETUP_HOME, 0},
			{"INHIBIT ITEMS ", APPL_DRAW_MENU,&disableMenu},
			{"SET TIME      ", APPL_SETUP_TIME_SET, 0},
			{"SET TEMP      ", APPL_MENU, &tempMenu},
			{"SET HUMIDITY  ", APPL_MENU, &rhMenu},
			{"SET LIGHTING  ", APPL_MENU, &lightMenu},
			{"SET VENT      ", APPL_MENU, &fanMenu}
	};
	mainMenu.currentItem = 0;
	mainMenu.items = mainMenuItems;
	mainMenu.numItems = 7;


	const char* enableAll = "ENABLE ALL    ";
	const char* disableAll = "DISABLE ALL   ";

	const char* enableTemp = "ENABLE TEMP   ";
	const char* disableTemp = "DISABLE TEMP  ";

	const char* enableRh = "ENABLE R/H    ";
	const char* disableRh = "DISABLE R/H   ";

	const char* enableLight = "ENABLE LIGHT  ";
	const char* disableLight = "DISABLE LIGHT ";

	const char* enableVent = "ENABLE VENT   ";
	const char* disableVent = "DISABLE VENT  ";

	menuItem_t disableMenuItems[]={
			{"GO BACK       ", APPL_MENU, &mainMenu},
			{disableAll, APPL_DISABLE_ALL,0},
			{disableTemp, APPL_DISABLE_TEMP,0},
			{disableRh, APPL_DISABLE_RH,0},
			{disableLight, APPL_DISABLE_LIGHT,0},
			{disableVent, APPL_DISABLE_VENT,0},
			{"EXIT MENU     ", APPL_SETUP_HOME, 0}
	};

	disableMenu.items = disableMenuItems;
	disableMenu.currentItem = 0;
	disableMenu.numItems = 7;

	menuItem_t *menuItemDisableAll = &disableMenuItems[1];
	menuItem_t *menuItemDisableTemp = &disableMenuItems[2];
	menuItem_t *menuItemDisableRh = &disableMenuItems[3];
	menuItem_t *menuItemDisableLight = &disableMenuItems[4];
	menuItem_t *menuItemDisableVent = &disableMenuItems[5];

#define UPDATE_TIME_SECONDS 5


    while (1)
    {
        if (next_screen_update <= GetSystickMs())
        {
        	next_screen_update += (UPDATE_TIME_SECONDS * 1000);
            trh_error = read_temp_rh_sensor();
            if (application_state == APPL_HOME)
                application_state = APPL_DRAW_HOME; // force home screen refresh if required.
            if (application_state == APPL_DOOR_OPEN)
            {
                door_open_time_in_seconds -= UPDATE_TIME_SECONDS;
                if (door_open_time_in_seconds <= 0)
                {
                    application_state = APPL_SETUP_HOME;
                }

                VENTILATION(0);
                HUMIDITY(0);
                HEATING(0);
                COOLING(0);
                LIGHTING(1);
            }
            else
            {
            	if (GetSystickMs() > fanstop)
            	{
            		// fan counter has rolled over
            			output_status &= ~(STATUS_VENTILATING);
            			VENTILATION(0);
            		fanstop += (fanrunminutes + fanidleminutes) * 60000;
            		fanstart = fanstop - fanrunminutes * 60000;
            	}
            	else if (GetSystickMs() > fanstart)
            	{
            		// ventilation should run continuously
            		// because we have not had enough ventilation
            		// from the humidifier during the cycle
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
            	else
            	{
            		// do not ventilate (the humidifier may control the fan
            		// and move fanstart accordingly
					output_status &= ~(STATUS_VENTILATING);
					VENTILATION(0);

            	}


            	if ((humidity > (humidityset + humidityband)) || disable_humidity)
            	{
            		output_status &= ~(STATUS_HUMIDIFYING);
            		HUMIDITY(0);
            	}
            	else if (humidity < (humidityset - humidityband))
            	{
            		output_status |= STATUS_HUMIDIFYING;
            		HUMIDITY(1);

            	}

            	// run the ventilator whenever we are humidifying
            	if (output_status & STATUS_HUMIDIFYING){
            		if (GetSystickMs() < fanstart){
						output_status |= STATUS_VENTILATING;
						VENTILATION(1);
						fanstart += UPDATE_TIME_SECONDS * 1000;
            		}
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

            	if (disable_temp)
            	{
            		output_status &= ~(STATUS_HEATING | STATUS_COOLING);
            	}
            	else
            	{
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
            			output_status &= ~(STATUS_COOLING);
            			output_status &= ~(STATUS_HEATING);
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
        //keyvalue = scan_for_keycode();
        while (next_app_gate > GetSystickMs())
        {
            // ******************************* IDLE LOOP *********************************
            //if (keyvalue == 0)
            //    keyvalue = scan_for_keycode(); // keep looking for keypresses while idle, but only if none have already been registered.
            //LCDService();
        }
        next_app_gate += 125;
        ReadRtc(); // update time values
        if (Uart2TxFree()) Uart2Tx(&output_status, 1);

        // ************************ THIS IS THE MAIN STATE MACHINE FOR THE MENU/DISPLAY SYSTEM ***********************
        uint16_t temptime;
        switch (application_state)
        {
        case APPL_SETUP_HOME:
        	OledCls();
            application_state = APPL_DRAW_HOME;
        	break;
        case APPL_DRAW_HOME:

        	currentMenu = &mainMenu;

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
                OledDisplayChar(DEGREES_CHAR);
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

            //OledGotoXY(80,0);
            //printchar(my_enc_value, 3, 1);

            //LCDRefresh(0);
            application_state = APPL_HOME;
            break;
        case APPL_HOME:

        	//if (keyvalue){
        	//	my_enc_value += keyvalue;
        	//	keyvalue = 0;
        	//	application_state = APPL_DRAW_HOME;
        	//}

        	if (RotaryEncoderHasActivity())
            {
                if (RotaryEncoderGetValue() != 0)
                {
                    application_state = APPL_DRAW_DOOR_OPEN;
                }
                else if (RotaryEncoderGetPressed())
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
            if (RotaryEncoderHasActivity())
            {
            	RotaryEncoderGetPressed();
            	RotaryEncoderGetValue();
                application_state = APPL_SETUP_HOME;
            }
            break;
        case APPL_DRAW_MENU:
        	MenuDraw(currentMenu);
            application_state = APPL_MENU;
            break;
        case APPL_MENU:
        	if (RotaryEncoderHasActivity())
        	{
        		MenuHandleEncoder(currentMenu,RotaryEncoderGetValue(), RotaryEncoderGetPressed(), &application_state);
        	}
        	break;
        case APPL_SETUP_TIME_SET:
        	setup_time_widget((hour * 100 + minute), &numwidget);
            draw_setting_screen(" CURRENT TIME ", " (24HR TIME)  ", &numwidget,
                    "");
        	application_state = APPL_DRAW_TIME_SET;
        	break;
        case APPL_DRAW_TIME_SET:
        	update_setting_screen(&numwidget);
            application_state = APPL_TIME_SET;
            break;
        case APPL_TIME_SET:
            // the setting of the time is a special case because we do not have a single variable to hold the time value, and
            // interrupts must be disabled to change the time value outside the isr, so we use a temporary variable and transfer
            // the values if it changes.

            temptime = 0xFFFF;
            handle_settings_keys((int16_t*)&temptime, 0xFFFF, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_TIME_SET, 0);
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
		case APPL_SETUP_TEMP_SET:
			setup_decimal_widget(temperatureset, &numwidget);
            draw_setting_screen(" TEMPERATURE  ", 0, &numwidget, DEGREES_STR "C");
			application_state = APPL_DRAW_TEMP_SET;
			break;
        case APPL_DRAW_TEMP_SET:
        	update_setting_screen(&numwidget);
            application_state = APPL_TEMP_SET;
            break;
        case APPL_TEMP_SET:
            handle_settings_keys(&temperatureset, ADDR_temperatureset, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_TEMP_SET, 0);
            break;
        case APPL_SETUP_TEMP_BAND_SET:
        	setup_decimal_widget(temperatureband, &numwidget);
        	application_state = APPL_DRAW_TEMP_BAND_SET;
            draw_setting_screen(" TEMPERATURE  ", "     BAND     " , &numwidget, DEGREES_STR "C");
        	break;
        case APPL_DRAW_TEMP_BAND_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_TEMP_BAND_SET;
            break;
        case APPL_TEMP_BAND_SET:
            handle_settings_keys(&temperatureband, ADDR_temperatureband, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_TEMP_BAND_SET, 0);
            break;
        case APPL_SETUP_HUMIDITY_SET:
                setup_decimal_widget(humidityset, &numwidget);
                draw_setting_screen(" HUMIDITY SET ", 0, &numwidget, "%");
                application_state = APPL_DRAW_HUMIDITY_SET;
        	break;
        case APPL_DRAW_HUMIDITY_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_HUMIDITY_SET;
            break;
        case APPL_HUMIDITY_SET:
            handle_settings_keys(&humidityset, ADDR_humidityset, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_HUMIDITY_SET, 0);
            break;
        case APPL_SETUP_HUMIDITY_BAND_SET:
        	setup_decimal_widget(humidityband, &numwidget);
            draw_setting_screen("HUMIDITY BAND ", 0, &numwidget,"%");
        	application_state = APPL_DRAW_HUMIDITY_BAND_SET;
            break;
        case APPL_DRAW_HUMIDITY_BAND_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_HUMIDITY_BAND_SET;
            break;
        case APPL_HUMIDITY_BAND_SET:
            handle_settings_keys(&humidityband, ADDR_humidityband, &application_state, &numwidget, APPL_DRAW_MENU, APPL_DRAW_HUMIDITY_BAND_SET, 0);
            break;
        case APPL_SETUP_LIGHT_ON_SET:
        	setup_time_widget(startlight, &numwidget);
        	application_state = APPL_DRAW_LIGHT_ON_SET;
        	draw_setting_screen("   LIGHT ON   ", " (24HR TIME)  ", &numwidget, "");
        	break;
        case APPL_DRAW_LIGHT_ON_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_LIGHT_ON_SET;
            break;
        case APPL_LIGHT_ON_SET:
            handle_settings_keys(&startlight, ADDR_startlight, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_LIGHT_ON_SET, 0);
            break;
        case APPL_SETUP_LIGHT_OFF_SET:
        	setup_time_widget(stoplight, &numwidget);
            draw_setting_screen("  LIGHT OFF   ", " (24HR TIME)  ", &numwidget, "");
        	application_state = APPL_DRAW_LIGHT_OFF_SET;
        	break;
        case APPL_DRAW_LIGHT_OFF_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_LIGHT_OFF_SET;
            break;
        case APPL_LIGHT_OFF_SET:
            handle_settings_keys(&stoplight, ADDR_stoplight, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_LIGHT_OFF_SET, 0);
            break;
        case APPL_SETUP_FAN_RUN_SET:
        	setup_3digit_integer_widget(fanrunminutes, &numwidget);
        	draw_setting_screen(" FAN RUN TIME ", " (IN MINUTES) ", &numwidget, "min");
        	application_state = APPL_DRAW_FAN_RUN_SET;
        	break;
        case APPL_DRAW_FAN_RUN_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_FAN_RUN_SET;
            break;
        case APPL_FAN_RUN_SET:
            handle_settings_keys(&fanrunminutes, ADDR_fanrunminutes, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_FAN_RUN_SET, reset_vent_settings);
            break;
        case APPL_SETUP_FAN_OFF_SET:
        	setup_3digit_integer_widget(fanidleminutes, &numwidget);
        	draw_setting_screen("FAN IDLE TIME ", " (IN MINUTES) ", &numwidget, "min");
        	application_state = APPL_DRAW_FAN_OFF_SET;
        	break;
        case APPL_DRAW_FAN_OFF_SET:
            update_setting_screen(&numwidget);
            application_state = APPL_FAN_OFF_SET;
            break;
        case APPL_FAN_OFF_SET:
            handle_settings_keys(&fanidleminutes, ADDR_fanidleminutes, &application_state,
                    &numwidget, APPL_DRAW_MENU, APPL_DRAW_FAN_OFF_SET, reset_vent_settings);
            break;
        case APPL_DISABLE_ALL: // EN/DIS ALL
        	if (disable_temp && disable_humidity &&  disable_light && disable_vent)
        	{
        		disable_temp = false;
        		disable_humidity = false;
        		disable_light = false;
        		disable_vent = false;

        		menuItemDisableAll->title = disableAll;
        		menuItemDisableTemp->title = disableTemp;
        		menuItemDisableRh->title = disableRh;
        		menuItemDisableLight->title = disableLight;
        		menuItemDisableVent->title = disableVent;
        	}
        	else
        	{
        		disable_temp = true;
        		disable_humidity = true;
        		disable_light = true;
        		disable_vent = true;

        		menuItemDisableAll->title = enableAll;
        		menuItemDisableTemp->title = enableTemp;
        		menuItemDisableRh->title = enableRh;
        		menuItemDisableLight->title = enableLight;
        		menuItemDisableVent->title = enableVent;
        	}

        	application_state = APPL_DRAW_MENU;
        	break;
        case APPL_DISABLE_TEMP: // SET TEMP
        	disable_temp = ! disable_temp;
        	if(disable_temp)
        		menuItemDisableTemp->title = enableTemp;
        	else
        		menuItemDisableTemp->title = disableTemp;

        	application_state = APPL_DRAW_MENU;
        	break;
        case APPL_DISABLE_RH: // SET RH
        	disable_humidity = ! disable_humidity;
        	if(disable_humidity)
        		menuItemDisableRh->title = enableRh;
        	else
        		menuItemDisableRh->title = disableRh;

        	application_state = APPL_DRAW_MENU;
        	break;
        case APPL_DISABLE_LIGHT: // SET LIGHT
        	disable_light = ! disable_light;
        	if(disable_light)
        		menuItemDisableLight->title = enableLight;
        	else
        		menuItemDisableLight->title = disableLight;

        	application_state = APPL_DRAW_MENU;
        	break;
        case APPL_DISABLE_VENT: // SET FAN
        	disable_vent = ! disable_vent;
        	if(disable_vent)
        		menuItemDisableVent->title = enableVent;
        	else
        		menuItemDisableVent->title = disableVent;

        	application_state = APPL_DRAW_MENU;
        	break;

        } // switch application_state
    } // while (1)
} // int16_t main()
