#ifndef __RADIO_H
#define __RADIO_H

#include "stm32f4xx.h"
#include "typedef.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_fmc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_adc.h"

#include "tm_stm32f4_button.h"

#include "tm_stm32f4_sdram.h"

#include "typedef.h"
#include "ui_ra8875.h"
#include "gsl1680.h"

#include <stdio.h>
#include "arm_math.h"

#include "codec.h"
#include "i2s.h"
#include "audio.h"
#include "ui.h"
#include "ui_rotary.h"

#include "ui_si570.h"
#include "mchf_hw_i2c.h"

#include "si5351.h"


// FILTROS SOFTROCK

#define FL_GPIO					GPIOC
#define FL_SEL0					GPIO_Pin_0
#define FL_SEL1					GPIO_Pin_1

// pin 6
#define I2C1_SCL_PIN            GPIO_Pin_6
#define I2C1_SCL_PINSRC         GPIO_PinSource6
#define I2C1_SCL_GPIO           GPIOB
// pin 7
#define I2C1_SDA_PIN            GPIO_Pin_7
#define I2C1_SDA_PINSRC         GPIO_PinSource7
#define I2C1_SDA_GPIO           GPIOB

#define	MIN_BANDS					0		// lowest band number
#define	MAX_BANDS					9		// Highest band number:  9

#define	KHZ_MULT					4000

#define	BAND_MODE_80				0
#define	BAND_FREQ_80				3705*KHZ_MULT		// 3500 kHz
#define	BAND_SIZE_80				500*KHZ_MULT		// 500 kHz in size (Region 2)
//
#define	BAND_MODE_60				1
#define	BAND_FREQ_60				5258*KHZ_MULT		//5258 kHz
#define	BAND_SIZE_60				150*KHZ_MULT		// 150 kHz in size to allow different allocations
//
#define	BAND_MODE_40				2
#define	BAND_FREQ_40				7100*KHZ_MULT		// 7000 kHz
#define	BAND_SIZE_40				300*KHZ_MULT		// 300 kHz in size (Region 2)
//
#define	BAND_MODE_30				3
#define	BAND_FREQ_30				10100*KHZ_MULT		// 10100 kHz
#define	BAND_SIZE_30				50*KHZ_MULT			// 50 kHz in size
//
#define	BAND_MODE_20				4
#define	BAND_FREQ_20				14100*KHZ_MULT		// 14000 kHz
#define	BAND_SIZE_20				350*KHZ_MULT		// 350 kHz in size
//
#define	BAND_MODE_17				5
#define	BAND_FREQ_17				18068*KHZ_MULT		// 18068 kHz
#define	BAND_SIZE_17				100*KHZ_MULT		// 100 kHz in size
//
#define	BAND_MODE_15				6
#define	BAND_FREQ_15				21000*KHZ_MULT		// 21000 kHz
#define	BAND_SIZE_15				450*KHZ_MULT		// 450 kHz in size
//
#define	BAND_MODE_12				7
#define	BAND_FREQ_12				24890*KHZ_MULT		// 24890 kHz
#define	BAND_SIZE_12				100*KHZ_MULT		// 100 kHz in size
//
#define	BAND_MODE_10				8
#define	BAND_FREQ_10				28000*KHZ_MULT		// 28000 kHz
#define	BAND_SIZE_10				2700*KHZ_MULT		// 2700 kHz in size
//
#define	BAND_MODE_GEN				9					// General Coverage
#define	BAND_FREQ_GEN				10000*KHZ_MULT		// 10000 kHz
#define	BAND_SIZE_GEN				1*KHZ_MULT			// Dummy variable




#define DEBUG_PRINT	 			GPIO_Pin_2
#define DEBUG_PRINT_SOURCE		GPIO_PinSource2
#define DEBUG_PRINT_PIO    		GPIOA

#define DEBUG_COM               USART2

//
// Audio filter select enumeration
//
enum	{
	AUDIO_300HZ = 0,
	AUDIO_500HZ,
	AUDIO_1P8KHZ,
	AUDIO_2P3KHZ,
	AUDIO_2P7KHZ,
	AUDIO_2P9KHZ,
	AUDIO_3P6KHZ,
	AUDIO_WIDE
};

enum	{
	WIDE_FILTER_10K_AM = 0,
	WIDE_FILTER_7K5_AM,
	WIDE_FILTER_6K_AM,
	WIDE_FILTER_5K_AM,
	WIDE_FILTER_10K,
	WIDE_FILTER_7K5,
	WIDE_FILTER_6K,
	WIDE_FILTER_5K,
	WIDE_FILTER_MAX
};

#define	FILTER_WIDE_DEFAULT		WIDE_FILTER_10K

//
// use below to define the lowest-used filter number
//
#define	AUDIO_DEFAULT_FILTER		AUDIO_2P3KHZ
//
#define AUDIO_MIN_FILTER			0
//
// use below to define the highest-used filter number-1
//
#define AUDIO_MAX_FILTER		8
//
//
#define MIN_FILTER_SELECT_VAL		1		// Minimum value for selection of sub-filter
//
#define	MAX_300HZ_FILTER		9		// Highest number selection of 500 Hz filter
#define	FILTER_300HZ_DEFAULT		6		// Center frequency of 750 Hz
//
#define	MAX_500HZ_FILTER		5
#define	FILTER_500HZ_DEFAULT		3		// Center frequency of 750 Hz
//
#define	MAX_1K8_FILTER			6
#define	FILTER_1K8_DEFAULT		3		// Center frequency of 1425 Hz
//
#define	MAX_2K3_FILTER			5
#define	FILTER_2K3_DEFAULT		2		// Center frequency of 1412 Hz
//
#define	MAX_2K7_FILTER			2
#define	FILTER_2K7_DEFAULT		2		// Center frequency of 1412 Hz
//
#define	MAX_2K9_FILTER			2
#define	FILTER_2K9_DEFAULT		2		// Center frequency of 1412 Hz
//
#define	MAX_3K6_FILTER			2		// only on/off
#define	FILTER_3K6_DEFAULT		2		// 1 = Enabled

//
//
#define	FILTER_WIDE_DEFAULT			WIDE_FILTER_10K		// 10k selected by default
//
//

// Define visual widths of audio filters for on-screen indicator in Hz
//
#define	FILTER_300HZ_WIDTH		300
#define	FILTER_500HZ_WIDTH		500
#define	FILTER_1800HZ_WIDTH		1800
#define FILTER_2300HZ_WIDTH		2300
#define FILTER_2700HZ_WIDTH		2700
#define FILTER_2900HZ_WIDTH		2900
#define FILTER_3600HZ_WIDTH		3600
#define	FILTER_5000HZ_WIDTH		5000
#define	FILTER_6000HZ_WIDTH		6000
#define FILTER_7500HZ_WIDTH		7500
#define	FILTER_10000HZ_WIDTH		10000
//
#define	HILBERT_3600HZ_WIDTH		3800	// Approximate bandwidth of 3.6 kHz wide Hilbert - This used to depict FM detection bandwidth
//
#define	FILT300_1			500
#define	FILT300_2			550
#define	FILT300_3			600
#define	FILT300_4			650
#define	FILT300_5			700
#define	FILT300_6			750
#define	FILT300_7			800
#define	FILT300_8			850
#define	FILT300_9			900
//
#define	FILT500_1			550
#define	FILT500_2			650
#define	FILT500_3			750
#define	FILT500_4			850
#define	FILT500_5			950
//
#define	FILT1800_1			1125
#define	FILT1800_2			1275
#define	FILT1800_3			1427
#define	FILT1800_4			1575
#define	FILT1800_5			1725
#define	FILT1800_6			 900
//
#define	FILT2300_1			1262
#define	FILT2300_2			1412
#define	FILT2300_3			1562
#define	FILT2300_4			1712
#define	FILT2300_5			1150
//
#define FILT2700_1			1350
#define	FILT2700_2			1425
//
#define FILT2900_1			1450
#define	FILT2900_2			1525
//
#define FILT3600_1			1800
#define	FILT3600_2			1875
#define	FILT3600			1800
//
#define	FILT5000			2500
//
#define	FILT6000			3000
//
#define	FILT7500			3750
//
#define	FILT10000			5000
//
#define	HILBERT3600			1900	// "width" of "3.6 kHz" Hilbert filter - This used to depict FM detection bandwidth
//
#define	FILT_DISPLAY_WIDTH		800		// width, in pixels, of the spectral display on the screen - this value used to calculate Hz/pixel for indicating width of filter

#define	US_DELAY			15  // 15 gives 1 uS delay in loop without optimization(O0)

//
// Enumeration of colours used in spectrum scope display
//
enum {
	SPEC_WHITE = 0,
	SPEC_GREY,
	SPEC_BLUE,
	SPEC_RED,
	SPEC_MAGENTA,
	SPEC_GREEN,
	SPEC_CYAN,
	SPEC_YELLOW,
	SPEC_ORANGE,
	SPEC_BLACK,
	SPEC_GREY2,
	SPEC_MAX_COLOUR,
};
//
#define	SPEC_COLOUR_TRACE_DEFAULT	SPEC_WHITE
#define	SPEC_COLOUR_GRID_DEFAULT	SPEC_GREY
#define SPEC_COLOUR_SCALE_DEFAULT	SPEC_GREY
#define	FILTER_DISP_COLOUR_DEFAULT	SPEC_GREY

typedef struct TransceiverState
{

	uchar	iq_freq_mode;
	uchar	spectrum_scope_nosig_adjust;
	uchar   scope_agc_rate;

	// AGC mode
	uchar	agc_mode;
	uchar	agc_custom_decay;

	uchar	lcd_backlight_brightness;	// LCD backlight brightness, 0-3:  0 = full, 3 = dimmest

	int32_t	dmod_mode;

	int 	rf_gain;
	uchar	max_rf_gain;
	uchar	rf_codec_gain;		// gain for codec (A/D converter) in receive mode

	uchar 	band;
	uchar   band_mode;
	uchar	freq_step_config;
	uchar	tune_step;


	ulong	tune_freq;			// main synthesizer frequency
	ulong	tune_freq_old;		// used to detect change of main synthesizer frequency

	int	freq_cal;				// frequency calibration

	uchar	scope_speed;	// update rate for spectrum scope

	uchar	scope_filter;	// strength of filter in spectrum scope

	uchar	scope_trace_colour;	// color of spectrum scope trace;
	uchar	scope_grid_colour;	// saved color of spectrum scope grid;
	ulong	scope_grid_colour_active;	// active color of spectrum scope grid;
	uchar	scope_centre_grid_colour;	// color of center line of scope grid
	ushort	scope_centre_grid_colour_active;	// active colour of the spectrum scope center grid line
	uchar	scope_scale_colour;	// color of spectrum scope frequency scale
	uchar	scope_rescale_rate;	// rescale rate on the 'scope
	uchar	spectrum_db_scale;	// db/Division scale setting on spectrum scope
	uchar	waterfall_speed;	// speed of update of the waterfall

	int		rx_iq_lsb_gain_balance;		// setting for RX IQ gain balance
	int		rx_iq_usb_gain_balance;		// setting for RX IQ gain balance
	//
	int		rx_iq_am_gain_balance;		// setting for RX IQ gain balance
	//
	int		rx_iq_lsb_phase_balance;	// setting for RX IQ phase balance
	int		rx_iq_usb_phase_balance;	// setting for RX IQ phase balance

	float	rx_adj_gain_var_i;		// active variables for adjusting rx gain balance
	float	rx_adj_gain_var_q;

	// Audio filter ID
	uchar	filter_id;
	//
	uchar	filter_300Hz_select;
	uchar	filter_500Hz_select;
	uchar	filter_1k8_select;
	uchar	filter_2k3_select;
	uchar	filter_2k7_select;
	uchar	filter_2k9_select;
	uchar	filter_3k6_select;
	uchar	filter_wide_select;

	//
	uchar	filter_cw_wide_disable;		// TRUE if wide filters are disabled in CW mode
	uchar	filter_ssb_narrow_disable;	// TRUE if narrow filters are disabled in SSB modes

	bool	nb_disable;					// TRUE if noise blanker is to be disabled
	//
	uchar	dsp_active;					// Used to hold various aspects of DSP mode selection
										// LSB = 1 if DSP NR mode is on (| 1)
										// LSB+1 = 1 if DSP NR is to occur post AGC (| 2)
										// LSB+2 = 1 if DSP Notch mode is on (| 4)
										// LSB+3 = 0 if DSP is to be displayed on screen instead of NB (| 8)
										// MSB	 = 1 if button G2 toggle NOT initialized (| 128)
	uchar	dsp_active_toggle;			// holder used on the press-hold of button G2 to "remember" the previous setting
	uchar	dsp_nr_strength;			// "Strength" of DSP Noise reduction - to be converted to "Mu" factor
	ulong	dsp_nr_delaybuf_len;		// size of DSP noise reduction delay buffer
	uchar	dsp_nr_numtaps;				// Number of FFT taps on the DSP Noise reduction
	uchar	dsp_notch_mu;				// mu adjust of notch DSP LMS
	ulong	dsp_notch_delaybuf_len;		// size of DSP notch delay buffer
	bool	dsp_inhibit;				// if TRUE, DSP (NR, Notch) functions are inhibited.  Used during power-up
	bool	dsp_timed_mute;				// TRUE if DSP is to be muted for a timed amount
	ulong	dsp_inhibit_timing;			// used to time inhibiting of DSP when it must be turned off for some reason
	bool	reset_dsp_nr;				// TRUE if DSP NR coefficients are to be reset when "audio_driver_set_rx_audio_filter()" is called

	uchar 	nb_setting;
	uchar	nb_agc_time_const;			// used to calculate the AGC time constant

	bool	cw_lsb;

	uchar	waterfall_color_scheme;		// stores waterfall color scheme
	uchar	waterfall_vert_step_size;	// vertical step size in waterfall mode
	ulong	waterfall_offset;			// offset for waterfall display
	ulong	waterfall_contrast;			// contrast setting for waterfall display

	uchar	waterfall_nosig_adjust;		// Adjustment for no signal adjustment conditions for waterfall
	uchar	waterfall_size;				// size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved

	uchar	misc_flags1;				// Used to hold individual status flags, stored in EEPROM location "EEPROM_MISC_FLAGS1"
										// LSB = 0 if on-screen AFG/(STG/CMP) and WPM/(MIC/LIN) indicators are changed on TX
										// LSB+1 = 1 if BAND-/BAND+ buttons are to be swapped in their positions
										// LSB+2 = 1 if TX audio output from LINE OUT is to be muted during transmit (audio output only enabled
											//	when translate mode is DISABLED
										// LSB+3 = 1 if AM TX has transmit filter DISABLED
										// LSB+4 = 1 if FWD/REV A/D inputs from RF power detectors are to be reversed
										// LSB+5 = 1 if Frequency tuning is to be relaxed
										// LSB+6 = 1 if SSB TX has transmit filter DISABLED
										// LSB+7 = 0 = Spectrum Scope (analyzer), 1 = Waterfall display

	uchar 	audio_gain;


	// Transceiver menu mode variables
	uchar	menu_mode;		// TRUE if in menu mode
	uchar	menu_item;		// Used to indicate specific menu item
	int		menu_var;		// Used to change specific menu item
	bool	menu_var_changed;	// TRUE if something changed in a menu and that an EEPROM save should be done!

	uchar	audio_max_volume;	// limit for maximum audio gain
	uchar   audio_gain_change;	// change detect for audio gain

	uchar	fft_window_type;

	uchar	filter_band;

	bool 	rx_muting;

	ulong	voltmeter_calibrate;

	uchar	filter_disp_colour;			// used to hold the current color of the line that indicates the filter passband/bandwidth

	uchar c_line;

	uchar	spectrum_size;				// size of waterfall display (and other parameters) - size setting is in lower nybble, upper nybble/byte reserved

	uint32_t audio_int_counter;		// used for encoder timing - test DL2FW

	uchar spectrum_filter;

} TransceiverState;
//

//
// *******************************************************************************************************
//
typedef struct FilterCoeffs
{
	float		rx_filt_q[128];
	uint16_t	rx_q_num_taps;
	uint32_t	rx_q_block_size;
	float		rx_filt_i[128];
	uint16_t	rx_i_num_taps;
	uint32_t	rx_i_block_size;
} FilterCoeffs;

#define non_os_delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

#define non_os_delay_a()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 10000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

#endif
