/*
 * gsl1680.h
 *
 *  Created on: 27/11/2016
 *      Author: gmtii
 */

/*
Pin outs
the FPC on the touch panel is six pins, pin 1 is to the left pin 6 to the right with the display facing up

pin | function  | Arduino Mega | Arduino Uno
--------------------------------------------
1   | SCL       | SCL(21)      | A5
2   | SDA       | SDA(20)      | A4
3   | VDD (3v3) | 3v3          | 3v3
4   | Wake      | 4            | 4
5   | Int       | 2            | 2
6   | Gnd       | gnd          | gnd
*/

#ifndef RA8875_GSL1680_H_
#define RA8875_GSL1680_H_

#define GSL1680_WAKE_PORT			GPIOE
#define GSL1680_WAKE_PIN			GPIO_Pin_2

#define GSL1680_INT_PORT			GPIOE
#define GSL1680_INT_PIN				GPIO_Pin_3

#define GSL1680_WAKE_SET			GPIO_SetBits(GSL1680_WAKE_PORT, GSL1680_WAKE_PIN)
#define GSL1680_WAKE_RESET			GPIO_ResetBits(GSL1680_WAKE_PORT, GSL1680_WAKE_PIN)

// ts_event[i].x
// ts_event[i].y
// ts_event[i].finger

struct _coord { uint32_t x, y; uint8_t finger; };

struct _ts_event
{
	uint8_t  n_fingers;
	struct _coord coords[5];
};

uint8_t gsl1680_read_data(void);
uint8_t gsl1680_init_chip();
uint8_t gsl1680_test(void);

#endif /* RA8875_GSL1680_H_ */
