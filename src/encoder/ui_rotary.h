/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __UI_ROTARY_H
#define __UI_ROTARY_H

#include "typedef.h"


// pin 6
#define FREQ_ENC_CH1 			GPIO_Pin_6
#define FREQ_ENC_CH1_SOURCE		GPIO_PinSource6
#define FREQ_ENC_CH1_PIO        GPIOC
// pin 7
#define FREQ_ENC_CH2 			GPIO_Pin_7
#define FREQ_ENC_CH2_SOURCE		GPIO_PinSource7
#define FREQ_ENC_CH2_PIO        GPIOC

#define ENC_ONE_CH1 			GPIO_Pin_4
#define ENC_ONE_CH1_SOURCE		GPIO_PinSource4
#define ENC_ONE_CH1_PIO       	GPIOB

#define ENC_ONE_CH2 			GPIO_Pin_5
#define ENC_ONE_CH2_SOURCE		GPIO_PinSource5
#define ENC_ONE_CH2_PIO       	GPIOB

#define ENC_TWO_CH1 			GPIO_Pin_12
#define ENC_TWO_CH1_SOURCE		GPIO_PinSource12
#define ENC_TWO_CH1_PIO         GPIOD

#define ENC_TWO_CH2 			GPIO_Pin_13
#define ENC_TWO_CH2_SOURCE		GPIO_PinSource13
#define ENC_TWO_CH2_PIO         GPIOD

#define ENC_THREE_CH1 			GPIO_Pin_0
#define ENC_THREE_CH1_SOURCE	GPIO_PinSource0
#define ENC_THREE_CH1_PIO       GPIOA

#define ENC_THREE_CH2 			GPIO_Pin_1
#define ENC_THREE_CH2_SOURCE	GPIO_PinSource1
#define ENC_THREE_CH2_PIO       GPIOA



// Some encoders have detend and generate x
// ammount of pulses per click
#define USE_DETENTED_ENCODERS
#define USE_DETENTED_VALUE	4

// Protective band on top and bottom scale
// dependent on encoder quality and debounce
// capacitors
#define ENCODER_FLICKR_BAND	4

// Maximum pot value
#define FREQ_ENCODER_RANGE	0xFFF

// Divider to create non linearity
#define FREQ_ENCODER_LOG_D	1

// The UI checking function will skip
// reading too often based on this value
#define FREQ_UPDATE_SKIP	100

// --------------------------------
// Maximum pot value
#define ENCODER_ONE_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_ONE_LOG_D	1

// Audio Gain public structure
typedef struct EncoderOneSelection
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value
	uchar	de_detent;			// sw de-detent flag

} EncoderOneSelection;

// --------------------------------
// Maximum pot value
#define ENCODER_TWO_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_TWO_LOG_D	1

// Audio Gain public structure
typedef struct EncoderTwoSelection
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value
	uchar	de_detent;			// sw de-detent flag

} EncoderTwoSelection;

// --------------------------------
// Maximum pot value
#define ENCODER_THR_RANGE	0xFFF

// Divider to create non linearity
#define ENCODER_THR_LOG_D	1

// Frequency public structure
typedef struct DialFrequency
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value

	// SI570 actual frequency
	ulong	tune_old;			// previous value
	ulong	tune_new;			// most current value

	// Current tuning step
	ulong	tuning_step;		// selected step by user
	ulong	selected_idx;		// id of step
	ulong	last_tune_step;		// last tunning step used during dial rotation
	ulong	step_new;			// Eth driver req step

	ulong	update_skip;

	ulong	transv_freq;

	ulong 	tunning_step;

	// Shift used on TX
	//int		tx_shift;

	// TCXO routine factor/flag
	int		temp_factor;
	uchar	temp_enabled;

	uchar	de_detent;			// sw de-detent flag

	// Virtual segments
	uchar	dial_100_mhz;
	uchar	dial_010_mhz;
	uchar	dial_001_mhz;
	uchar	dial_100_khz;
	uchar	dial_010_khz;
	uchar	dial_001_khz;
	uchar	dial_100_hz;
	uchar	dial_010_hz;
	uchar	dial_001_hz;

	// Second display
	uchar	sdial_100_mhz;
	uchar	sdial_010_mhz;
	uchar	sdial_001_mhz;
	uchar	sdial_100_khz;
	uchar	sdial_010_khz;
	uchar	sdial_001_khz;
	uchar	sdial_100_hz;
	uchar	sdial_010_hz;
	uchar	sdial_001_hz;
				
} DialFrequency;

// FIR selector public structure
typedef struct EncoderThreeSelection
{
	// pot values
	//
	ulong	value_old;			// previous value
	ulong	value_new;			// most current value
	uchar	de_detent;			// sw de-detent flag

} EncoderThreeSelection;

void UiRotaryFreqEncoderInit(void);
void UiRotaryEncoderOneInit(void);
void UiRotaryEncoderTwoInit(void);
void UiRotaryEncoderThreeInit(void);

#endif
