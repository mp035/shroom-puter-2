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
#define DISPLAY_STRING_WITH_CURSOR(string,cursorpos) OledDisplayStringWithCursor(string,cursorpos)
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
        valstr[0] = (widget->numeric_value % 10) + NUMERIC_OFFSET;
        valstr[1] = (widget->numeric_value / 10 % 10) + NUMERIC_OFFSET;
        valstr[2] = ':';
        valstr[3] = (widget->numeric_value / 100 % 10) + NUMERIC_OFFSET;
        valstr[4] = (widget->numeric_value / 1000 % 10) + NUMERIC_OFFSET;
        valstr[5] = 0; // null terminator

        if (widget->cursor_position > 1) truecursorpos = widget->cursor_position +1; else truecursorpos = widget->cursor_position;
        DISPLAY_STRING_WITH_CURSOR(valstr, truecursorpos);

        // note that we don't refresh (if required), refresh should be controlled at the application level.
        break;
    case NUMCTRL:
        gotoXY(widget->xpos, widget->ypos);

        if (widget->decimal_position && (widget->cursor_position >= widget->decimal_position))
        {
        	truecursorpos = widget->cursor_position +1;
        }
		else
		{
			truecursorpos = widget->cursor_position;
		}

        strpos = 0;
		divisor = ipow(10,widget->digits);
		while (divisor)
		{
			valstr[strpos++] = (widget->numeric_value / divisor % 10) + NUMERIC_OFFSET;
			divisor /= 10;
			if (strpos == widget->decimal_position)
			{
				valstr[strpos++] = '.';
			}
		}
		valstr[strpos++] = 0; // null terminator

        DISPLAY_STRING_WITH_CURSOR(valstr, truecursorpos);

        break;
    }
}

void pass_key_to_widget(unsigned char keycode, widget_t* widget)
{
    widget->return_value = WIDGET_NOTHING; // do nothing as default

    switch(widget->widget_type)
    {
    case TIMECTRL:
        if (keycode >= '0' && keycode <= '9')
        {
            if(widget->cursor_position < 4)
            {
                int charvalue = keycode - '0';
                int replacevalue = widget->numeric_value;
                int multiplier = 1000;
                unsigned char i;
                for (i=0; i<widget->cursor_position; i++)
                {
                    multiplier/=10;
                }
                replacevalue = replacevalue - (replacevalue % multiplier); // round out values below cursor to 0
                replacevalue = replacevalue % (multiplier * 10); // remove digits above cursor
                widget->numeric_value -= replacevalue;
                widget->numeric_value += (charvalue * multiplier);
                widget->cursor_position += 1;
            }
        }
        else if (keycode == '*')
        {
            if(widget->cursor_position == 0)
            {
                widget->return_value = WIDGET_CANCEL; // cancel pending operation
            }
            else
            {
                widget->cursor_position -= 1;
            }
        }
        else if (keycode == '#')
        {
            if(widget->cursor_position > 3)
            {
                widget->return_value = WIDGET_CONFIRM; // cancel pending operation
            }
            else
            {
                widget->cursor_position += 1;
            }
        }

        if (widget->return_value == WIDGET_NOTHING)
        {
            if (widget->cursor_position > 3)
            {
                widget->return_value = WIDGET_DRAW_CONFIRM;
            }
            else if (widget->cursor_position == 0)
            {
                widget->return_value = WIDGET_DRAW_CANCEL;
            }
        }
        break;
    case NUMCTRL:
        if (keycode >= '0' && keycode <= '9')
        {
            if(widget->cursor_position < widget->digits)
            {
                int charvalue = keycode - '0';
                int replacevalue = widget->numeric_value;
                int multiplier = 1;
                unsigned char i;
                for (i=0; i<(widget->digits - widget->cursor_position - 1); i++)
                {
                    multiplier*=10;
                }
                replacevalue = replacevalue - (replacevalue % multiplier); // round out values below cursor to 0
                replacevalue = replacevalue % (multiplier * 10); // remove digits above cursor
                widget->numeric_value -= replacevalue;
                widget->numeric_value += (charvalue * multiplier);
                widget->cursor_position += 1;
            }
        }
        else if (keycode == '*')
        {
            if(widget->cursor_position == 0)
            {
                widget->return_value = WIDGET_CANCEL; // cancel pending operation
            }
            else
            {
                widget->cursor_position -= 1;
            }
        }
        else if (keycode == '#')
        {
            if(widget->cursor_position >= widget->digits)
            {
                widget->return_value = WIDGET_CONFIRM; // cancel pending operation
            }
            else
            {
                widget->cursor_position += 1;
            }
        }

        if (widget->return_value == WIDGET_NOTHING)
        {
            if (widget->cursor_position >= widget->digits)
            {
                widget->return_value = WIDGET_DRAW_CONFIRM;
            }
            else if (widget->cursor_position == 0)
            {
                widget->return_value = WIDGET_DRAW_CANCEL;
            }
        }
        break;
    }//swich widget->widget_type
    //draw_widget(widget);
}


