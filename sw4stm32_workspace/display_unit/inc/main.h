#ifndef MAIN_H_
#define MAIN_H_

/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

/* TODO Application specific user parameters used in user.c may go here */

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

/* TODO User level functions prototypes (i.e. InitApp) go here */
#include <stdint.h>
#include <stdbool.h>

void setup(int firsttime);         /* I/O and Peripheral Initialization */

#define VERSION 0

// from setup.c
void setup(int firsttime);

// for/from main.c
//#define APP_TIME_IN_S(x) (x*10)
volatile extern int16_t app_gate;
volatile extern int16_t fanrunminutes;
volatile extern int16_t fanidleminutes;

typedef enum
{
    APPL_SETUP_HOME
    ,APPL_DRAW_HOME
    ,APPL_HOME
    ,APPL_DRAW_DOOR_OPEN
    ,APPL_DOOR_OPEN
    ,APPL_DRAW_MENU
    ,APPL_MENU
    ,APPL_SETUP_TIME_SET
    ,APPL_DRAW_TIME_SET
    ,APPL_TIME_SET
    ,APPL_SETUP_TEMP_SET
    ,APPL_DRAW_TEMP_SET
    ,APPL_TEMP_SET
    ,APPL_SETUP_TEMP_BAND_SET
    ,APPL_DRAW_TEMP_BAND_SET
    ,APPL_TEMP_BAND_SET
    ,APPL_SETUP_HUMIDITY_SET
    ,APPL_DRAW_HUMIDITY_SET
    ,APPL_HUMIDITY_SET
    ,APPL_SETUP_HUMIDITY_BAND_SET
    ,APPL_DRAW_HUMIDITY_BAND_SET
    ,APPL_HUMIDITY_BAND_SET
    ,APPL_SETUP_LIGHT_ON_SET
    ,APPL_DRAW_LIGHT_ON_SET
    ,APPL_LIGHT_ON_SET
    ,APPL_SETUP_LIGHT_OFF_SET
    ,APPL_DRAW_LIGHT_OFF_SET
    ,APPL_LIGHT_OFF_SET
    ,APPL_SETUP_FAN_RUN_SET
    ,APPL_DRAW_FAN_RUN_SET
    ,APPL_FAN_RUN_SET
    ,APPL_SETUP_FAN_OFF_SET
    ,APPL_DRAW_FAN_OFF_SET
    ,APPL_FAN_OFF_SET
	,APPL_DISABLE_ALL
	,APPL_DISABLE_TEMP
	,APPL_DISABLE_RH
	,APPL_DISABLE_LIGHT
	,APPL_DISABLE_VENT
} app_state_e;

extern volatile bool disable_temp;

#endif
