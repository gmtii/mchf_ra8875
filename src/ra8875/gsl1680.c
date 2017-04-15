/*
// Original comments:
// driver for the GSL1680 touch panel
// Information gleaned from https://github.com/rastersoft/gsl1680.git and various other sources
// firmware for the specific panel was found here:- http://www.buydisplay.com/default/5-inch-tft-lcd-module-800x480-display-w-controller-i2c-serial-spi
// As was some test code.
// This is for that 800X480 display and the 480x272 from buydisplay.com
// ------
// Modified by Helge Langehaug with help from https://forum.pjrc.com/threads/26256-Has-anyone-tried-running-the-GSL16880-capacitive-touchscreen-controller-with-Teensy3

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


#include "radio.h"
#include "gsl1680.h"
#include "gsl1680_firmware.h"
#include "tm_stm32f4_i2c.h"
#include "tm_stm32f4_delay.h"

// Defines

#define SCREEN_MAX_X 		800
#define SCREEN_MAX_Y 		480

#define GSLX680_I2C_ADDR 	0x40<<1

#define GSL_DATA_REG		0x80
#define GSL_STATUS_REG		0xe0
#define GSL_PAGE_REG		0xf0

// 	Variables

struct _ts_event ts_event;

// Functions

static void delay(ulong delay);

// 	Functions

void gsl1680_pin_init()
{

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	// Configure GPIO PIN for GSL1680_WAKE_PIN
	GPIO_InitStructure.GPIO_Pin		= GSL1680_WAKE_PIN;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GSL1680_WAKE_PORT, &GPIO_InitStructure);

	// Configure GPIO PIN for GSL1680_INT_PIN
	GPIO_InitStructure.GPIO_Pin		= GSL1680_INT_PIN;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GSL1680_INT_PORT, &GPIO_InitStructure);

	GSL1680_WAKE_RESET;

}

void gsl1680_clr_reg(void)
{

	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,0xe0, 0x88);
	delay(20000);

	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,0x80, 0x01);
	delay(5000);

	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,0xe4, 0x04);
	delay(5000);

	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,0xe0, 0x00);
	delay(20000);
}

void gsl1680_reset_chip()
{
	uint8_t buf[4];

	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,GSL_STATUS_REG, 0x88);
	delay(10000);

	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,0xe4, 0x04);
	delay(10000);

	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
    TM_I2C_WriteMulti(I2C1,GSLX680_I2C_ADDR,0xbc,buf, 4);
    delay(10000);
}

void gsl1680_load_fw(void)
{
    uint8_t addr;
    uint8_t Wrbuf[4];
    uint source_line = 0;
    uint source_len = sizeof(GSLX680_FW) / sizeof(struct fw_data);


    for (source_line = 0; source_line < source_len; source_line++) {
        addr = GSLX680_FW[source_line].offset;
        Wrbuf[0] = (char)( GSLX680_FW[source_line].val & 0x000000ff);
        Wrbuf[1] = (char)((GSLX680_FW[source_line].val & 0x0000ff00) >> 8);
        Wrbuf[2] = (char)((GSLX680_FW[source_line].val & 0x00ff0000) >> 16);
        Wrbuf[3] = (char)((GSLX680_FW[source_line].val & 0xff000000) >> 24);

        TM_I2C_WriteMulti(I2C1,GSLX680_I2C_ADDR,addr, Wrbuf, 4);
    }
}


void gsl1680_startup_chip(void)
{
	TM_I2C_Write(I2C1,GSLX680_I2C_ADDR,0xe0, 0x00);
}

//		PinS PACK 1		PINS PACK 2		PINS PACK 3
//I2Cx	SCL		SDA		SCL		SDA		SCL	SDA			APB
//I2C1	PB6		PB7		PB8		PB9		PB6	PB9			1
//I2C2	PB10 	PB11 	PF1		PF0		PH4	PH5			1
//I2C3	PA8		PC9		PH7		PH8						1

uint8_t gsl1680_init_chip()
{

	TM_I2C_Init(I2C1, TM_I2C_PinsPack_2, 400000);		// PB8		PB9

	gsl1680_pin_init();

	GSL1680_WAKE_SET;
	delay(50000);
	GSL1680_WAKE_RESET;
	delay(50000);
	GSL1680_WAKE_SET;
	delay(30000);

	if ( !(TM_I2C_IsDeviceConnected(I2C1, GSLX680_I2C_ADDR)) )
	return 0;

	// CTP startup sequence

	gsl1680_clr_reg();

	gsl1680_reset_chip();

	gsl1680_load_fw();

	gsl1680_reset_chip();

	gsl1680_startup_chip();

	return 1;
}

uint8_t gsl1680_read_data(void)
{

	uint8_t i;
	uint8_t touch_data[24] = {0};

	TM_I2C_ReadMulti(I2C1,GSLX680_I2C_ADDR,GSL_DATA_REG, touch_data, 24);

	ts_event.n_fingers= touch_data[0];

	for(i=0; i<ts_event.n_fingers; i++){

		ts_event.coords[i].x = ( (((uint32_t)touch_data[(i*4)+5])<<8) | (uint32_t)touch_data[(i*4)+4] ) & 0x00000FFF; // 12 bits of X coord
		ts_event.coords[i].y = ( (((uint32_t)touch_data[(i*4)+7])<<8) | (uint32_t)touch_data[(i*4)+6] ) & 0x00000FFF;
		ts_event.coords[i].finger = (uint32_t)touch_data[(i*4)+7] >> 4; // finger that did the touch
	}

	return ts_event.n_fingers;
}

#pragma GCC optimize("O0")

static void delay(ulong delay)
{
    ulong    i,k;

    for ( k = 0 ; (k < delay ); k++ )
        for ( i = 0 ; (i < US_DELAY ); i++ )
        {
            asm("");
        }
}

