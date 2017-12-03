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


extern volatile bool disable_temp;

#endif
