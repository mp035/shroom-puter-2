/*
 * widgets.h
 *
 *  Created on: 07/04/2014
 *      Author: mark
 */

#ifndef WIDGETS_H_
#define WIDGETS_H_
typedef enum
{
    NUMCTRL
    ,TIMECTRL
    ,LABEL
    ,BUTTON
} widget_type_e;

typedef enum
{
    WIDGET_NOTHING
    ,WIDGET_CANCEL
    ,WIDGET_CONFIRM
    ,WIDGET_DRAW_CANCEL
    ,WIDGET_DRAW_CONFIRM
} return_instruction_type_e;

typedef struct
{
        // public variables
        unsigned char xpos;
        unsigned char ypos;
        widget_type_e widget_type;
        int32_t numeric_value;
        unsigned char digits;
        unsigned char zero_pad;
        unsigned char decimal_position;
        return_instruction_type_e return_value;

        // private variables
        unsigned char cursor_position;

} widget_t;

void draw_widget(widget_t* widget);
void pass_key_to_widget(unsigned char keycode, widget_t* widget);

#endif /* WIDGETS_H_ */
