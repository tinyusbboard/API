
#ifndef _TINYUSBBOARD_H_85047c6162b94e2382447fe91aefb5e3
#define _TINYUSBBOARD_H_85047c6162b94e2382447fe91aefb5e3 1


#if (defined(ARDUINO) && (ARDUINO >= 100))
#	define Pins_Arduino_h
#	include "pins_arduino.h"
#endif

#include <avr/io.h>
#include "iocomfort.h"


#ifndef BUTTON_PROG
#	define BUTTON_PROG	D,6
#endif

#ifndef LED_B
#	define LED_B		B,0
#endif

#ifndef LED_PWM
#	define LED_PWM		B,1
#endif

#ifndef LED_LEFT
#	define LED_LEFT		D,5
#endif

#ifndef LED_RIGHT
#	define LED_RIGHT	D,3
#endif

#endif
