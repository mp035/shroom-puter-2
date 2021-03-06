/*
 * menu.c
 *
 *  Created on: 2Dec.,2017
 *      Author: mark
 */
#include "menu.h"
#include "oled96.h"

const char* blank_line = "              ";

#define NUM_DISPLAY_LINES 4
#define SET_TEXT_LINE(line) OledGotoXY(0, line * 16)
#define DISPLAY_STRING(str) OledDisplayString(str)
#define CLS() OledCls()
menu_t *currentMenu;

void MenuDraw(menu_t * menu)
{
	// handle rollovers to prevent clobbering or bad reads.
	if (menu->currentItem >= menu->numItems){
		menu->currentItem = menu->numItems - 1;
	}
	if (menu->currentItem < 0){
		menu->currentItem = 0;
	}

	int i;
	for(i = 0; i < NUM_DISPLAY_LINES; i++ )
	{
		SET_TEXT_LINE(i);
		if (menu->currentItem + i >= menu->numItems){
			DISPLAY_STRING(blank_line);
		}
		else
		{
			DISPLAY_STRING(menu->items[menu->currentItem + i].title);
		}
	}
}

void MenuHandleEncoder(menu_t *menu, int value, int press, app_state_e* appState)
{
	if (press)
	{
		*appState = menu->items[menu->currentItem].action;
		CLS();
		if (menu->items[menu->currentItem].submenu)
		{
			currentMenu = menu->items[menu->currentItem].submenu;
			MenuDraw(currentMenu);
		}
	}
	else if (value)
	{
		menu->currentItem += value;
		while (menu->currentItem >= menu->numItems){
			menu->currentItem = menu->numItems - 1;
		}

		while (menu->currentItem < 0){
			menu->currentItem = 0;
		}
		MenuDraw(menu);
	}
}

