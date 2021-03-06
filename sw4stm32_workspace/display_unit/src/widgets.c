/*
 * widgets.c
 *
 *  Created on: 07/04/2014
 *      Author: mark
 */
#include "main.h"
#include "oled96.h"
#define gotoXY(x,y) OledGotoXY(x,y)
#define DISPLAY_CHAR(x) OledDisplayChar(x)
#define DISPLAY_STRING_WITH_CURSOR(string,cursorpos, cursorlength) OledDisplayStringWithCursor(string,cursorpos, cursorlength)
#define NUMERIC_OFFSET OLED_NUMERIC_OFFSET

#include "print.h"
#include "widgets.h"

// integer power-of function for
// calculating decimal positions.
uint32_t ipow(uint32_t base, uint32_t exp)
{
    uint32_t result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

void draw_widget(widget_t* widget)
{

    int truecursorpos; // used only once in each case to calculate the displayed cursor position.
	unsigned int divisor; // used only in NUMCTRL
    int strpos; //used only in NUMCTRL

	// a 128 pixel screen can only display 14 8*16 characters,so we assume a digit limit of 15 chars plus null.
    char valstr[16]; // used only once in each case to store the string to be displayed.

    switch(widget->widget_type)
    {
    case TIMECTRL:
        gotoXY(widget->xpos, widget->ypos);
        // print the characters
        valstr[0] = (widget->numeric_value / 1000 % 10) + NUMERIC_OFFSET;
        valstr[1] = (widget->numeric_value / 100 % 10) + NUMERIC_OFFSET;
        valstr[2] = ':';
        valstr[3] = (widget->numeric_value / 10 % 10) + NUMERIC_OFFSET;
        valstr[4] = (widget->numeric_value % 10) + NUMERIC_OFFSET;
        valstr[5] = 0; // null terminator

        // timectrl only has cursor position 0 or 1
        if (widget->cursor_position == 1) truecursorpos = 3;
        else if (widget->cursor_position == 0) truecursorpos = widget->cursor_position;
        else truecursorpos = -2;

        DISPLAY_STRING_WITH_CURSOR(valstr, truecursorpos, 2);

        // note that we don't refresh (if required), refresh should be controlled at the application level.
        break;
    case NUMCTRL:
        gotoXY(widget->xpos, widget->ypos);

        /*
        if (widget->decimal_position && (widget->cursor_position >= widget->decimal_position))
        {
        	truecursorpos = widget->cursor_position +1;
        }
		else
		{
			truecursorpos = widget->cursor_position;
		}
		*/

        strpos = 0;
		divisor = ipow(10,widget->digits - 1);
		while (divisor)
		{
			valstr[strpos++] = (widget->numeric_value / divisor % 10) + NUMERIC_OFFSET;
			divisor /= 10;
			if (strpos == widget->decimal_position)
			{
				valstr[strpos++] = '.';
			}
		}
		valstr[strpos] = 0; // null terminator

		if (widget->cursor_position == 0)// only one cursor position for now.
		{
			DISPLAY_STRING_WITH_CURSOR(valstr, 0, strpos);
		}
		else
		{
			DISPLAY_STRING_WITH_CURSOR(valstr, -1, 1);
		}

        break;
    }
}

void pass_encoder_to_widget(int value, int pressed, widget_t* widget)
{

    switch(widget->widget_type)
    {
case TIMECTRL:
        if (value)
        {
            if(widget->cursor_position < 2)
            {
                int multiplier = 100;
                if (widget->cursor_position) multiplier = 1;

				int replacevalue = (widget->numeric_value / 100) * 100;
                if (widget->cursor_position) replacevalue = widget->numeric_value % 100;

                int digit = replacevalue/multiplier;
                digit += value;

                if (widget->cursor_position)
                {
                	if (digit > 59) digit = 59;
                	if (digit < 0) digit = 0;
                }
                else
                {
                	if (digit > 23) digit = 23;
                	if (digit < 0) digit = 0;
                }

                widget->numeric_value -= replacevalue;
                widget->numeric_value += (digit * multiplier);
				widget->return_value = WIDGET_NOTHING; // do nothing as default
            }
            else if (widget->return_value == WIDGET_DRAW_CANCEL)
			{
            	widget->return_value = WIDGET_DRAW_CONFIRM;
			}
            else if (widget->return_value == WIDGET_DRAW_CONFIRM)
			{
            	widget->return_value = WIDGET_DRAW_BACK;
			}
            else
            {
            	widget->return_value = WIDGET_DRAW_CANCEL;
            }
        }
        else if (pressed)
        {
            if(widget->cursor_position < 1)
            {
            	widget->cursor_position++;
				widget->return_value = WIDGET_NOTHING; // do nothing as default
            }
            else if (widget->cursor_position == 1)
            {
            	widget->cursor_position++;
                widget->return_value = WIDGET_DRAW_CANCEL; // cancel pending operation
            }
            else
            {
            	if (widget->return_value == WIDGET_DRAW_CANCEL) widget->return_value = WIDGET_CANCEL;
            	else if (widget->return_value == WIDGET_DRAW_CONFIRM) widget->return_value = WIDGET_CONFIRM;
            	else if (widget->return_value == WIDGET_DRAW_BACK)
            	{
            		widget->return_value = WIDGET_NOTHING;
            		widget->cursor_position = 0;
            	}

            }
        }

        break;
    case NUMCTRL:
    	if (value)
    	{
    		if(widget->cursor_position < 1) // we use a single cursor position for now.
    		{
    			widget->numeric_value += value;
    			if (widget->numeric_value > widget->max_value) widget->numeric_value = widget->max_value;
    			else if (widget->numeric_value < widget->min_value) widget->numeric_value = widget->min_value;
    		}
    		else if (widget->return_value == WIDGET_DRAW_CANCEL)
    		{
    			widget->return_value = WIDGET_DRAW_CONFIRM;
    		}
    		else if (widget->return_value == WIDGET_DRAW_CONFIRM)
    		{
    			widget->return_value = WIDGET_DRAW_BACK;
    		}
    		else
    		{
    			widget->return_value = WIDGET_DRAW_CANCEL;
    		}
    	}
    	else if (pressed)
    	{
    		/*
    		if(widget->cursor_position < 3)
    		{
    			widget->cursor_position++;
    			widget->return_value = WIDGET_NOTHING; // do nothing as default
    		}

    		else */ if (widget->cursor_position == 0)
    		{
    			widget->cursor_position++;
    			widget->return_value = WIDGET_DRAW_CANCEL; // cancel pending operation
    		}
    		else
    		{
    			if (widget->return_value == WIDGET_DRAW_CANCEL) widget->return_value = WIDGET_CANCEL;
    			else if (widget->return_value == WIDGET_DRAW_CONFIRM) widget->return_value = WIDGET_CONFIRM;
    			else if (widget->return_value == WIDGET_DRAW_BACK)
    			{
    				widget->return_value = WIDGET_NOTHING;
    				widget->cursor_position = 0;
    			}

    		}
    	}

    	        break;
    }//swich widget->widget_type
    //draw_widget(widget);
}
