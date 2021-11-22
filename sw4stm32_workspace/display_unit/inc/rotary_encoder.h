/*
 * rotary_encoder.h
 *
 *  Created on: 1Dec.,2017
 *      Author: mark
 */

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_


void RotaryEncoderInit();
int RotaryEncoderGetValue();
int RotaryEncoderGetPressed();
int RotaryEncoderHasActivity();
void RotaryEncoderSystickService();

#endif /* ROTARY_ENCODER_H_ */
