/*
 * sht10.h
 *
 *  Created on: 8 Feb 2017
 *      Author: mark
 */

#ifndef SHT10_H_
#define SHT10_H_

extern int16_t temperature;
extern int16_t humidity;
int read_temp_rh_sensor();
void setup_sht10(void);

#endif /* SHT10_H_ */
