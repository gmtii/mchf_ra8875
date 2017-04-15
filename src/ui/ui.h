#ifndef __UI_H
#define __UI_H

#include "radio.h"
#include "arm_const_structs.h"


// SI570 startup value (receive freq * 4)
//#define 	STARTUP_FREQ					112000000
#define 	STARTUP_FREQ					56000000

#define 	MAX_STEPS						7
#define 	T_STEP_1HZ						1
#define 	T_STEP_10HZ						10
#define 	T_STEP_100HZ					100
#define 	T_STEP_1KHZ						1000
#define 	T_STEP_5KHZ						5000
#define 	T_STEP_10KHZ					10000
#define 	T_STEP_100KHZ					100000

enum {
	T_STEP_1HZ_IDX = 0,
	T_STEP_10HZ_IDX,
	T_STEP_100HZ_IDX,
	T_STEP_1KHZ_IDX,
	T_STEP_5KHZ_IDX,
	T_STEP_10KHZ_IDX,
	T_STEP_100KHZ_IDX,
	T_STEP_1MHZ_IDX,
	T_STEP_10MHZ_IDX,
	T_STEP_MAX_STEPS
};


// Spectrum display
#define POS_SPECTRUM_IND_X					0
#define POS_SPECTRUM_IND_Y					200
#define POS_SPECTRUM_IND_H					80
#define	POS_SPECTRUM_FREQ_BAR_Y				64	// reducing value moves upwards
#define	POS_SPECTRUM_FILTER_WIDTH_BAR_Y		61
#define POS_SPECTRUM_IND_W					258
#define COL_SPECTRUM_GRAD					0x40

#define SPECTRUM_START_X		POS_SPECTRUM_IND_X
//
// Shift of whole spectrum in vertical direction
#define SPECTRUM_START_Y		(POS_SPECTRUM_IND_Y - 10)
//
// Spectrum hight is bit lower that the whole control
#define SPECTRUM_HEIGHT			(POS_SPECTRUM_IND_H - 10)
#define RA8875_SPECTRUM_HEIGHT	200

//
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH			800

#define	FFT_WINDOW_DEFAULT		FFT_WINDOW_NUTTALL

// --------------------------------------------------------------------------
// Controls positions
// --------------------
#define SMALL_FONT_WIDTH					8
#define LARGE_FONT_WIDTH					16

#define	AUDIO_DEFAULT_FILTER		AUDIO_2P3KHZ

// --------------------------------------------------------------------------
// Controls positions and some related colours
// --------------------
#define SMALL_FONT_WIDTH					8
#define LARGE_FONT_WIDTH					16

// Frequency display control
#define POS_TUNE_FREQ_X						476
#define POS_TUNE_FREQ_Y						60
//
#define	POS_TUNE_SPLIT_FREQ_X				POS_TUNE_FREQ_X+72
#define	POS_TUNE_SPLIT_MARKER_X				POS_TUNE_FREQ_X+40
#define	POS_TUNE_SPLIT_FREQ_Y_TX			POS_TUNE_FREQ_Y+12

//
#define	SPLIT_ACTIVE_COLOUR		Yellow		// colour of "SPLIT" indicator when active
#define	SPLIT_INACTIVE_COLOUR	Grey		// colour of "SPLIT" indicator when NOT active

// Second frequency display control
#define POS_TUNE_SFREQ_X					(POS_TUNE_FREQ_X + 240)
#define POS_TUNE_SFREQ_Y					(POS_TUNE_FREQ_Y + 60)

// Band selection control
#define POS_BAND_MODE_X						(POS_TUNE_FREQ_X + 250)
#define POS_BAND_MODE_Y						(POS_TUNE_FREQ_Y - 30)
#define POS_BAND_MODE_MASK_X				(POS_BAND_MODE_X - 1)
#define POS_BAND_MODE_MASK_Y				(POS_BAND_MODE_Y - 1)
#define POS_BAND_MODE_MASK_H				13
#define POS_BAND_MODE_MASK_W				33

// Demodulator mode control
#define POS_DEMOD_MODE_X					(POS_TUNE_FREQ_X + 1)
#define POS_DEMOD_MODE_Y					(POS_TUNE_FREQ_Y - 30)
#define POS_DEMOD_MODE_MASK_X				(POS_DEMOD_MODE_X - 1)
#define POS_DEMOD_MODE_MASK_Y				(POS_DEMOD_MODE_Y - 1)
#define POS_DEMOD_MODE_MASK_H				30
#define POS_DEMOD_MODE_MASK_W				60

// Tunning step control
#define POS_TUNE_STEP_X						(POS_TUNE_FREQ_X + 50)
#define POS_TUNE_STEP_Y						(POS_TUNE_FREQ_Y - 30)
#define POS_TUNE_STEP_MASK_X				(POS_TUNE_STEP_X - 1)
#define POS_TUNE_STEP_MASK_Y				(POS_TUNE_STEP_Y - 1)
#define POS_TUNE_STEP_MASK_H				17
#define POS_TUNE_STEP_MASK_W				49

#define POS_RADIO_MODE_X					4
#define POS_RADIO_MODE_Y					5

// Bottom bar
#define POS_BOTTOM_BAR_X					0
#define POS_BOTTOM_BAR_Y					425
#define POS_BOTTOM_BAR_BUTTON_W				100
#define POS_BOTTOM_BAR_BUTTON_H				55

// Virtual Button 1
#define POS_BOTTOM_BAR_F1_X					(POS_BOTTOM_BAR_X +                              2)
#define POS_BOTTOM_BAR_F1_Y					POS_BOTTOM_BAR_Y

// Virtual Button 2
#define POS_BOTTOM_BAR_F2_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*1 +  4)
#define POS_BOTTOM_BAR_F2_Y					POS_BOTTOM_BAR_Y

// Virtual Button 3
#define POS_BOTTOM_BAR_F3_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*2 +  6)
#define POS_BOTTOM_BAR_F3_Y					POS_BOTTOM_BAR_Y

// Virtual Button 4
#define POS_BOTTOM_BAR_F4_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*3 +  8)
#define POS_BOTTOM_BAR_F4_Y					POS_BOTTOM_BAR_Y

// Virtual Button 5
#define POS_BOTTOM_BAR_F5_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*4 + 10)
#define POS_BOTTOM_BAR_F5_Y					POS_BOTTOM_BAR_Y

// Virtual Button 6
#define POS_BOTTOM_BAR_F6_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*5 + 12)
#define POS_BOTTOM_BAR_F6_Y					POS_BOTTOM_BAR_Y


// Virtual Button 7
#define POS_BOTTOM_BAR_F7_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*6 + 14)
#define POS_BOTTOM_BAR_F7_Y					POS_BOTTOM_BAR_Y


// Virtual Button 8
#define POS_BOTTOM_BAR_F8_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*7 + 16)
#define POS_BOTTOM_BAR_F8_Y					POS_BOTTOM_BAR_Y

// --------------------------------------------------
// Encoder one controls indicator
// audio gain
#define POS_AG_IND_X						0+350
#define POS_AG_IND_Y						125
// sidetone gain
#define POS_SG_IND_X						60+350
#define POS_SG_IND_Y						125

// --------------------------------------------------
// Encoder two controls indicator
// RF Gain indicator
#define POS_RF_IND_X						0+350+120
#define POS_RF_IND_Y						125
// RF attenuator
#define POS_RA_IND_X						60+350+120
#define POS_RA_IND_Y						125

// --------------------------------------------------
// Encoder three controls indicator
// RIT indicator
#define POS_RIT_IND_X						0+350
#define POS_RIT_IND_Y						150
// keyer speed
#define POS_KS_IND_X						60+350
#define POS_KS_IND_Y						150

// --------------------------------------------------
// Calibration mode
//
// PA bias
#define POS_PB_IND_X						0
#define POS_PB_IND_Y						78
// IQ gain balance
#define POS_BG_IND_X						0
#define POS_BG_IND_Y						94
// IQ phase balance
#define POS_BP_IND_X						0
#define POS_BP_IND_Y						110
// Frequency Calibrate
#define POS_FC_IND_X						0
#define POS_FC_IND_Y						78

// --------------------------------------------------
// Standalone controls
//
// DSP mode
// Upper DSP box
#define POS_DSPU_IND_X						0+350
#define POS_DSPU_IND_Y						150
// Lower DSP box
#define POS_DSPL_IND_X						60+350
#define POS_DSPL_IND_Y						150

// Power level
#define POS_PW_IND_X						0+350+120
#define POS_PW_IND_Y						150
// Filter indicator
#define POS_FIR_IND_X						60+350+180
#define POS_FIR_IND_Y						125

// ETH position
#define POS_ETH_IND_X						220
#define POS_ETH_IND_Y						2

// USB Keyboard position
#define POS_KBD_IND_X						220
#define POS_KBD_IND_Y						18

// S meter position
#define POS_SM_IND_X						580
#define POS_SM_IND_Y						0

// Supply Voltage indicator
#define POS_PWRN_IND_X						0+350
#define POS_PWRN_IND_Y						193
#define POS_PWR_IND_X						5+350
#define POS_PWR_IND_Y						(POS_PWRN_IND_Y + 15)
#define COL_PWR_IND							Grey

#define POS_TEMP_IND_X						0
#define POS_TEMP_IND_Y						0
//
// Starting position of configuration menu
//
#define	POS_MENU_IND_X						60		// X position of description of menu item being changed
#define	POS_MENU_IND_Y						128		// Y position of first (top) item being changed
#define	POS_MENU_CHANGE_X					244		// Position of variable being changed
#define	POS_MENU_CURSOR_X					311		// Position of cursor used to indicate selected item

#define	SPECTRUM_SCOPE_AGC_MIN				1	// minimum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_MAX				50	// maximum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_DEFAULT			25	// default spectrum scope AGC rate setting

#define	SPECTRUM_AGC_SCALING				25	// scaling factor by which the on-screen Spec. AGC Adj. is divided for adjustment.

// Spectrum scope operational constants
//
#define	SPECTRUM_SCOPE_TOP_LIMIT			5	// Top limit of spectrum scope magnitude
//
#define SPECTRUM_SCOPE_AGC_THRESHOLD		2000//400	// AGC "Knee" above which output from spectrum scope FFT  AGC will cause action
#define SPECTRUM_SCOPE_MAX_FFT_VAL			8192//16384 // Value above which input to spectrum scope FFT will cause AGC action
#define SPECTRUM_SCOPE_MIN_GAIN				0.001//0.1	// Minimum gain for spectrum scope FFT AGC loop
#define SPECTRUM_SCOPE_MAX_GAIN				140	// Maximum gain for spectrum scope FFT AGC loop
#define	SPECTRUM_SCOPE_AGC_ATTACK			0.5//0.1	// Attack rate for spectrum scope FFT AGC/gain
#define	SPECTRUM_SCOPE_AGC_DECAY			0.166//0.1	// Decay rate for spectrum scope FFT AGC/gain
//
#define	SPECTRUM_SCOPE_LPF_FACTOR			4	// IIR Factor for spectrum scope low-pass filtering
	// Higher = slower response.  3 = 33% input; 66% feedback, 4 = 25% input, 75% feedback, 5 = 20% input, 80% feedback
#define	SPECTRUM_SCOPE_LPF_FACTOR_SPI		2	// IIR Factor for spectrum scope low-pass filtering using SPI display (update rate is slower, use faster filter)
//
#define PK_AVG_RESCALE_THRESH		3		// This sets the minimum peak-to-average ratio of the spectrum display before it starts to rescale the peak
		// value from the top.  This prevents it from going completely "white" on noise/no-signal conditions.
//
#define	SPECTRUM_SCOPE_RESCALE_ATTACK_RATE	0.1	// Rate at which scaling of spectrum scope adapts to strong signals within its passband
#define	SPECTRUM_SCOPE_RESCALE_DECAY_RATE	0.033	// Rate at which scaling of spectrum scope decays after strong signals disappear from its passband
//
#define SPECTRUM_SCOPE_SPEED_MIN			1	// minimum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_MAX			25	// maximum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_DEFAULT		5
//
#define SPECTRUM_SCOPE_FILTER_MIN			1	// minimum filter setting
#define	SPECTRUM_SCOPE_FILTER_MAX			10	// maximum filter setting
#define SPECTRUM_SCOPE_FILTER_DEFAULT		4	// default filter setting
//
#define	SPECTRUM_SCOPE_AGC_MIN				1	// minimum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_MAX				50	// maximum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_DEFAULT			25	// default spectrum scope AGC rate setting
//
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MIN		10
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MAX		30
#define	SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT		20
//
#define	SPECTRUM_SCOPE_ADJUST_OFFSET	100
//
//
#define	SPECTRUM_AGC_SCALING				25	// scaling factor by which the on-screen Spec. AGC Adj. is divided for adjustment.
//
#define	SCOPE_PREAMP_GAIN					1000//200	// amount of "amplification" in front of the FFT used for the Spectrum Scope and Waterfall used to overcome mathematical "noise floor"
//
#define INIT_SPEC_AGC_LEVEL					-80	// Initial offset for AGC level for spectrum/waterfall display
//
#define SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE	25	// time, in 100's of second to inhibit spectrum scope update after adjusting tuning while in SPI mode
//
#define	NUMBER_WATERFALL_COLOURS			64		// number of colors in the waterfall table

#define	MIN_TX_IQ_GAIN_BALANCE	-99	// Minimum setting for TX IQ gain balance
#define MAX_TX_IQ_GAIN_BALANCE	99	// Maximum setting for TX IQ gain balance
//
#define	MIN_RX_IQ_GAIN_BALANCE	-99	// Minimum setting for RX IQ gain balance
#define	MAX_RX_IQ_GAIN_BALANCE	99	// Maximum setting for RX IQ gain balance
//
#define	MIN_TX_IQ_PHASE_BALANCE	-32	// Minimum setting for TX IQ phase balance
#define	MAX_TX_IQ_PHASE_BALANCE	32	// Maximum setting for TX IQ phase balance
//
#define	MIN_RX_IQ_PHASE_BALANCE	-32	// Minimum setting for RX IQ phase balance
#define	MAX_RX_IQ_PHASE_BALANCE	32	// Maximum setting for RX IQ phase balance
//

//
#define	DB_DIV_ADJUST_MIN	DB_DIV_DEFAULT
#define	DB_DIV_ADJUST_MAX	S_3_DIV
#define	DB_DIV_ADJUST_DEFAULT	DB_DIV_10
//
// scaling factors for the various dB/division settings
//
#define	DB_SCALING_5	63.2456		// 5dB/division scaling
#define	DB_SCALING_7	42.1637		// 7.5dB/division scaling
#define	DB_SCALING_10	31.6228		// 10dB/division scaling
#define	DB_SCALING_15	21.0819		// 15dB/division scaling
#define	DB_SCALING_20	15.8114		// 20dB/division scaling
#define	DB_SCALING_S1	52.7046		// 1 S unit (6 dB)/division scaling
#define DB_SCALING_S2	26.3523		// 2 S unit (12 dB)/division scaling
#define	DB_SCALING_S3	17.5682		// 3 S unit (18 dB)/division scaling

#define	DEFAULT_AUDIO_GAIN	16		// Default audio gain

#define	SPECTRUM_FILTER_MAX			20	// maximum filter setting
#define SPECTRUM_FILTER_DEFAULT		4	// default filter setting
//
// Enumeration to select which waterfall palette to use
//
enum	{
	WFALL_GRAY = 0,
	WFALL_HOT_COLD,
	WFALL_RAINBOW,
	WFALL_MAXVAL
};
//
#define	WATERFALL_COLOR_MIN			0
#define WATERFALL_COLOR_MAX			WFALL_MAXVAL
#define	WATERFALL_COLOR_DEFAULT		WFALL_GRAY
//
#define	WATERFALL_STEP_SIZE_MIN	1
#define	WATERFALL_STEP_SIZE_MAX	5
#define	WATERFALL_STEP_SIZE_DEFAULT	2
//
#define	WATERFALL_OFFSET_MIN	60
#define	WATERFALL_OFFSET_MAX	140
#define	WATERFALL_OFFSET_DEFAULT	100
//
#define	WATERFALL_CONTRAST_MIN	10
#define	WATERFALL_CONTRAST_MAX	225
#define	WATERFALL_CONTRAST_DEFAULT	120
//
#define	WATERFALL_SPEED_MIN		1
#define	WATERFALL_SPEED_MAX		30
#define	WATERFALL_SPEED_DEFAULT_PARALLEL	10
//
#define WATERFALL_NOSIG_ADJUST_MIN		10
#define WATERFALL_NOSIG_ADJUST_MAX		30
#define	WATERFALL_NOSIG_ADJUST_DEFAULT	20
//
// The following include warnings and settings for SPI interfaces, which needs less frequent updates or else the screen update will make button/dial response very sluggish!
//
#define WATERFALL_SPEED_DEFAULT_SPI		15
//
#define	WATERFALL_SPEED_WARN_SPI		10
#define	WATERFALL_SPEED_WARN1_SPI		14
//
// "faster" than this can make knobs/buttons sluggish with parallel
//
#define	WATERFALL_SPEED_WARN_PARALLEL	5
#define WATERFALL_SPEED_WARN1_PARALLEL  9
//
// Constants for waterfall size settings
//
enum	{
	WATERFALL_NORMAL=0,
	WATERFALL_MEDIUM,
	WATERFALL_MAX
};
//
enum	{
	FFT_WINDOW_RECTANGULAR=0,
	FFT_WINDOW_COSINE,
	FFT_WINDOW_BARTLETT,
	FFT_WINDOW_WELCH,
	FFT_WINDOW_HANN,
	FFT_WINDOW_HAMMING,
	FFT_WINDOW_BLACKMAN,
	FFT_WINDOW_NUTTALL,
	FFT_WINDOW_MAX
};
//
#define	WATERFALL_SIZE_DEFAULT	WATERFALL_NORMAL
#define	WFALL_MEDIUM_ADDITIONAL	12					// additional vertical height in pixels of "medium" waterfall
//
#define SWR_SAMPLES_SKP						1	//5000
#define SWR_SAMPLES_CNT						5	//10
//
#define	SD_DB_DIV_SCALING					0.0316	// Scaling factor for number of dB/Division	0.0316 = 10dB/Division

#define	MAX_RF_GAIN							50		// Maximum RF gain setting
#define	DEFAULT_RF_GAIN						50		// Default RF gain setting

//
// The following are used in the max volume setting in the menu system
//
#define	MAX_VOLUME_MIN						8		// Minimum setting for maximum volume
#define	MAX_VOLUME_MAX						MAX_AUDIO_GAIN		// Maximum setting for maximum volume
#define	MAX_VOLUME_DEFAULT					DEFAULT_AUDIO_GAIN

// Power supply
typedef struct PowerMeter
{
	ulong	skip;

	ulong	pwr_aver;
	uchar	p_curr;

	uchar	v10;
	uchar	v100;
	uchar	v1000;
	uchar	v10000;

} PowerMeter;

//
// Volt (DC power) meter
//
#define POWER_SAMPLES_SKP				1500	//1500
#define POWER_SAMPLES_CNT				32
//
// used to limit the voltmeter calibration parameters
//
#define	POWER_VOLTMETER_CALIBRATE_DEFAULT		100
#define	POWER_VOLTMETER_CALIBRATE_MIN			00
#define	POWER_VOLTMETER_CALIBRATE_MAX			200
//
#define	VOLTMETER_ADC_FULL_SCALE		4095

// Exports

void ui_driver_init(void);
void UiCalcRxIqGainAdj(void);
void UiDriverInitSpectrumDisplay(void);
void UiDriverReDrawSpectrumDisplay(void);
void UiDriverHandleSmeter(void);
bool UiDriverCheckFrequencyEncoder(void);
void UiDriverUpdateLcdFreq(ulong dial_freq, ushort color);
void UiDriverCreateDesktop(void);
void UiDriverInitFrequency(void);
void UiDriverShowMode(void);
void UiDriverShowFilter(uchar filter_id);
void UiDriverChangeTunningStep(uchar is_up);
void UiDriverShowStep(ulong step);
void UiDriverUpdateFrequencyFast(void);
void UiDriverChangeBand(uchar is_up);
void UiDriverChangeBandFilter(uchar band,uchar bpf_only);
void UiDriverUpdateFrequency(char skip_encoder_check);
void UiDriverChangeDSPMode(void);
void UiCalcNB_AGC(void);
void UiDriverChangeCodecGain(uchar enabled);
void UiDriverChangeRfGain(uchar enabled);
void UiDriverReDrawWaterfallDisplay(void);
void UiCalcRxPhaseAdj(void);
void UiInitSpectrumScopeWaterfall(void);
void UiDriverChangeAfGain(uchar enabled);
void UiDriverChangeRfGain(uchar enabled);
void ui_driver_init(void) ;
void UiDriverDisplayFilterBW(void);
void UiDriverCheckEncoderTwo(void);
void UiDrawSpectrumScopeFrequencyBarText(void);
void UiDriverHandlePowerSupply(void);
void UiDriverChangeVariable_1(uchar enabled, char variable, char *label);
void UiDriverChangeVariable_2(uchar enabled, char variable, char *label);
void UiCalcAGCDecay(void);
void UiDriverIndicadorFiltro(char *text);
void UiDriverChangeFilter(uchar ui_only_update);
void UiCalcRFGain(void);

#endif
