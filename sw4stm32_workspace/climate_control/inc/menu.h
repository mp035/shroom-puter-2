/*
 * menu.h
 *
 *  Created on: 2Dec.,2017
 *      Author: mark
 */

#ifndef MENU_H_
#define MENU_H_

#include "main.h"

struct s_menu;

typedef struct {
	const char* title;
	app_state_e action;
	struct s_menu *submenu;
} menuItem_t;


struct s_menu {
	int numItems;
	menuItem_t *items;
	int currentItem;
};

typedef struct s_menu menu_t;

extern menu_t *currentMenu;

void MenuDraw(menu_t * menu);
void MenuHandleEncoder(menu_t *menu, int value, int press, app_state_e* appState);


#endif /* MENU_H_ */
