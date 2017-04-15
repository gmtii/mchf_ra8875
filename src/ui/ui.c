#include "radio.h"
#include "ui_ra8875.h"

#include "waterfall_colours.h"

#include "filters/q_rx_filter_10kHz.h"
#include "filters/i_rx_filter_10kHz.h"
//
#include "filters/q_rx_filter_7kHz5.h"
#include "filters/i_rx_filter_7kHz5.h"
//
#include "filters/q_rx_filter_6kHz.h"
#include "filters/i_rx_filter_6kHz.h"
//
#include "filters/q_rx_filter_5kHz.h"
#include "filters/i_rx_filter_5kHz.h"
//
#include "filters/q_rx_filter_3k6.h"
#include "filters/i_rx_filter_3k6.h"
//
#include "filters/q_tx_filter.h"
#include "filters/i_tx_filter.h"

#include "filters/iq_rx_filter_am_10kHz.h"
#include "filters/iq_rx_filter_am_7kHz5.h"
#include "filters/iq_rx_filter_am_6kHz.h"
#include "filters/iq_rx_filter_am_5kHz.h"
#include "filters/iq_rx_filter_am_3k6.h"
#include "filters/iq_rx_filter_am_2k3.h"

//
extern volatile FilterCoeffs fc;

// ------------------------------------------------
// Encoder two public
volatile EncoderTwoSelection ews;

// Power supply meter
volatile PowerMeter					pwmt;

extern volatile Si5351Variables			vfo;

//
// RX Hilbert transform (90 degree) FIR filter state tables and instances
//
static float32_t FirState_I[128];
extern volatile arm_fir_instance_f32 FIR_I;
//
static float32_t FirState_Q[128];
extern volatile arm_fir_instance_f32 FIR_Q;

const ulong tune_steps[MAX_STEPS] = {
T_STEP_1HZ,
T_STEP_10HZ,
T_STEP_100HZ,
T_STEP_1KHZ,
T_STEP_5KHZ,
T_STEP_10KHZ,
T_STEP_100KHZ };

//
// Bands tuning values
volatile ulong band_dial_value[MAX_BANDS + 1];
volatile ulong band_decod_mode[MAX_BANDS + 1];
volatile ulong band_filter_mode[MAX_BANDS + 1];

// Band definitions - band base frequency value
const ulong tune_bands[MAX_BANDS] = { BAND_FREQ_80,
BAND_FREQ_60,
BAND_FREQ_40,
BAND_FREQ_30,
BAND_FREQ_20,
BAND_FREQ_17,
BAND_FREQ_15,
BAND_FREQ_12,
BAND_FREQ_10 };

// Band definitions - band frequency size
const ulong size_bands[MAX_BANDS] = { BAND_SIZE_80,
BAND_SIZE_60,
BAND_SIZE_40,
BAND_SIZE_30,
BAND_SIZE_20,
BAND_SIZE_17,
BAND_SIZE_15,
BAND_SIZE_12,
BAND_SIZE_10 };

// Spectrum display public
extern volatile SpectrumDisplay sd;

// Audio driver publics
extern volatile AudioDriverState ads;

// S meter public
extern volatile SMeter sm;

// Radio driver public
extern volatile TransceiverState ts;

extern DialFrequency df;

// The following are calibrations for the S-meter based on 6 dB per S-unit, 10 dB per 10 dB mark above S-9
// The numbers within are linear gain values, not logarithmic, starting with a zero signal level of 1
// There are 33 entries, one corresponding with each point on the S-meter
#define	S_Meter_Cal_Size	33	// number of entries in table below
const float S_Meter_Cal[] = {
// - Dummy variable		1,		//0, S0, 0dB
		14.1,//1.41,	//1, S0.5, 3dB
		20,		//2,		//2, S1, 6dB
		28.1,	//2.81,	//3, S1.5, 9dB
		30,		//3,		//4, S2, 12dB
		56.2,	//5.62,	//5, S2.5, 15dB
		79.4,	//7.94,	//6, S3, 18dB
		112.2,	//11.22,	//7, S3.5, 21dB
		158.5,	//15.85,	//8, S4, 24dB
		223.9,	//22.39,	//9, S4.5, 27dB
		316.3,	//31.63,	//10, S5, 30dB
		446.7,	//44.67,	//11, S5.5, 33dB
		631,	//63.10,	//12, S6, 36dB
		891.3,	//89.13,	//13, S6.5, 39dB
		1258.9,	//125.89,	//14, S7, 42dB
		1778.3,	//177.83,	//15, S7.5, 45dB
		2511.9,	//251.19,	//16, S8, 48dB
		3548.1,	//354.81,	//17, S8.5, 51dB
		5011.9,	//501.19,	//18, S9, 54dB
		8912.5,	//891.25,	//19, +5, 59dB
		15848.9,	//1584.89,	//20, +10, 64dB
		28183.8,	//2818.38,	//21, +15, 69dB
		50118.7,	//5011.87,	//22, +20, 74dB
		89125.1,	//8912.51,	//23, +25, 79dB
		158489.3,	//15848.93,	//24, +30, 84dB
		281838.2,	//28183.82,	//25, +35, 89dB
		501187.2,	//50118.72,	//26, +35, 94dB
		891250.9,	//89125.09,	//27, +40, 99dB
		1585893.2,	//158489.32,	//28, +45, 104dB
		2818382.9,	//281838.29,	//29, +50, 109dB
		5011872.3,	//501187.23,	//30, +55, 114dB
		8912509.4,	//891250.94,	//31, +60, 119dB
		15848931.9,	//1584893.19,	//32, +65, 124dB
		28183829.3,	//2818382.93	//33, +70, 129dB
		};

void UiDriverFFTWindowFunction(char mode);
bool UiDriverCheckFrequencyEncoder(void);

static void UiDriverDeleteSMeter(void);
static void UiDriverCreateSMeter(void);
static void UiDriverDrawWhiteSMeter(void);
static void UiDriverDrawRedSMeter(void);
//
static void UiDriverUpdateTopMeterA(uchar val, uchar old);
static void UiDriverUpdateBtmMeter(uchar val, uchar warn);
void UiDriverChangeBandFilter(uchar band,uchar bpf_only);

void UiDriverDrawWhiteSMeter(void);
void UiDriverDrawRedSMeter(void);

void ui_driver_init(void) {
	short res;

	// Clear display
	//LCD_LcdClear(Black);

	// Init frequency publics
	UiDriverInitFrequency();

	//
	UiCalcRxIqGainAdj();		// Init RX IQ gain
	//
	UiCalcRxPhaseAdj();			// Init RX IQ Phase (Hilbert transform)

	//
	// Init spectrum display
	UiDriverInitSpectrumDisplay();
	//UiInitSpectrumScopeWaterfall();
	sd.display_offset = INIT_SPEC_AGC_LEVEL;// initialize setting for display offset/AGC

	// Read SI570 settings
	//res = ui_si570_get_configuration();
	//if (res != 0) {
	//	//printf("err I2C: %d\n\r",res);
	//}

	// Create desktop screen
	UiDriverCreateDesktop();
	UiDriverDisplayFilterBW();// Update on-screen indicator of filter bandwidth

	// Update codec volume
	//  0 - 16: via codec command
	// 17 - 30: soft gain after decoder
	Codec_Volume(ts.audio_gain);// This is only approximate - it will be properly set later

	UiRotaryFreqEncoderInit();
	UiRotaryEncoderTwoInit();
	//UiRotaryEncoderThreeInit();   // Brokes SPI pins

	UiCalcRFGain();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UiCalcRFGain(void) {
	float tcalc;// temporary value as "ads.agc_rf_gain" may be used during the calculation!

	// calculate working RF gain value
	tcalc = (float) ts.rf_gain;
	tcalc *= 1.4;
	tcalc -= 20;
	tcalc /= 10;
	ads.agc_rf_gain = powf(10, tcalc);

}

void UiCalcRxPhaseAdj(void)
{
	float f_coeff, f_offset, var_norm, var_inv;
	ulong i;
	int phase;
	//
	// always make a fresh copy of the original Q and I coefficients
	// NOTE:  We are assuming that the I and Q filters are of the same length!
	//
	fc.rx_q_num_taps = Q_NUM_TAPS;
	fc.rx_i_num_taps = I_NUM_TAPS;
	//
	fc.rx_q_block_size = Q_BLOCK_SIZE;
	fc.rx_i_block_size = I_BLOCK_SIZE;
	//
	if(ts.dmod_mode == DEMOD_AM)	{		// AM - load low-pass, non Hilbert filters (e.g. no I/Q phase shift
		if(ts.filter_id == AUDIO_WIDE)	{	// Wide AM - selectable from menu
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				switch(ts.filter_wide_select)	{
					case WIDE_FILTER_5K:
					case WIDE_FILTER_5K_AM:
						fc.rx_filt_q[i] = iq_rx_am_5k_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_5k_coeffs[i];
						break;
					case WIDE_FILTER_6K:
					case WIDE_FILTER_6K_AM:
						fc.rx_filt_q[i] = iq_rx_am_6k_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_6k_coeffs[i];
						break;
					case WIDE_FILTER_7K5:
					case WIDE_FILTER_7K5_AM:
						fc.rx_filt_q[i] = iq_rx_am_7k5_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_7k5_coeffs[i];
						break;
					case WIDE_FILTER_10K:
					case WIDE_FILTER_10K_AM:
					default:
						fc.rx_filt_q[i] = iq_rx_am_10k_coeffs[i];
						fc.rx_filt_i[i] = iq_rx_am_10k_coeffs[i];
						break;
				}
			}
		}
		else if((ts.filter_id == AUDIO_3P6KHZ) )	{	// "Medium" AM - "3.6" kHz filter (total of 7.2 kHz bandwidth) - or if we are using FM
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				fc.rx_filt_q[i] = iq_rx_am_3k6_coeffs[i];
				fc.rx_filt_i[i] = iq_rx_am_3k6_coeffs[i];
			}
		}
		else	{
			for(i = 0; i < Q_NUM_TAPS; i++)	{		// "Narrow" AM - "1.8" kHz filter (total of 3.6 kHz bandwidth)
				fc.rx_filt_q[i] = iq_rx_am_2k3_coeffs[i];
				fc.rx_filt_i[i] = iq_rx_am_2k3_coeffs[i];
			}
		}
	}

	else	{		// Not AM or FM - load Hilbert transformation filters
		if(ts.filter_id == AUDIO_WIDE)	{
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				switch(ts.filter_wide_select)	{
					case WIDE_FILTER_5K:
					case WIDE_FILTER_5K_AM:
						fc.rx_filt_q[i] = q_rx_5k_coeffs[i];
						fc.rx_filt_i[i] = i_rx_5k_coeffs[i];
						break;
					case WIDE_FILTER_6K:
					case WIDE_FILTER_6K_AM:
						fc.rx_filt_q[i] = q_rx_6k_coeffs[i];
						fc.rx_filt_i[i] = i_rx_6k_coeffs[i];
						break;
					case WIDE_FILTER_7K5:
					case WIDE_FILTER_7K5_AM:
						fc.rx_filt_q[i] = q_rx_7k5_coeffs[i];
						fc.rx_filt_i[i] = i_rx_7k5_coeffs[i];
						break;
					case WIDE_FILTER_10K:
					case WIDE_FILTER_10K_AM:
					default:
						fc.rx_filt_q[i] = q_rx_10k_coeffs[i];
						fc.rx_filt_i[i] = i_rx_10k_coeffs[i];
						break;
				}

			}
		}
		else	{
			for(i = 0; i < Q_NUM_TAPS; i++)	{
				fc.rx_filt_q[i] = q_rx_3k6_coeffs[i];
				fc.rx_filt_i[i] = i_rx_3k6_coeffs[i];	// phase shift in other modes
			}
		}
		//
		if(ts.dmod_mode == DEMOD_LSB)	// get phase setting appropriate to mode
			phase = ts.rx_iq_lsb_phase_balance;		// yes, get current gain adjustment setting for LSB
		else
			phase = ts.rx_iq_usb_phase_balance;		// yes, get current gain adjustment setting for USB and other mdoes
		//
		if(phase != 0)	{	// is phase adjustment non-zero?
			var_norm = (float)phase;
			var_norm = fabs(var_norm);		// get absolute value of this gain adjustment
			var_inv = 32 - var_norm;		// calculate "inverse" of number of steps
			var_norm /= 32;		// fractionalize by the number of steps
			var_inv /= 32;						// fractionalize this one, too
			if(phase < 0)	{	// was the phase adjustment negative?
				if(ts.filter_id == AUDIO_WIDE)	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						switch(ts.filter_wide_select)	{
							case WIDE_FILTER_5K:
							case WIDE_FILTER_5K_AM:
								f_coeff = var_inv * q_rx_5k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_5k_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
							case WIDE_FILTER_6K:
							case WIDE_FILTER_6K_AM:
								f_coeff = var_inv * q_rx_6k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_6k_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
							case WIDE_FILTER_7K5:
							case WIDE_FILTER_7K5_AM:
								f_coeff = var_inv * q_rx_7k5_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_7k5_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
							case WIDE_FILTER_10K:
							case WIDE_FILTER_10K_AM:
							default:
								f_coeff = var_inv * q_rx_10k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_10k_coeffs_minus[i];	// get fraction of 89.5 degree setting
								break;
						}
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
				else	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						f_coeff = var_inv * q_rx_3k6_coeffs[i];	// get fraction of 90 degree setting
						f_offset = var_norm * q_rx_3k6_coeffs_minus[i];	// get fraction of 89.5 degree setting
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
			}
			else	{							// adjustment was positive
				if(ts.filter_id == AUDIO_WIDE)	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						switch(ts.filter_wide_select)	{
							case WIDE_FILTER_5K:
							case WIDE_FILTER_5K_AM:
								f_coeff = var_inv * q_rx_5k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_5k_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
							case WIDE_FILTER_6K:
							case WIDE_FILTER_6K_AM:
								f_coeff = var_inv * q_rx_6k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_6k_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
							case WIDE_FILTER_7K5:
							case WIDE_FILTER_7K5_AM:
								f_coeff = var_inv * q_rx_7k5_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_7k5_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
							case WIDE_FILTER_10K:
							case WIDE_FILTER_10K_AM:
							default:
								f_coeff = var_inv * q_rx_10k_coeffs[i];	// get fraction of 90 degree setting
								f_offset = var_norm * q_rx_10k_coeffs_plus[i];	// get fraction of 90.5 degree setting
								break;
						}
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
				else	{
					for(i = 0; i < Q_NUM_TAPS; i++)	{
						f_coeff = var_inv * q_rx_3k6_coeffs[i];	// get fraction of 90 degree setting
						f_offset = var_norm * q_rx_3k6_coeffs_plus[i];	// get fraction of 90.5 degree setting
						fc.rx_filt_q[i] = f_coeff + f_offset;	// synthesize new coefficient
					}
				}
			}
		}
	}
	//
	// In AM mode we do NOT do 90 degree phase shift, so we do FIR low-pass instead of Hilbert, setting "I" channel the same as "Q"
	if(ts.dmod_mode == DEMOD_AM)		// use "Q" filter settings in AM mode for "I" channel
		arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_q_block_size);	// load "I" with "Q" coefficients
	else								// not in AM mode, but SSB or FM - use normal settings where I and Q are 90 degrees apart
		arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_I,fc.rx_i_num_taps,(float32_t *)&fc.rx_filt_i[0], &FirState_I[0],fc.rx_i_block_size);	// load "I" with "I" coefficients
	//
	arm_fir_init_f32((arm_fir_instance_f32 *)&FIR_Q,fc.rx_q_num_taps,(float32_t *)&fc.rx_filt_q[0], &FirState_Q[0],fc.rx_q_block_size);		// load "Q" with "Q" coefficients
	//
}

void UiCalcRxIqGainAdj(void) {
	if (ts.dmod_mode == DEMOD_AM)
		ts.rx_adj_gain_var_i = (float) ts.rx_iq_am_gain_balance;// get current gain adjustment for AM
	else if (ts.dmod_mode == DEMOD_LSB)
		ts.rx_adj_gain_var_i = (float) ts.rx_iq_lsb_gain_balance;// get current gain adjustment setting for LSB
	else
		ts.rx_adj_gain_var_i = (float) ts.rx_iq_usb_gain_balance;// get current gain adjustment setting	USB and other modes
	//
	ts.rx_adj_gain_var_i /= 1024;		// fractionalize it
	ts.rx_adj_gain_var_q = -ts.rx_adj_gain_var_i;		// get "invert" of it
	ts.rx_adj_gain_var_i += 1;		// offset it by one (e.g. 0 = unity)
	ts.rx_adj_gain_var_q += 1;
}

void UiCalcNB_AGC(void) {
	float temp_float;

	temp_float = (float) ts.nb_agc_time_const;	// get user setting
	temp_float = NB_MAX_AGC_SETTING - temp_float;		// invert (0 = minimum))
	temp_float /= 1.1;								// scale calculation
	temp_float *= temp_float;						// square value
	temp_float += 1;								// offset by one
	temp_float /= 44000;							// rescale
	temp_float += 1;							// prevent negative log result
	ads.nb_sig_filt = log10f(temp_float);// de-linearize and save in "new signal" contribution parameter
	ads.nb_agc_filt = 1 - ads.nb_sig_filt;// calculate parameter for recyling "old" AGC value
}

void UiDriverCreateFunctionButtons(bool full_repaint) {
	char cap1[20], cap2[20], cap3[20], cap4[20], cap5[20],cap6[20], cap7[20], cap8[20];

	// Create bottom bar
	if (full_repaint) {
		LCD_DrawBottomButton((POS_BOTTOM_BAR_X + 0),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				POS_BOTTOM_BAR_BUTTON_W, Grey);
		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 1 + 2),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				POS_BOTTOM_BAR_BUTTON_W, Grey);
		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 2 + 4),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				POS_BOTTOM_BAR_BUTTON_W, Grey);
		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 3 + 6),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				POS_BOTTOM_BAR_BUTTON_W, Grey);
		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 4 + 8),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				(POS_BOTTOM_BAR_BUTTON_W + 1), Grey);
		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 5 + 8),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				(POS_BOTTOM_BAR_BUTTON_W + 1), Grey);

		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 6 + 8),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				(POS_BOTTOM_BAR_BUTTON_W + 1), Grey);

		LCD_DrawBottomButton(
				(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W * 7 + 8),
				(POS_BOTTOM_BAR_Y - 4), POS_BOTTOM_BAR_BUTTON_H,
				(POS_BOTTOM_BAR_BUTTON_W + 1), Grey);
	}

	strcpy(cap1, "80 M");
	strcpy(cap2, "60 M");
	strcpy(cap3, "40 M");
	strcpy(cap4, "30 M");
	strcpy(cap5, "20 M");
	strcpy(cap6, "15 M");
	strcpy(cap7, "11 M");
	strcpy(cap8, "10 M");



	// Draw buttons text
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F1_X, POS_BOTTOM_BAR_F1_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap1, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F2_X, POS_BOTTOM_BAR_F2_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap2, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F3_X, POS_BOTTOM_BAR_F3_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap3, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F4_X, POS_BOTTOM_BAR_F4_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap4, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F5_X, POS_BOTTOM_BAR_F5_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap5, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F6_X, POS_BOTTOM_BAR_F6_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap6, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F7_X, POS_BOTTOM_BAR_F7_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap7, White,
			Black, 2);
	LCD_PrintTextCentered(POS_BOTTOM_BAR_F8_X, POS_BOTTOM_BAR_F8_Y,POS_BOTTOM_BAR_BUTTON_W-2,cap8, White,
			Black, 2);
}

void UiDriverChangeBand(uchar is_up) {
	ulong curr_band_index;	// index in band table of currently selected band
	ulong new_band_index;		// index of the new selected band

	ulong new_band_freq;		// new dial frequency

	curr_band_index = ts.band_mode;

	// Save old band values
	if (curr_band_index < (MAX_BANDS - 1)) {
		// Save dial
		band_dial_value[curr_band_index] = df.tune_old;

		// Save decode mode
		band_decod_mode[curr_band_index] = ts.dmod_mode;

	}

	// Handle direction
	if (is_up) {
		if (curr_band_index < (MAX_BANDS - 1)) {
			// Increase
			new_band_freq = tune_bands[curr_band_index + 1];
			new_band_index = curr_band_index + 1;
		} else
			return;
	} else {
		if (curr_band_index) {
			// Decrease
			new_band_freq = tune_bands[curr_band_index - 1];
			new_band_index = curr_band_index - 1;
		} else
			return;
	}

	// Load frequency value - either from memory or default for
	// the band if this is first band selection
	if (band_dial_value[new_band_index] != 0xFFFFFFFF) {
		//printf("load value from memory\n\r");

		// Load old frequency from memory
		df.tune_new = band_dial_value[new_band_index];
	} else {
		//printf("load default band freq\n\r");

		// Load default band startup frequency
		df.tune_new = new_band_freq;
	}

	df.transv_freq = 0;

	// Display frequency update
	//UiDriverUpdateFrequency(1);

	// Also reset second freq display
	//UiDriverUpdateSecondLcdFreq(df.tune_new/4);

	// Change decode mode if need to
	if (ts.dmod_mode != band_decod_mode[new_band_index]) {
		// Update mode
		ts.dmod_mode = band_decod_mode[new_band_index];

		// Update Decode Mode (USB/LSB/AM/FM/CW)
		UiDriverShowMode();
	}

	// Create Band value
	UiDriverShowBand(new_band_index);

	// Set filters
	UiDriverChangeBandFilter(new_band_index,0);

	// Finally update public flag
	ts.band_mode = new_band_index;

}

void UiDriverChangeTunningStep(uchar is_up) {
	ulong idx = df.selected_idx;
	ulong dial_freq;

	if (is_up) {
		// Increase step index or reset
		if (idx < (MAX_STEPS - 1))
			idx++;
		else
			idx = 0;
	} else {
		// Decrease step index or reset
		if (idx)
			idx--;
		else
			idx = (MAX_STEPS - 1);
	}

	// Update publics
	df.tuning_step = tune_steps[idx];
	df.selected_idx = idx;

	//printf("step_n: %d\n\r",  df.tuning_step);

	// Update step on screen
	UiDriverShowStep(idx);

	// Save to Eeprom
	//TRX4M_VE_WriteStep(idx);
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeTunningStep
//* Object              : Change tunning step
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*
 Sel 1 (R12)	Sel 0 (R13)	Band	Low HF  MHz	High HF  MHz
 L			L			0			1 			4			BAND_MODE_80
 L			H			1			4 			8			BAND_MODE_60, BAND_MODE_40
 H			L			2			8 			16			BAND_MODE_30, BAND_MODE_20
 H			H			3			16 			30			BAND_MODE_17, BAND_MODE_15, BAND_MODE_12, BAND_MODE_10
 */

void UiDriverChangeBandFilter(uchar band,uchar bpf_only) {
	// ---------------------------------------------
	// Set BPFs
	// Constant line states for the BPF filter,
	// always last - after LPF change
	switch (band) {
	case BAND_MODE_80: {
		GPIO_ResetBits(FL_GPIO, FL_SEL1);
		GPIO_ResetBits(FL_GPIO, FL_SEL0);
		non_os_delay();
		break;
	}

	case BAND_MODE_60:
	case BAND_MODE_40: {
		GPIO_SetBits(FL_GPIO, FL_SEL1);
		GPIO_ResetBits(FL_GPIO, FL_SEL0);
		non_os_delay();
		break;
	}

	case BAND_MODE_30:
	case BAND_MODE_20: {
		GPIO_ResetBits(FL_GPIO, FL_SEL1);
		GPIO_SetBits(FL_GPIO, FL_SEL0);
		non_os_delay();
		break;
	}

	case BAND_MODE_17:
	case BAND_MODE_15:
	case BAND_MODE_12:
	case BAND_MODE_10: {
		GPIO_SetBits(FL_GPIO, FL_SEL1);
		GPIO_SetBits(FL_GPIO, FL_SEL0);
		non_os_delay();
		break;
	}

	default:
		GPIO_ResetBits(FL_GPIO, FL_SEL1);
		GPIO_SetBits(FL_GPIO, FL_SEL0);
		non_os_delay();
		break;
	}
}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckFilter
//* Object              : Sets the filter appropriate for the currently-tuned frequency
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCheckFilter(ulong freq)
{

#define	BAND_FILTER_UPPER_80		4250000				// Upper limit for 80 meter filter
#define	BAND_FILTER_UPPER_40		8000000				// Upper limit for 40/60 meter filter
#define	BAND_FILTER_UPPER_20		16000000			// Upper limit for 20/30 meter filter


#define	FILTER_BAND_80					1
#define	FILTER_BAND_40					2
#define	FILTER_BAND_20					3
#define FILTER_BAND_15					4


	if(freq < BAND_FILTER_UPPER_80)	{	// are we low enough if frequency for the 80 meter filter?
		if(ts.filter_band != FILTER_BAND_80)	{
			UiDriverChangeBandFilter(BAND_MODE_80, 0);	// yes - set to 80 meters
			ts.filter_band = FILTER_BAND_80;
		}
	}
	else if(freq < BAND_FILTER_UPPER_40)	{
		if(ts.filter_band != FILTER_BAND_40)	{
			UiDriverChangeBandFilter(BAND_MODE_40, 0);	// yes - set to 40 meters
			ts.filter_band = FILTER_BAND_40;
		}
	}
	else if(freq < BAND_FILTER_UPPER_20)	{
		if(ts.filter_band != FILTER_BAND_20)	{
			UiDriverChangeBandFilter(BAND_MODE_20, 0);	// yes - set to 20 meters
			ts.filter_band = FILTER_BAND_20;
		}
	}
	else if(freq >= FILTER_BAND_15)	{
		if(ts.filter_band != FILTER_BAND_15)	{
			UiDriverChangeBandFilter(BAND_MODE_10, 0);	// yes - set to 10 meters
			ts.filter_band = FILTER_BAND_15;
		}
	}

}

void UiDriverChangeAfGain(uchar enabled) {
	ushort color = Grey;
	char temp[100];

	if (enabled)
		color = White;

	LCD_DrawEmptyRect( POS_AG_IND_X, POS_AG_IND_Y, 13, 57, Grey);

	if (enabled)
		LCD_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1), "AFG",
				Black, Grey, 0);
	else
		LCD_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1), "AFG",
				Grey1, Grey, 0);

	sprintf(temp, "%02d", ts.audio_gain);
	LCD_PrintText((POS_AG_IND_X + 38), (POS_AG_IND_Y + 1), temp, color,
			Black, 0);
}

void UiDriverChangeCodecGain(uchar enabled) {
	ushort color = White;
	char temp[100];

	LCD_DrawEmptyRect( POS_SG_IND_X, POS_SG_IND_Y, 13, 49, Grey);

	if (enabled)
		LCD_PrintText((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1), "COG",
				Black, Grey, 0);
	else
		LCD_PrintText((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1), "COG",
				Grey1, Grey, 0);

	sprintf(temp, "%02d", ts.rf_codec_gain);
	LCD_PrintText((POS_SG_IND_X + 30), (POS_SG_IND_Y + 1), temp, color,
			Black, 0);

	if (enabled == 0)		// always display grey if disabled
		color = Grey;

	LCD_PrintText((POS_SG_IND_X + 30), (POS_SG_IND_Y + 1), temp, color,
			Black, 0);
}

void UiDriverChangeRfGain(uchar enabled) {
	ushort color = Grey;
	char temp[100];

	if (enabled)
		color = White;

	LCD_DrawEmptyRect( POS_RF_IND_X, POS_RF_IND_Y, 13, 57, Grey);

	if (enabled) {
		LCD_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1), "RFG",
		Black, Grey, 0);
		//
		// set color as warning that RX sensitivity is reduced
		//
		if (ts.rf_gain < 20)
			color = Red;
		else if (ts.rf_gain < 30)
			color = Orange;
		else if (ts.rf_gain < 40)
			color = Yellow;
	} else
		LCD_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1), "RFG",
		Grey1, Grey, 0);

	sprintf(temp, " %02d", ts.rf_gain);
	LCD_PrintText((POS_RF_IND_X + 32), (POS_RF_IND_Y + 1), temp, color,
	Black, 0);

}

bool UiDriverCheckFrequencyEncoder(void) {
	int pot_diff;

	// Load pot value
	df.value_new = TIM_GetCounter(TIM8);

	// Ignore lower value flickr
	if (df.value_new < ENCODER_FLICKR_BAND)
		return false;

	// Ignore higher value flickr
	if (df.value_new
			> (FREQ_ENCODER_RANGE / FREQ_ENCODER_LOG_D) + ENCODER_FLICKR_BAND)
		return false;

	// No change, return
	if (df.value_old == df.value_new)
		return false;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	df.de_detent++;
	if (df.de_detent < USE_DETENTED_VALUE) {
		df.value_old = df.value_new; // update and skip
		return false;
	}
	df.de_detent = 0;
#endif

	// Encoder value to difference
	if (df.value_new > df.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	// Finaly convert to frequency incr/decr
	if (pot_diff < 0)
		df.tune_new -= (df.tuning_step * 4);
	else
		df.tune_new += (df.tuning_step * 4);

	// Updated
	df.value_old = df.value_new;

	return true;
}

void UiDriverCheckEncoderTwo(void) {

	int pot_diff;
	char temp[20];

	ews.value_new = TIM_GetCounter(TIM4);

	// Ignore lower value flickr
	if (ews.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore lower value flickr
	if (ews.value_new
			> (ENCODER_TWO_RANGE / ENCODER_TWO_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if (ews.value_old == ews.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	ews.de_detent++;
	if (ews.de_detent < USE_DETENTED_VALUE) {
		ews.value_old = ews.value_new; // update and skip
		return;
	}
	ews.de_detent = 0;
#endif

	//printf("gain pot: %d\n\r",gs.value_new);

	// Encoder value to difference
	if (ews.value_new > ews.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	if (ts.menu_mode) {
		switch (ts.menu_item) {
		case 0: {

			if (pot_diff < 0) {
				if (ts.rf_gain)
					ts.rf_gain -= 1;
			} else {
				ts.rf_gain += 1;
				if (ts.rf_gain > MAX_RF_GAIN)
					ts.rf_gain = MAX_RF_GAIN;
			}
			//
			// get RF gain value and calculate new value
			//
			UiCalcRFGain();	// convert from user RF gain value to "working" RF gain value
			UiDriverChangeRfGain(1);	// change on screen

			break;
		}

		case 1:

			if (ts.dsp_active & 8) {	// is it in noise blanker mode?
				// Convert to NB incr/decr
				if (pot_diff < 0) {
					if (ts.nb_setting)
						ts.nb_setting -= 1;
				} else {
					ts.nb_setting += 1;
					if (ts.nb_setting > MAX_NB_SETTING)
						ts.nb_setting = MAX_NB_SETTING;
				}
			} else if (ts.dsp_active & 1) {	// only allow adjustment if DSP NR is active
				// Convert to NB incr/decr
				if (pot_diff < 0) {
					if (ts.dsp_nr_strength)
						ts.dsp_nr_strength -= 1;
				} else {
					ts.dsp_nr_strength += 1;
					if (ts.dsp_nr_strength > DSP_NR_STRENGTH_MAX)
						ts.dsp_nr_strength = DSP_NR_STRENGTH_MAX;
				}
				audio_driver_set_rx_audio_filter();
			}
			// Signal processor setting
			UiDriverChangeSigProc(1);
			break;

		case 2:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {
				if (ts.audio_gain)
					ts.audio_gain -= 1;
			} else {
				ts.audio_gain += 1;
				if (ts.audio_gain > 29) //ts.audio_max_volume)
					ts.audio_gain = 29; //ts.audio_max_volume;
			}
			Codec_Volume((ts.audio_gain));

			UiDriverChangeAfGain(1);

			break;

		case 3:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (!ts.nb_setting) {
					ts.nb_disable = 1;
					ts.dsp_active &= 0xF7;
				}

				if (ts.nb_setting)
					ts.nb_setting--;

			} else {

				if (!ts.nb_setting) {
					ts.dsp_active |= 8;
					ts.nb_disable = 0;
				}

				ts.nb_setting++;
				if (ts.nb_setting > MAX_NB_SETTING)
					ts.nb_setting = MAX_NB_SETTING;

			}
			UiDriverChangeSigProc(1);

			break;

		case 4:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {
				if (ts.rf_codec_gain)
					ts.rf_codec_gain -= 1;
			} else {
				ts.rf_codec_gain += 1;
				if (ts.rf_codec_gain > 9)
					ts.rf_codec_gain = 9;
			}
			UiDriverChangeCodecGain(1);

			break;

		case 5:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (ts.spectrum_scope_nosig_adjust > 10)
				ts.spectrum_scope_nosig_adjust-=1;
			} else {
				if (ts.spectrum_scope_nosig_adjust < 40)
				ts.spectrum_scope_nosig_adjust+=1;
			}
			UiDriverChangeVariable_1(1, ts.spectrum_scope_nosig_adjust, "NOI");
			break;

		case 6:

		// Convert to Audio Gain incr/decr

			switch(ts.filter_id)
			{

			case AUDIO_300HZ:

				if (pot_diff < 0) {

				if (ts.filter_300Hz_select > 1)
						ts.filter_300Hz_select-=1;
				}
				 else {
					if (ts.filter_300Hz_select < 10)
						ts.filter_300Hz_select+=1;
				}

				UiDriverChangeVariable_2(1, ts.filter_300Hz_select, "300");

				break;
			case AUDIO_500HZ:

				if (pot_diff < 0) {

				if (ts.filter_500Hz_select > 1)
						ts.filter_500Hz_select-=1;
				}
				 else {
					if (ts.filter_500Hz_select < 5)
						ts.filter_500Hz_select+=1;
				}

				UiDriverChangeVariable_2(1, ts.filter_500Hz_select, "500");

				break;
			case AUDIO_1P8KHZ:

				if (pot_diff < 0) {

				if (ts.filter_1k8_select > 1)
						ts.filter_1k8_select-=1;
				}
				 else {
					if (ts.filter_1k8_select < 4)
						ts.filter_1k8_select+=1;
				}

				UiDriverChangeVariable_2(1, ts.filter_1k8_select, "1k8");

				break;
			case AUDIO_2P3KHZ:

				if (pot_diff < 0) {

				if (ts.filter_2k3_select > 1)
						ts.filter_2k3_select-=1;
				}
				 else {
					if (ts.filter_2k3_select < 4)
						ts.filter_2k3_select+=1;
				}

				UiDriverChangeVariable_2(1, ts.filter_2k3_select, "2k3");

				break;
			case AUDIO_2P7KHZ:


				break;
			case AUDIO_2P9KHZ:


				break;
			case AUDIO_3P6KHZ:

				break;

			}



			audio_driver_set_rx_audio_filter();
			UiDriverDisplayFilterBW();
			break;


		case 7:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (ts.filter_wide_select > 0)
					ts.filter_wide_select-=1;
			} else {
				if (ts.filter_wide_select < 3)
					ts.filter_wide_select+=1;
			}
			UiDriverChangeVariable_2(1, ts.filter_wide_select, "AMW");

			UiDriverChangeFilter(0);
			UiCalcRxPhaseAdj();						// update Hilbert/LowPass filter setting
			UiDriverDisplayFilterBW();	// update on-screen filter bandwidth indicator
			break;

		case 8:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (ts.agc_mode > AGC_SLOW)
					ts.agc_mode-=1;
			} else {
				if (ts.agc_mode < AGC_MAX_MODE)
					ts.agc_mode+=1;
			}
			UiDriverChangeVariable_2(1, ts.agc_mode, "AGC");

			switch (ts.agc_mode)
			{
			case AGC_SLOW:
				UiDriverIndicadorFiltro("AGC_SLW");
				break;
			case AGC_MED:
				UiDriverIndicadorFiltro("AGC_MED");
				break;
			case AGC_FAST:
				UiDriverIndicadorFiltro("AGC_FST");
				break;
			case AGC_CUSTOM:
				UiDriverIndicadorFiltro("AGC_CUS");
				break;
			case AGC_OFF:
				UiDriverIndicadorFiltro("AGC_OFF");
				break;

			}
			UiCalcAGCDecay();
			break;

		case 9:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (ts.fft_window_type > FFT_WINDOW_RECTANGULAR)
					ts.fft_window_type-=1;
			} else {
				if (ts.fft_window_type < FFT_WINDOW_NUTTALL)
					ts.fft_window_type+=1;
			}
			UiDriverChangeVariable_2(1, ts.fft_window_type, "FFT");

			break;

		case 10:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (ts.scope_agc_rate > 0)
					ts.scope_agc_rate-=1;
			} else {
				if (ts.scope_agc_rate < 30)
					ts.scope_agc_rate+=1;
			}
			UiDriverChangeVariable_2(1, ts.scope_agc_rate, "S.G");

			break;

		case 11:

			// Convert to Audio Gain incr/decr
			if (pot_diff ) {

				if((!(ts.dsp_active & 1)) && (!(ts.dsp_active & 4)))	// both NR and notch are inactive
						ts.dsp_active |= 1;									// turn on NR
					else if((ts.dsp_active & 1) && (!(ts.dsp_active & 4))) {	// NR active, notch inactive
						if(ts.dmod_mode != DEMOD_CW)	{	// NOT in CW mode
							ts.dsp_active |= 4;									// turn on notch
							ts.dsp_active &= 0xfe;								// turn off NR
						}
						else	{	// CW mode - do not select notches, skip directly to "off"
							ts.dsp_active &= 0xfa;	// turn off NR and notch
						}
					}
					else if((!(ts.dsp_active & 1)) && (ts.dsp_active & 4))	//	NR inactive, notch active
						if((ts.dmod_mode == DEMOD_AM) && (ts.filter_id == AUDIO_WIDE))		// was it AM with a 10 kHz filter selected?

							ts.dsp_active &= 0xfa;			// it was AM + 10 kHz - turn off NR and notch
						else
							ts.dsp_active |= 1;				// no - turn on NR
					//
					else	{
						ts.dsp_active &= 0xfa;								// turn off NR and notch
					}
					//
					ts.dsp_active_toggle = ts.dsp_active;	// save update in "toggle" variable
					//
					ts.reset_dsp_nr = 1;				// reset DSP NR coefficients
					audio_driver_set_rx_audio_filter();	// update DSP settings
					ts.reset_dsp_nr = 0;
					UiDriverChangeDSPMode();			// update on-screen display

			}

			break;

		case 12:

			// Convert to Audio Gain incr/decr
			if (pot_diff < 0) {

				if (ts.spectrum_filter > 1)
					ts.spectrum_filter-=1;
			} else {
				if (ts.spectrum_filter < SPECTRUM_FILTER_MAX)
					ts.spectrum_filter+=1;
			}
			UiDriverChangeVariable_2(1, ts.spectrum_filter, "S.F");

			break;

		case 13:

			// Convert to Audio Gain incr/decr
			if (pot_diff) {

				sd.enabled^=1;
			}

			break;


		default:
			break;

		}
	}

	else
	{
		if (pot_diff < 0) {

		if (ts.rf_gain>0)
			ts.rf_gain -= 1;
		}
		else
		{

		if (ts.rf_gain < MAX_RF_GAIN)
			ts.rf_gain += 1;
		}

		//
		// get RF gain value and calculate new value
		//
		UiCalcRFGain();	// convert from user RF gain value to "working" RF gain value
		UiDriverChangeRfGain(1);	// change on screen
	}


	// Updated
	ews.value_old = ews.value_new;
}



void UiDriverChangeFilter(uchar ui_only_update) {

	ushort fcolor = White;

	LCD_PrintText(POS_FIR_IND_X,  POS_FIR_IND_Y,       "  FILT ",	White, 	Orange, 0);

	// Do a filter re-load
	if(!ui_only_update) {
		audio_driver_set_rx_audio_filter();
	}
	// Draw top line
	LCD_DrawStraightLine(POS_FIR_IND_X,(POS_FIR_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);

	const char* filter_ptr;

	switch(ts.filter_id)
		{
		case AUDIO_300HZ:
			filter_ptr = " 300Hz";
			break;
		case AUDIO_500HZ:
			filter_ptr = " 500Hz";
			break;
		case AUDIO_1P8KHZ:
			filter_ptr = "  1.8k";
			break;
		case AUDIO_2P3KHZ:
			filter_ptr = "  2.3k";
			break;
		case AUDIO_2P7KHZ:
			filter_ptr = "  2.7k";
			break;
		case AUDIO_2P9KHZ:
			filter_ptr = "  2.9k";
			break;
		case AUDIO_3P6KHZ:
			filter_ptr = "  3.6k";
			break;
		case AUDIO_WIDE:
			switch(ts.filter_wide_select)	{
			case WIDE_FILTER_5K:
			case WIDE_FILTER_5K_AM:
				filter_ptr = "   5k ";
				break;
			case WIDE_FILTER_6K:
			case WIDE_FILTER_6K_AM:
				filter_ptr = "   6k ";
				break;
			case WIDE_FILTER_7K5:
			case WIDE_FILTER_7K5_AM:
				filter_ptr = "  7.5k";
				break;
			case WIDE_FILTER_10K:
			case WIDE_FILTER_10K_AM:
			default:
				filter_ptr = "  10k ";
				break;
			}
			break;
			default:
				filter_ptr = "      ";
				break;
		}


	LCD_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),filter_ptr,fcolor,Black,0);

	UiDriverDisplayFilterBW();
}

void UiDriverCreateSpectrumScope(void) {
	ulong i, clr;
	char s[32];
	ulong slen;

	//
	// get grid colour of all but center line
	//
	if (ts.scope_grid_colour == SPEC_GREY)
		ts.scope_grid_colour_active = Grid;
	else if (ts.scope_grid_colour == SPEC_BLUE)
		ts.scope_grid_colour_active = Blue;
	else if (ts.scope_grid_colour == SPEC_RED)
		ts.scope_grid_colour_active = Red;
	else if (ts.scope_grid_colour == SPEC_MAGENTA)
		ts.scope_grid_colour_active = Magenta;
	else if (ts.scope_grid_colour == SPEC_GREEN)
		ts.scope_grid_colour_active = Green;
	else if (ts.scope_grid_colour == SPEC_CYAN)
		ts.scope_grid_colour_active = Cyan;
	else if (ts.scope_grid_colour == SPEC_YELLOW)
		ts.scope_grid_colour_active = Yellow;
	else if (ts.scope_grid_colour == SPEC_BLACK)
		ts.scope_grid_colour_active = Black;
	else if (ts.scope_grid_colour == SPEC_ORANGE)
		ts.scope_grid_colour_active = Orange;
	else if (ts.scope_grid_colour == SPEC_GREY2)
		ts.scope_grid_colour_active = Grey;
	else
		ts.scope_grid_colour_active = White;
	//
	//
	// Get color of center vertical line of spectrum scope
	//
	if (ts.scope_centre_grid_colour == SPEC_GREY)
		ts.scope_centre_grid_colour_active = Grid;
	else if (ts.scope_centre_grid_colour == SPEC_BLUE)
		ts.scope_centre_grid_colour_active = Blue;
	else if (ts.scope_centre_grid_colour == SPEC_RED)
		ts.scope_centre_grid_colour_active = Red;
	else if (ts.scope_centre_grid_colour == SPEC_MAGENTA)
		ts.scope_centre_grid_colour_active = Magenta;
	else if (ts.scope_centre_grid_colour == SPEC_GREEN)
		ts.scope_centre_grid_colour_active = Green;
	else if (ts.scope_centre_grid_colour == SPEC_CYAN)
		ts.scope_centre_grid_colour_active = Cyan;
	else if (ts.scope_centre_grid_colour == SPEC_YELLOW)
		ts.scope_centre_grid_colour_active = Yellow;
	else if (ts.scope_centre_grid_colour == SPEC_BLACK)
		ts.scope_centre_grid_colour_active = Black;
	else if (ts.scope_centre_grid_colour == SPEC_ORANGE)
		ts.scope_centre_grid_colour_active = Orange;
	else if (ts.scope_centre_grid_colour == SPEC_GREY2)
		ts.scope_centre_grid_colour_active = Grey;
	else
		ts.scope_centre_grid_colour_active = White;

	// Clear screen where frequency information will be under graticule
	//
	LCD_PrintText(POS_SPECTRUM_IND_X - 2, POS_SPECTRUM_IND_Y + 60,
			"                                 ", Black, Black, 0);

	//
	strcpy(s, "SPECTRUM SCOPE ");
	slen = 0;	// init string length variable
	//

	//
	slen += strlen(s);				// get width of entire banner string
	slen /= 2;						// scale it for half the width of the string

	// Draw top band
	for (i = 0; i < 16; i++)
		LCD_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,
				(POS_SPECTRUM_IND_Y - 20 + i), POS_SPECTRUM_IND_W,
				COL_SPECTRUM_GRAD);

	if (!(ts.misc_flags1 & 128)) {	// Display Spectrum Scope banner if enabled

		// Top band text - middle caption
		LCD_PrintText((POS_SPECTRUM_IND_X + slen),
				(POS_SPECTRUM_IND_Y - 18), s,
				Grey,
				RGB((COL_SPECTRUM_GRAD*2), (COL_SPECTRUM_GRAD*2),
						(COL_SPECTRUM_GRAD*2)), 0);
	} else {			// Waterfall Mode banner if that is enabled

		// Top band text - middle caption
		LCD_PrintText((POS_SPECTRUM_IND_X + 68),
				(POS_SPECTRUM_IND_Y - 18), "WATERFALL DISPLAY",
				Grey,
				RGB((COL_SPECTRUM_GRAD*2), (COL_SPECTRUM_GRAD*2),
						(COL_SPECTRUM_GRAD*2)), 0);
	}

	// Top band text - grid size
	//LCD_PrintText(			(POS_SPECTRUM_IND_X +  2),
	//								(POS_SPECTRUM_IND_Y - 18),
	//								"Grid 6k",
	//								Grey,
	//								RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)),4);

	// Draw control left and right border
	for (i = 0; i < 2; i++) {
		LCD_DrawStraightLine((POS_SPECTRUM_IND_X - 2 + i),
				(POS_SPECTRUM_IND_Y - 20), (POS_SPECTRUM_IND_H + 12),
				LCD_DIR_VERTICAL,
//									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
				ts.scope_grid_colour_active);

		LCD_DrawStraightLine(
				(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2 + i),
				(POS_SPECTRUM_IND_Y - 20), (POS_SPECTRUM_IND_H + 12),
				LCD_DIR_VERTICAL,
//									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
				ts.scope_grid_colour_active);
	}

	// Frequency bar separator
	LCD_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,
			(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 20), POS_SPECTRUM_IND_W,
			COL_SPECTRUM_GRAD);

	// Draw Frequency bar text
	UiDrawSpectrumScopeFrequencyBarText();

	// Horizontal grid lines
	for (i = 1; i < 4; i++) {
		// Save y position for repaint
		sd.horz_grid_id[i - 1] = (POS_SPECTRUM_IND_Y - 5 + i * 16);

		// Draw
		LCD_DrawStraightLine( POS_SPECTRUM_IND_X, sd.horz_grid_id[i - 1],
		POS_SPECTRUM_IND_W,
		LCD_DIR_HORIZONTAL,
//									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));
				ts.scope_grid_colour_active);
		//printf("vy: %d\n\r",sd.horz_grid_id[i - 1]);
	}

	// Vertical grid lines
	for (i = 1; i < 8; i++) {

		// determine if we are drawing the "center" line on the spectrum  display
		if (!sd.magnify) {
			if ((ts.iq_freq_mode == 2) && (i == 5))// is it frequency translate RF LOW mode?  If so, shift right of center
				clr = ts.scope_centre_grid_colour_active;
			else if((ts.iq_freq_mode == 1) && (i == 3))		// shift left of center if RF HIGH translate mode
				clr = ts.scope_centre_grid_colour_active;
			else if((ts.iq_freq_mode == 3) && (i == 2))		// shift left of center if RF HIGH translate mode
				clr = ts.scope_centre_grid_colour_active;
			else if((ts.iq_freq_mode == 4) && (i == 6))
				clr = ts.scope_centre_grid_colour_active;
			else if ((ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF) && (i == 4))// center if translate mode not active
				clr = ts.scope_centre_grid_colour_active;
			else
				clr = ts.scope_grid_colour_active;// normal color if other lines
		} else if (i == 4)
			clr = ts.scope_centre_grid_colour_active;
		else
			clr = ts.scope_grid_colour_active;	// normal color if other lines

		// Save x position for repaint
		sd.vert_grid_id[i - 1] = (POS_SPECTRUM_IND_X + 32 * i + 1);

		// Draw
		LCD_DrawStraightLine(sd.vert_grid_id[i - 1],
				(POS_SPECTRUM_IND_Y - 4), (POS_SPECTRUM_IND_H - 15),
				LCD_DIR_VERTICAL,
//									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));
				clr);

		//printf("vx: %d\n\r",sd.vert_grid_id[i - 1]);
	}
}

void UiDriverChangeSigProc(uchar enabled) {

	ushort color = Grey;
	char temp[100];

	LCD_DrawEmptyRect( POS_RA_IND_X, POS_RA_IND_Y, 13, 49, Grey);	// draw box

	//
	// Noise blanker settings display
	//
	if (ts.dsp_active & 8) {	// is noise blanker to be displayed
		if (enabled) {
			if (ts.nb_setting >= NB_WARNING3_SETTING)
				color = Red;		// above this value, make it red
			else if (ts.nb_setting >= NB_WARNING2_SETTING)
				color = Orange;		// above this value, make it orange
			else if (ts.nb_setting >= NB_WARNING1_SETTING)
				color = Yellow;		// above this value, make it yellow
			else
				color = White;		// Otherwise, make it white
		}
		//
		if ((!enabled) || (ts.dmod_mode == DEMOD_AM)
				|| (ts.filter_id == AUDIO_WIDE))// is NB disabled, at 10 kHZ and/or are we in AM mode?
			LCD_PrintText((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1), "NB ",
			Grey1, Grey, 0);	// yes - it is gray
		else
			LCD_PrintText((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1), "NB ",
			Black, Grey, 0);
		//
		sprintf(temp, "%02d", ts.nb_setting);
	}
	//
	// DSP settings display
	//
	else {			// DSP settings are to be displayed
		if (enabled && (ts.dsp_active & 1)) {// if this menu is enabled AND the DSP NR is also enabled...
			color = White;		// Make it white by default
			//
			if (ts.dsp_nr_strength >= DSP_STRENGTH_RED)
				color = Red;
			else if (ts.dsp_nr_strength >= DSP_STRENGTH_ORANGE)
				color = Orange;
			else if (ts.dsp_nr_strength >= DSP_STRENGTH_YELLOW)
				color = Yellow;
		}
		//
		if (enabled)
			LCD_PrintText((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1), "DSP",
			Black, Grey, 0);
		else
			LCD_PrintText((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1), "DSP",
			Grey1, Grey, 0);

		sprintf(temp, "%02d", ts.dsp_nr_strength);
	}
	//
	// display numerical value
	//
	LCD_PrintText((POS_RA_IND_X + 30), (POS_RA_IND_Y + 1), temp, color,
	Black, 0);
}

void UiDriverCreateDesktop(void) {

	LCD_drawVU();

	// Create Band value
	UiDriverShowBand(ts.band_mode);

	// Set filters
	UiDriverChangeBandFilter(ts.band_mode,0);

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	//UiDriverShowMode();

	// Create Step Mode
	UiDriverShowStep(df.tuning_step);

	// Frequency
	LCD_PrintText(POS_TUNE_FREQ_X, POS_TUNE_FREQ_Y, "14:000:000", Orange,
			Black, 5);

	// Second Frequency
	LCD_PrintText(POS_TUNE_SFREQ_X, POS_TUNE_SFREQ_Y, "14.000.000", Orange,
			Black, 0);

	// Function buttons
	//UiDriverCreateFunctionButtons(true);

	// S-meter
	//UiDriverCreateSMeter();

	// FIR via keypad, not encoder mode
	//UiDriverChangeFilter(1);

	// Set correct frequency
	UiDriverUpdateFrequency(1);

	// DSP mode
	//UiDriverChangeDSPMode();

	UiDriverChangeSigProc(1);
	UiDriverChangeAfGain(1);
	UiDriverChangeRfGain(1);
	UiDriverChangeCodecGain(1);

//	LCD_DrawStraightLine(POS_PWRN_IND_X, (POS_PWRN_IND_Y - 1), 56,
//	LCD_DIR_HORIZONTAL, Grey);
//	LCD_PrintText(POS_PWRN_IND_X, POS_PWRN_IND_Y, "  VCC  ", Grey, Blue,
//			0);
//	LCD_PrintText(POS_PWR_IND_X, POS_PWR_IND_Y, "12.00V", COL_PWR_IND,
//	Black, 0);


	UiDriverChangeVariable_1(0,0,"OPT");
	UiDriverChangeVariable_2(0,0,"OPT");


}

static void UiDriverCreateSMeter(void) {
	uchar i, v_s;
	char num[20];
	int col;

	// W/H ratio ~ 3.5
	LCD_DrawEmptyRect(POS_SM_IND_X, POS_SM_IND_Y, 72, 202, Grey);

	// Draw top line
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 20), 92,
			LCD_DIR_HORIZONTAL, White);
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 21), 92,
			LCD_DIR_HORIZONTAL, White);
	LCD_DrawStraightLine((POS_SM_IND_X + 113), (POS_SM_IND_Y + 20), 75,
			LCD_DIR_HORIZONTAL, Green);
	LCD_DrawStraightLine((POS_SM_IND_X + 113), (POS_SM_IND_Y + 21), 75,
			LCD_DIR_HORIZONTAL, Green);

	// Leading text
	LCD_PrintText(((POS_SM_IND_X + 18) - 12), (POS_SM_IND_Y + 5), "S",
			White, Black, 4);
	LCD_PrintText(((POS_SM_IND_X + 18) - 12), (POS_SM_IND_Y + 36), "P",
			White, Black, 4);
	LCD_PrintText(((POS_SM_IND_X + 18) - 12), (POS_SM_IND_Y + 59), "SWR",
			White, Black, 4);

	// Trailing text
	LCD_PrintText((POS_SM_IND_X + 185), (POS_SM_IND_Y + 5), "dB", Green,
			Black, 4);
	LCD_PrintText((POS_SM_IND_X + 185), (POS_SM_IND_Y + 36), " W", White,
			Black, 4);

	// Draw s markers on top white line
	for (i = 0; i < 10; i++) {
		num[0] = i + 0x30;
		num[1] = 0;

		// Draw s text, only odd numbers
		if (i % 2) {
			LCD_PrintText(((POS_SM_IND_X + 18) - 4 + i * 10),
					(POS_SM_IND_Y + 5), num, White, Black, 4);
			v_s = 5;
		} else
			v_s = 3;

		// Lines
		LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 10),
				((POS_SM_IND_Y + 20) - v_s), v_s, LCD_DIR_VERTICAL, White);
		LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 10),
				((POS_SM_IND_Y + 20) - v_s), v_s, LCD_DIR_VERTICAL, White);
	}

	// Draw s markers on top red line
	for (i = 0; i < 4; i++) {
		// Prepare text
		num[0] = i * 2 + 0x30;
		num[1] = 0x30;
		num[2] = 0x00;

		if (i) {
			// Draw text
			LCD_PrintText(((POS_SM_IND_X + 113) - 6 + i * 20),
					(POS_SM_IND_Y + 5), num, Green, Black, 4);

			// Draw vert lines
			LCD_DrawStraightLine(((POS_SM_IND_X + 113) + i * 20),
					(POS_SM_IND_Y + 15), 5, LCD_DIR_VERTICAL, Green);
			LCD_DrawStraightLine(((POS_SM_IND_X + 114) + i * 20),
					(POS_SM_IND_Y + 15), 5, LCD_DIR_VERTICAL, Green);
		}
	}

	// Draw middle line
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 32), 170,
			LCD_DIR_HORIZONTAL, White);
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 33), 170,
			LCD_DIR_HORIZONTAL, White);

	// Draw s markers on middle white line
	for (i = 0; i < 12; i++) {
		if (i < 10) {
			num[0] = i + 0x30;
			num[1] = 0;
		} else {
			num[0] = i / 10 + 0x30;
			num[1] = i % 10 + 0x30;
			num[2] = 0;
		}

		// Draw s text, only odd numbers
		if (!(i % 2)) {
			// Text
			LCD_PrintText(((POS_SM_IND_X + 18) - 3 + i * 15),
					(POS_SM_IND_Y + 36), num, White, Black, 4);

			// Lines
			if (i) {
				LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 15),
						((POS_SM_IND_Y + 32) - 2), 2, LCD_DIR_VERTICAL, White);
				LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 15),
						((POS_SM_IND_Y + 32) - 2), 2, LCD_DIR_VERTICAL, White);
			} else {
				LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 15),
						((POS_SM_IND_Y + 32) - 7), 7, LCD_DIR_VERTICAL, White);
				LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 15),
						((POS_SM_IND_Y + 32) - 7), 7, LCD_DIR_VERTICAL, White);
			}
		}
	}

	// Draw bottom line
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 55), 62,
			LCD_DIR_HORIZONTAL, White);
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 56), 62,
			LCD_DIR_HORIZONTAL, White);
	LCD_DrawStraightLine((POS_SM_IND_X + 83), (POS_SM_IND_Y + 55), 105,
			LCD_DIR_HORIZONTAL, Red);
	LCD_DrawStraightLine((POS_SM_IND_X + 83), (POS_SM_IND_Y + 56), 105,
			LCD_DIR_HORIZONTAL, Red);
	col = White;

	// Draw S markers on middle white line
	for (i = 0; i < 12; i++) {
		if (i > 6)
			col = Red;

		if (!(i % 2)) {
			if (i) {
				num[0] = i / 2 + 0x30;
				num[1] = 0;

				// Text
				LCD_PrintText(((POS_SM_IND_X + 18) - 3 + i * 10),
						(POS_SM_IND_Y + 59), num, White, Black, 4);

				LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 10),
						((POS_SM_IND_Y + 55) - 2), 2, LCD_DIR_VERTICAL, col);
				LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 10),
						((POS_SM_IND_Y + 55) - 2), 2, LCD_DIR_VERTICAL, col);
			} else {
				LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 10),
						((POS_SM_IND_Y + 55) - 7), 7, LCD_DIR_VERTICAL, col);
				LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 10),
						((POS_SM_IND_Y + 55) - 7), 7, LCD_DIR_VERTICAL, col);
			}
		}
	}

	// Draw meters
	UiDriverUpdateTopMeterA(0, 0);
	UiDriverUpdateBtmMeter(0, 0);
}

void UiDriverClearSpectrumDisplay(void) {

		LCD_DrawFullRect(POS_SPECTRUM_IND_X - 2,
				(POS_SPECTRUM_IND_Y - 22), 94, 264, Black);	// Clear screen under spectrum scope by drawing a single, black block (faster with SPI!)

}


void UiDriverChangeDSPMode(void) {
	ushort color = White;
	char txt[32];
	ulong x_off = 0;

	// Draw line for upper box
	LCD_DrawStraightLine(POS_DSPU_IND_X, (POS_DSPU_IND_Y - 1), 100,
			LCD_DIR_HORIZONTAL, Grey);
	// Draw line for lower box
	LCD_DrawStraightLine(POS_DSPL_IND_X, (POS_DSPL_IND_Y - 1), 100,
			LCD_DIR_HORIZONTAL, Grey);
	//
	if (((ts.dsp_active & 1) || (ts.dsp_active & 4)))	// DSP active
		color = White;
	else
		// DSP not active
		color = Grey2;
	//
	LCD_PrintText((POS_DSPU_IND_X), (POS_DSPU_IND_Y), "  DSP  ", White,
			Orange, 0);
	//
	if ((ts.dsp_active & 1) && (ts.dsp_active & 4)
			&& (ts.dmod_mode != DEMOD_CW)) {
		sprintf(txt, "NR+NOT");
		x_off = 4;
	} else if (ts.dsp_active & 1) {
		sprintf(txt, "  NR  ");
		x_off = 4;
	} else if (ts.dsp_active & 4) {
		sprintf(txt, " NOTCH");
		if (ts.dmod_mode == DEMOD_CW)
			color = Grey2;
	} else
		sprintf(txt, "  OFF");
	//
	LCD_PrintText((POS_DSPU_IND_X), (POS_DSPL_IND_Y), "       ", White,
			Blue, 1);
	LCD_PrintText((POS_DSPL_IND_X + x_off), (POS_DSPL_IND_Y), txt, color,
			Blue, 1);

}

void UiDriverDrawWhiteSMeter(void) {
	uchar i, v_s;

// Draw top line
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 20), 92,
			LCD_DIR_HORIZONTAL, White);
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 21), 92,
			LCD_DIR_HORIZONTAL, White);

	// Draw s markers on top white line
	for (i = 0; i < 10; i++) {
		// Draw s text, only odd numbers
		if (i % 2) {
			v_s = 5;
		} else
			v_s = 3;

		// Lines
		LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 10),
				((POS_SM_IND_Y + 20) - v_s), v_s, LCD_DIR_VERTICAL, White);
		LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 10),
				((POS_SM_IND_Y + 20) - v_s), v_s, LCD_DIR_VERTICAL, White);
	}
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDrawRedSMeter
//* Object              : draw the part of the S meter in red to indicate A/D overload
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverDeleteSMeter(void) {
	ulong i;

	for (i = POS_SM_IND_Y; i < POS_SM_IND_Y + 72; i += 8) {	// Y coordinate of blanking line
		LCD_PrintText(POS_SM_IND_X + 6, ((POS_SM_IND_Y + 5) + i),
				"                                ", White, Black, 4);
	}
}

void UiDriverDrawRedSMeter(void) {
	uchar i, v_s;

// Draw top line
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 20), 92,
			LCD_DIR_HORIZONTAL, Red);
	LCD_DrawStraightLine((POS_SM_IND_X + 18), (POS_SM_IND_Y + 21), 92,
			LCD_DIR_HORIZONTAL, Red);

	// Draw s markers on top white line
	for (i = 0; i < 10; i++) {

		// Draw s text, only odd numbers
		if (i % 2) {
			v_s = 5;
		} else
			v_s = 3;

		// Lines
		LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 10),
				((POS_SM_IND_Y + 20) - v_s), v_s, LCD_DIR_VERTICAL, Red);
		LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 10),
				((POS_SM_IND_Y + 20) - v_s), v_s, LCD_DIR_VERTICAL, Red);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleSmeter
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverHandleSmeter(void) {
	uchar val;
	float rfg_calc;
	float gcalc;
	static bool clip_indicate = 0;
	static float auto_rfg = 8;
	static uint16_t rfg_timer = 0;	// counter used for timing RFG control decay
	char temp[10];
	//

	sm.skip++;
	if (sm.skip < S_MET_UPD_SKIP)
		return;

	sm.skip = 0;

	// ************************
	// Update S-Meter and control the input gain of the codec to maximize A/D and receiver dynamic range
	// ************************
	//
	// Calculate attenuation of "RF Codec Gain" setting so that S-meter reading can be compensated.
	// for input RF attenuation setting
	//
	if (ts.rf_codec_gain == 9)		// Is RF gain in "AUTO" mode?
		rfg_calc = auto_rfg;
	else {				// not in "AUTO" mode
		rfg_calc = (float) ts.rf_codec_gain;	// get copy of RF gain setting
		auto_rfg = rfg_calc;// keep "auto" variable updated with manual setting when in manual mode
		rfg_timer = 0;
	}
	//
	rfg_calc += 1;	// offset to prevent zero
	rfg_calc *= 2;	// double the range of adjustment
	rfg_calc += 13;	// offset, as bottom of range of A/D gain control is not useful (e.g. ADC saturates before RX hardware)
	if (rfg_calc > 31)	// limit calc to hardware range
		rfg_calc = 31;
	Codec_Line_Gain_Adj((uchar) rfg_calc);	// set the RX gain on the codec
	//
	// Now calculate the RF gain setting
	//
	gcalc = (float) rfg_calc;
	gcalc *= 1.5;	// codec has 1.5 dB/step
	gcalc -= 34.5;	// offset codec setting by 34.5db (full gain = 12dB)
	gcalc = pow10(gcalc / 10);	// convert to power ratio
	ads.codec_gain_calc = sqrt(gcalc);// convert to voltage ratio - we now have current A/D (codec) gain setting
	//
	sm.gain_calc = ads.agc_val;		// get AGC loop gain setting
	sm.gain_calc /= AGC_GAIN_CAL;	// divide by AGC gain calibration factor
	//
	sm.gain_calc = 1 / sm.gain_calc;// invert gain to convert to amount of attenuation
	//
	sm.gain_calc /= ads.codec_gain_calc;	// divide by known A/D gain setting
	//
	sm.s_count = 0;		// Init S-meter search counter
	while ((sm.gain_calc >= S_Meter_Cal[sm.s_count])
			&& (sm.s_count <= S_Meter_Cal_Size)) {// find corresponding signal level
		sm.s_count++;
		;
	}
	val = (uchar) sm.s_count;
	if (!val)	// make sure that the S meter always reads something!
		val = 1;
	//
	//UiDriverUpdateTopMeterA(val, sm.old);
	LCD_AnalogMeter(val, sm.old);
		sm.old = val;

	//
	// Now handle automatic A/D input gain control timing
	//
	rfg_timer++;	// bump RFG timer
	if (rfg_timer > 10000)	// limit count of RFG timer
		rfg_timer = 10000;
	//
	if (ads.adc_half_clip) {	// did clipping almost occur?
		if (rfg_timer >= AUTO_RFG_DECREASE_LOCKOUT) {// has enough time passed since the last gain decrease?
			if (auto_rfg) {	// yes - is this NOT zero?
				auto_rfg -= 0.5;// decrease gain one step, 1.5dB (it is multiplied by 2, above)
				rfg_timer = 0;	// reset the adjustment timer
			}
		}
	} else if (!ads.adc_quarter_clip) {	// no clipping occurred
		if (rfg_timer >= AUTO_RFG_INCREASE_TIMER) {	// has it been long enough since the last increase?
			auto_rfg += 0.5;// increase gain by one step, 1.5dB (it is multiplied by 2, above)
			rfg_timer = 0;// reset the timer to prevent this from executing too often
			if (auto_rfg > 8)	// limit it to 8
				auto_rfg = 8;

		}
	}
	ads.adc_half_clip = 0;// clear "half clip" indicator that tells us that we should decrease gain
	ads.adc_quarter_clip = 0;// clear indicator that, if not triggered, indicates that we can increase gain
	//
	// This makes a portion of the S-meter go red if A/D clipping occurs
	//
	if (ads.adc_clip) {		// did clipping occur?
		if (!clip_indicate) {	// have we seen it clip before?
			//UiDriverDrawRedSMeter();// No, make the first portion of the S-meter red
			clip_indicate = 1;// set flag indicating that we saw clipping and changed the screen (prevent continuous redraw)
		}
		ads.adc_clip = 0;		// reset clip detect flag
	} else {		// clipping NOT occur?
		if (clip_indicate) {// had clipping occurred since we last visited this code?
			//UiDriverDrawWhiteSMeter();// yes - restore the S meter to a white condition
			clip_indicate = 0;// clear the flag that indicated that clipping had occurred
		}
	}
}

 void UiDriverFFTWindowFunction(char mode)
{
    ulong i;
    float32_t gcalc;
    gcalc = 1/ads.codec_gain_calc;				// Get gain setting of codec and convert to multiplier factor
    float32_t s;

    // Information on these windowing functions may be found on the internet - check the Wikipedia article "Window Function"
    // KA7OEI - 20150602

    switch(mode)
    {
    case FFT_WINDOW_RECTANGULAR:	// No processing at all - copy from "Samples" buffer to "Windat" buffer
//			arm_copy_f32((float32_t *)sd.FFT_Windat, (float32_t *)sd.FFT_Samples,FFT_IQ_BUFF_LEN);	// use FFT data as-is
        break;
    case FFT_WINDOW_COSINE:			// Sine window function (a.k.a. "Cosine Window").  Kind of wide...
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = arm_sin_f32((PI * (float32_t)i)/FFT_IQ_BUFF_LEN - 1) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_BARTLETT:		// a.k.a. "Triangular" window - Bartlett (or Fej?r) window is special case where demonimator is "N-1". Somewhat better-behaved than Rectangular
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (1 - fabs(i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_WELCH:			// Parabolic window function, fairly wide, comparable to Bartlett
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (1 - ((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)*((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_HANN:			// Raised Cosine Window (non zero-phase version) - This has the best sidelobe rejection of what is here, but not as narrow as Hamming.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_HAMMING:		// Another Raised Cosine window - This is the narrowest with reasonably good sidelobe rejection.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_BLACKMAN:		// Approx. same "narrowness" as Hamming but not as good sidelobe rejection - probably best for "default" use.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (0.42659 - (0.49656*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.076849*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_NUTTALL:		// Slightly wider than Blackman, comparable sidelobe rejection.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (0.355768 - (0.487396*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.144232*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) - (0.012604*arm_cos_f32((6*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    }

}


void UiDriverInitSpectrumDisplay(void) {
	ulong i;
	arm_status a;

	// Init publics
	sd.state = 0;
	sd.samp_ptr = 0;
	sd.skip_process = 0;
	sd.enabled = 0;
	sd.dial_moved = 0;

	sd.enabled = 1;
	//sd.agc_rate = 5;
	sd.magnify = 0;
	sd.display_offset = -15;

	sd.rescale_rate = (float) ts.scope_rescale_rate;	// load rescale rate
	sd.rescale_rate = 1 / sd.rescale_rate;// actual rate is inverse of this setting

	sd.agc_rate = (float) ts.scope_agc_rate;	// calculate agc rate
	sd.agc_rate = sd.agc_rate / SPECTRUM_AGC_SCALING;

	sd.mag_calc = 1;	// initial setting of spectrum display scaling factor
	//
	sd.wfall_line_update = 0;// init count used for incrementing number of lines for each vertical waterfall screen update
	//
	// load buffer containing waterfall colours
	//
	for (i = 0; i < NUMBER_WATERFALL_COLOURS; i++) {
		sd.waterfall_colours[i] = waterfall_rainbow[i];
	}
	//
	//
	// Load "top" color of palette (the 65th) with that to be used for the center grid color
	//
	sd.waterfall_colours[NUMBER_WATERFALL_COLOURS] =
			(ushort) ts.scope_centre_grid_colour_active;
	//
	/*
	 //
	 // Load waterfall data with "splash" showing palette
	 //
	 uchar  j,k;

	 j = 0;					// init count of lines on display
	 k = sd.wfall_line;		// start with line currently displayed in buffer
	 while(j < RA8875_SPECTRUM_HEIGHT)	{		// loop number of times of buffer
	 for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{		// do this all of the way across, horizonally
	 sd.waterfall[k][i] = (RA8875_SPECTRUM_HEIGHT - j) % RA8875_SPECTRUM_HEIGHT;	// place the color of the palette, indexed to vertical position
	 }
	 j++;		// update line count
	 k++;		// update position within circular buffer - which also is used to calculate color
	 k %= RA8875_SPECTRUM_HEIGHT;	// limit to modulus count of circular buffer size
	 }
	 //

	 */

	//
	//
	if (ts.waterfall_size == WATERFALL_NORMAL) {// waterfall the same size as spectrum scope
		sd.wfall_height = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;
		sd.wfall_ystart = SPECTRUM_START_Y + SPECTRUM_SCOPE_TOP_LIMIT;
		sd.wfall_size = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;
	}				// waterfall larger, covering the word "Waterfall Display"
	else if (ts.waterfall_size == WATERFALL_MEDIUM) {
		sd.wfall_height = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;
		sd.wfall_ystart = SPECTRUM_START_Y - WFALL_MEDIUM_ADDITIONAL;
		sd.wfall_size = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;
	}
	//
	//
	sd.wfall_contrast = (float) ts.waterfall_contrast;// calculate scaling for contrast
	sd.wfall_contrast /= 100;

//	// Init FFT structures
//	a = arm_rfft_init_f32((arm_rfft_instance_f32 *) &sd.S,
//			(arm_cfft_radix4_instance_f32 *) &sd.S_CFFT, FFT_IQ_BUFF_LEN,
//			FFT_QUADRATURE_PROC, 1);
//
//	if (a != ARM_MATH_SUCCESS) {
//		return;
//	}

}

void UiDriverInitFrequency(void) {
	ulong i;

	// Clear band values array
	for (i = 0; i < MAX_BANDS; i++) {
		band_dial_value[i] = 0xFFFFFFFF;	// clear dial values
		band_decod_mode[i] = DEMOD_USB; 	// clear decode mode
		band_filter_mode[i] = AUDIO_DEFAULT_FILTER;	// clear filter mode
	}

	// Lower bands default to LSB mode
	for (i = 0; i < 4; i++)
		band_decod_mode[i] = DEMOD_LSB;

	// Init frequency publics(set diff values so update on LCD will be done)
	df.value_old = 0;
	df.value_new = 0;
	df.tune_old = tune_bands[ts.band];
	df.tune_new = tune_bands[ts.band];
	df.selected_idx = 3; 		// 1 Khz startup step
	df.tuning_step = tune_steps[df.selected_idx];
	df.update_skip = 0;	// skip value to compensate for fast dial rotation - test!!!
	df.temp_factor = 0;
	df.temp_enabled = 0;		// startup state of TCXO

	//df.tx_shift		= 0;		// offcet fo tx
	df.de_detent = 0;

	// Set virtual segments initial value (diff than zero!)
	df.dial_010_mhz = 1;
	df.dial_001_mhz = 4;
	df.dial_100_khz = 0;
	df.dial_010_khz = 0;
	df.dial_001_khz = 0;
	df.dial_100_hz = 0;
	df.dial_010_hz = 0;
	df.dial_001_hz = 0;


}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateBtmMeter
//* Object              : redraw indicator
//* Input Parameters    :
//* Output Parameters   : ToDo: Old value public, so to skip refresh!!!
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverReDrawWaterfallDisplay(void) {

	ulong i, spec_width;
	uint32_t max_ptr;  // throw-away pointer for ARM maxval AND minval functions
	float32_t gcalc;
	//

	gcalc = 1 / ads.codec_gain_calc; // Get gain setting of codec and convert to multiplier factor

	// Process implemented as state machine
	switch (sd.state) {
	//
	// Apply gain to collected IQ samples and then do FFT
	//
	case 1:		// Scale input according to A/D gain and apply Window function
	{

		UiDriverFFTWindowFunction(ts.fft_window_type);// do windowing function on input data to get less "Bin Leakage" on FFT data

		arm_cfft_f32(&arm_cfft_sR_f32_len1024,(float32_t *)(sd.FFT_Samples),0,1);

		sd.state++;

		break;
	}
	case 2:		// Do FFT and calculate complex magnitude
	{
		//
		// Save old display data - we will use later to mask pixel on the control
		//
		arm_copy_q15((q15_t *) sd.FFT_DspData, (q15_t *) sd.FFT_BkpData,
				FFT_IQ_BUFF_LEN / 2);
		//
		// Calculate magnitude
		//
		arm_cmplx_mag_f32((float32_t *) (sd.FFT_Samples),
				(float32_t *) (sd.FFT_MagData), (FFT_IQ_BUFF_LEN / 2));
		//
		sd.state++;

		break;
	}
		//
		//  Low-pass filter amplitude magnitude data
		//
	case 3: {
        uint32_t i;
        float32_t		filt_factor;
        //
        filt_factor = (float)ts.spectrum_filter;		// use stored filter setting
        filt_factor = 1/filt_factor;		// invert filter factor to allow multiplication

		//
		if (sd.dial_moved) {// Clear filter data if dial was moved in steps greater than 1 kHz
			sd.dial_moved = 0;	// Dial moved - reset indicator
			if (df.tuning_step > 1000) {	// was tuning step greater than 1kHz
				arm_copy_f32((float32_t *) sd.FFT_MagData,
						(float32_t *) sd.FFT_AVGData, FFT_IQ_BUFF_LEN / 2);	// copy current data into average buffer
			}
			//
			UiDrawSpectrumScopeFrequencyBarText();// redraw frequency bar on the bottom of the display
			//
		} else {// dial was not moved - do IIR lowpass filtering to "smooth" display
			arm_scale_f32((float32_t *) sd.FFT_AVGData, (float32_t) filt_factor,
					(float32_t *) sd.FFT_Samples, FFT_IQ_BUFF_LEN / 2);	// get scaled version of previous data
			arm_sub_f32((float32_t *) sd.FFT_AVGData,
					(float32_t *) sd.FFT_Samples, (float32_t *) sd.FFT_AVGData,
					FFT_IQ_BUFF_LEN / 2);// subtract scaled information from old, average data
			arm_scale_f32((float32_t *) sd.FFT_MagData, (float32_t) filt_factor,
					(float32_t *) sd.FFT_Samples, FFT_IQ_BUFF_LEN / 2);	// get scaled version of new, input data
			arm_add_f32((float32_t *) sd.FFT_Samples,
					(float32_t *) sd.FFT_AVGData, (float32_t *) sd.FFT_AVGData,
					FFT_IQ_BUFF_LEN / 2);// add portion new, input data into average
			//
			for (i = 0; i < FFT_IQ_BUFF_LEN / 2; i++) {	//		// guarantee that the result will always be >= 0
				if (sd.FFT_AVGData[i] < 1)
					sd.FFT_AVGData[i] = 1;
			}
		}
		sd.state++;


		break;
	}
		//
		// De-linearize and normalize display data and do AGC processing
		//
	case 4: {

		q15_t max1, min1;
		q15_t mean1;
		float32_t sig;
		//
		// De-linearize data with dB/division
		//

		for (i = 0; i < (FFT_IQ_BUFF_LEN / 2); i++) {
			sig = log10(sd.FFT_AVGData[i]) * DB_SCALING_S1;	// take FFT data, do a log10 and multiply it to scale 10dB (fixed)
			sig += sd.display_offset;// apply "AGC", vertical "sliding" offset (or brightness for waterfall)
			if (sig > 1)						// is the value greater than 1?
				sd.FFT_DspData[FFT_IQ_BUFF_LEN/2 - i - 1] = (q15_t)sig;					// it was a useful value - save it
			else
				sd.FFT_DspData[FFT_IQ_BUFF_LEN/2 - i - 1] = 1;							// not greater than 1 - assign it to a base value of 1 for sanity's sake
		}

		//
		arm_copy_q15((q15_t *) sd.FFT_DspData, (q15_t *) sd.FFT_TempData,
				FFT_IQ_BUFF_LEN / 2);
		//
		// Find peak and average to vertically adjust display
		//

		spec_width = FFT_IQ_BUFF_LEN/2;
		arm_max_q15((q15_t *)sd.FFT_TempData, spec_width, &max1, &max_ptr);		// find maximum element
		arm_min_q15((q15_t *)sd.FFT_TempData, spec_width, &min1, &max_ptr);		// find minimum element
		arm_mean_q15((q15_t *)sd.FFT_TempData, spec_width, &mean1);				// find mean value

		//
		// Vertically adjust spectrum scope so that the strongest signals are adjusted to the top
		//
		if(max1 > RA8875_SPECTRUM_HEIGHT*1) {	// zx<x<is result higher than display
			sd.display_offset -= sd.agc_rate;	// yes, adjust downwards quickly
//				if(max1 > RA8875_SPECTRUM_HEIGHT+(RA8875_SPECTRUM_HEIGHT/2))			// is it WAY above top of screen?
//					sd.display_offset -= sd.agc_rate*3;	// yes, adjust downwards REALLY quickly
		}
		//
		// Prevent "empty" spectrum display from filling with "noise" by checking the peak/average of what was found
		//
		else if(((max1*10/mean1) <= (q15_t)ts.spectrum_scope_nosig_adjust) && (max1 < RA8875_SPECTRUM_HEIGHT+(RA8875_SPECTRUM_HEIGHT/2)))	{	// was "average" signal ratio below set threshold and average is not insanely strong??
			if((min1 > 2) && (max1 > 2))	{		// prevent the adjustment from going downwards, "into the weeds"
				sd.display_offset -= sd.agc_rate;	// yes, adjust downwards
	            if(sd.display_offset < (-(RA8875_SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET)))
	               sd.display_offset = (-(RA8875_SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET));
			}
		}
		else
			sd.display_offset += (sd.agc_rate/1);	// no, adjust upwards more slowly
		//
		//
		if((min1 <= 2) && (max1 <= 2))	{	// ARGH - We must already be in the weeds, below the bottom - let's adjust upwards quickly to get it back onto the display!
			sd.display_offset += sd.agc_rate*10;
		}

		//
		// Now, re-arrange the spectrum for the various magnify modes so that they display properly!
		//
			arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/2);


		for (i = 0; i < FFT_IQ_BUFF_LEN / 2; i++) {
			if (sd.FFT_DspData[i] >= RA8875_SPECTRUM_HEIGHT)// is there an illegal height value?
				sd.FFT_DspData[i] = RA8875_SPECTRUM_HEIGHT - 1;	// yes - clip it

		}

		// FIN ESPECTRO

		sd.state++;


		break;
	}
	case 5:
	{

//		LCD_DrawStraightLine(605, 200,150, LCD_DIR_VERTICAL,LCD_COLOR_ra8875_red);
//		LCD_DrawStraightLine(605, 200,150, LCD_DIR_VERTICAL,LCD_COLOR_ra8875_red);
//		LCD_DrawStraightLine(605+1, 200,150, LCD_DIR_VERTICAL,LCD_COLOR_ra8875_red);
//
		LCD_fillRect(604, 200, 3, 150, LCD_COLOR_ra8875_red);

		uint16_t j=0;

		for (i=0; i<FFT_IQ_BUFF_LEN / 4; i++)
		{

			if ( (i % 5) )
			{
				sd.FFT_TempData[j]=sd.FFT_DspData[i];
				j++;
			}
		}

		j+=205;

		for (i=FFT_IQ_BUFF_LEN / 4; i<FFT_IQ_BUFF_LEN / 2; i++)
		{
			if ( (i % 5) )
			{
				sd.FFT_TempData[j]=sd.FFT_DspData[i];
				j++;
			}
		}

		arm_copy_q15((q15_t *)sd.FFT_TempData, (q15_t *)sd.FFT_DspData, FFT_IQ_BUFF_LEN/2);

		UiSpectrumDrawSpectrum(
				(q15_t *) (sd.FFT_BkpData + FFT_IQ_BUFF_LEN / 4 + 112),
				(q15_t *) (sd.FFT_DspData + FFT_IQ_BUFF_LEN / 4 + 112), Black, Blue2,
				0);
		// Right part of the screen (mask and update)
		UiSpectrumDrawSpectrum(
				(q15_t *) (sd.FFT_BkpData),
				(q15_t *) (sd.FFT_DspData), Black, Blue2,
				1);

		sd.state++;

		break;
	}
	case 6:	// rescale waterfall horizontally, apply brightness/contrast, process pallate and put vertical line on screen, if enabled.
	{
		float32_t max, min, mean, offset;

		//
		// Transfer data to the waterfall display circular buffer, putting the bins in frequency-sequential order!
		//
		for (i = 0; i < (FFT_IQ_BUFF_LEN / 2); i++) {
			if (i < (SPECTRUM_WIDTH / 2)) {	// build left half of spectrum data
				sd.FFT_Samples[i] = sd.FFT_DspData[i + FFT_IQ_BUFF_LEN / 4 +112];// get data
			} else {						// build right half of spectrum data
				sd.FFT_Samples[i] = sd.FFT_DspData[i - FFT_IQ_BUFF_LEN / 4 +112]; //+ (FFT_IQ_BUFF_LEN / 4 - SPECTRUM_WIDTH / 2)];// get data
			}
		}

		//
		// Find peak and average to vertically adjust display
		//

		spec_width = FFT_IQ_BUFF_LEN / 2;
		arm_max_f32((float32_t *) sd.FFT_Samples, spec_width, &max,
				&max_ptr);		// find maximum element
		arm_min_f32((float32_t *) sd.FFT_Samples, spec_width, &min,
				&max_ptr);		// find minimum element
		arm_mean_f32((float32_t *) sd.FFT_Samples, spec_width, &mean);// find mean value

		//
		// Calculate "brightness" offset for amplitude value
		//
		offset = (float) ts.waterfall_offset;
		offset -= 25;
		//
		//
		// Vertically adjust spectrum scope so that the strongest signals are adjusted to the top
		//
		if ((max - offset) >= NUMBER_WATERFALL_COLOURS - 1) {// is result higher than display brightness
			sd.display_offset -= sd.agc_rate;	// yes, adjust downwards quickly
		}
		//
		// Prevent "empty" spectrum display from filling with "noise" by checking the peak/average of what was found
		//
		else if (((max * 10 / mean) <= (q15_t) ts.waterfall_nosig_adjust)
				&& (max < RA8875_SPECTRUM_HEIGHT + (RA8875_SPECTRUM_HEIGHT / 2))) {// was "average" signal ratio below set threshold and average is not insanely strong??

			if ((min > 2) && (max > 2)) {// prevent the adjustment from going downwards, "into the weeds"
				sd.display_offset -= sd.agc_rate;	// yes, adjust downwards
				if (sd.display_offset
						< (-(RA8875_SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET)))
					sd.display_offset = (-(RA8875_SPECTRUM_HEIGHT
							+ SPECTRUM_SCOPE_ADJUST_OFFSET));
			}
		} else
			sd.display_offset += (sd.agc_rate / 3);	// no, adjust upwards more slowly
		//
		//
		if ((min <= 2) && (max <= 2)) {	// ARGH - We must already be in the weeds, below the bottom - let's adjust upwards quickly to get it back onto the display!
			sd.display_offset += sd.agc_rate * 10;
		}

		sd.state++;
		break;
	}

	case 7:
	{
		//
		sd.wfall_line %= sd.wfall_size;	// make sure that the circular buffer is clipped to the size of the display area
		//
		//
		// Contrast:  100 = 1.00 multiply factor:  125 = multiply by 1.25 - "sd.wfall_contrast" already converted to 100=1.00
		//
		arm_scale_f32((float32_t *) sd.FFT_Samples,
				(float32_t) sd.wfall_contrast, (float32_t *) sd.FFT_Samples,
				FFT_IQ_BUFF_LEN / 2);
		//
		//
		// After the above manipulation, clip the result to make sure that it is within the range of the palette table
		//
        for(i = 0; i < SPECTRUM_WIDTH; i++)
        {
            if(sd.FFT_Samples[i] >= NUMBER_WATERFALL_COLOURS)	// is there an illegal color value?
            {
                sd.FFT_Samples[i] = NUMBER_WATERFALL_COLOURS - 1;	// yes - clip it
            }

            sd.waterfall[sd.wfall_size-sd.wfall_line-1][i] = (ushort)sd.FFT_Samples[i];	// save the manipulated value in the circular waterfall buffer
        }


		//
		// Place center line marker on screen:  Location [64] (the 65th) of the palette is reserved is a special color reserved for this
		//

		sd.waterfall[sd.wfall_line][605] = NUMBER_WATERFALL_COLOURS;


		//
		sd.wfall_line++;// bump to the next line in the circular buffer for next go-around
		//
		// scan_top is used to limit AGC action to "magnified" portion
		//
		sd.state++;
		break;
	}

	case 8:
	{
		uchar lptr = sd.wfall_size-sd.wfall_line;		// get current line of "bottom" of waterfall in circular buffer


		//
		sd.wfall_line_update++;									// update waterfall line count
		sd.wfall_line_update %= 1; //ts.waterfall_vert_step_size;	// clip it to number of lines per iteration

        if(!sd.wfall_line_update)	 							// if it's count is zero, it's time to move the waterfall up
        {
            //
            lptr %= sd.wfall_size;		// do modulus limit of spectrum high
            //
            // set up LCD for bulk write, limited only to area of screen with waterfall display.  This allow data to start from the
            // bottom-left corner and advance to the right and up to the next line automatically without ever needing to address
            // the location of any of the display data - as long as we "blindly" write precisely the correct number of pixels per
            // line and the number of lines.
            //
            LCD_OpenBulkWrite(SPECTRUM_START_X, SPECTRUM_WIDTH, 350, sd.wfall_height);
            //
            uint16_t spectrumLine[2][SPECTRUM_WIDTH];
            uchar lcnt = 0;                 // initialize count of number of lines of display

            for(lcnt = 0;lcnt < sd.wfall_size; lcnt++)	 				// set up counter for number of lines defining height of waterfall
            {
                for(i = 0; i < (SPECTRUM_WIDTH); i++)	 	// scan to copy one line of spectral data - "unroll" to optimize for ARM processor
                {
                    spectrumLine[lcnt%2][i] = sd.waterfall_colours[sd.waterfall[lptr][i]];	// write to memory using waterfall color from palette
                }

                LCD_BulkWrite(spectrumLine[lcnt%2],SPECTRUM_WIDTH);

                lptr++;									// point to next line in circular display buffer
                lptr %= sd.wfall_size;				// clip to display height
            }
            //

            LCD_CloseBulkWrite();					// we are done updating the display - return to normal full-screen mode
        }
			sd.state = 0;	// Stage 0 - collection of data by the Audio driver

			break;
	}

	default:
		sd.state = 0;
		break;
	}
}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDisplayFilterBW
//* Object              : Display/Update line under the Waterfall or Spectrum that graphically indicates filter bandwidth and relative position
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriverDisplayFilterBW(void)
{
	float	width, offset, calc;
	ushort	lpos;
	bool	is_usb;
	ushort clr;

	// Update screen indicator - first get the width and center-frequency offset of the currently-selected filter
	//
	// Update screen indicator - first get the width and center-frequency offset of the currently-selected filter
	//
	switch(ts.filter_id)	{
		case AUDIO_300HZ:	// 300 Hz CW filter
			switch(ts.filter_300Hz_select)	{
				case 1:
					offset = FILT300_1;
					break;
				case 2:
					offset = FILT300_2;
					break;
				case 3:
					offset = FILT300_3;
					break;
				case 4:
					offset = FILT300_4;
					break;
				case 5:
					offset = FILT300_5;
					break;
				case 6:
					offset = FILT300_6;
					break;
				case 7:
					offset = FILT300_7;
					break;
				case 8:
					offset = FILT300_8;
					break;
				case 9:
					offset = FILT300_9;
					break;
				default:
					offset = FILT300_6;
					break;
			}
			width = FILTER_300HZ_WIDTH;
			//
			break;
		case AUDIO_500HZ:	// 500 Hz CW filter
			switch(ts.filter_500Hz_select)	{
				case 1:
					offset = FILT500_1;
					break;
				case 2:
					offset = FILT500_2;
					break;
				case 3:
					offset = FILT500_3;
					break;
				case 4:
					offset = FILT500_4;
					break;
				case 5:
					offset = FILT500_5;
					break;
				default:
					offset = FILT500_3;
					break;
			}
			width = FILTER_500HZ_WIDTH;
			//
			break;
		case AUDIO_1P8KHZ:		// 1.8 kHz wide filter
			switch(ts.filter_1k8_select)	{
				case 1:
					offset = FILT1800_1;
					break;
				case 2:
					offset = FILT1800_2;
					break;
				case 3:
					offset = FILT1800_3;
					break;
				case 4:
					offset = FILT1800_4;
					break;
				case 5:
					offset = FILT1800_5;
					break;
				case 6:
					offset = FILT1800_6;
					break;
				default:
					offset = FILT1800_3;
					break;
			}
			width = FILTER_1800HZ_WIDTH;
			//
			break;
		case AUDIO_2P3KHZ:		// 2.3 kHz wide filter
			switch(ts.filter_2k3_select)	{
				case 1:
					offset = FILT2300_1;
					break;
				case 2:
					offset = FILT2300_2;
					break;
				case 3:
					offset = FILT2300_3;
					break;
				case 4:
					offset = FILT2300_4;
					break;
				case 5:
					offset = FILT2300_5;
					break;
				default:
					offset = FILT2300_2;
					break;
			}
			width = FILTER_2300HZ_WIDTH;
			//
			break;
		case AUDIO_2P7KHZ:		// 2.7 kHz wide filter
			switch(ts.filter_2k7_select)	{
				case 1:
					offset = FILT2700_1;
					break;
				case 2:
					offset = FILT2700_2;
					break;
				default:
					offset = FILT2700_2;
					break;
			}
			width = FILTER_2700HZ_WIDTH;
			//
			break;
		case AUDIO_2P9KHZ:		// 2.9 kHz wide filter
			switch(ts.filter_2k9_select)	{
				case 1:
					offset = FILT2900_1;
					break;
				case 2:
					offset = FILT2900_2;
					break;
				default:
					offset = FILT2900_2;
					break;
			}
			width = FILTER_2900HZ_WIDTH;
			//
			break;
		case AUDIO_3P6KHZ:		// 3.6 kHz wide filter
			switch(ts.filter_3k6_select)	{
				case 1:
					offset = FILT3600_1;
					break;
				case 2:
					offset = FILT3600_2;
					break;
				default:
					offset = FILT3600_2;
					break;
			}
			width = FILTER_3600HZ_WIDTH;
			//
			break;
		case AUDIO_WIDE:	// selectable "wide" bandwidth filter
			switch(ts.filter_wide_select)	{
				case WIDE_FILTER_5K:
				case WIDE_FILTER_5K_AM:
					offset = FILT5000;
					width = FILTER_5000HZ_WIDTH;
					break;
				case WIDE_FILTER_6K:
				case WIDE_FILTER_6K_AM:
					offset = FILT6000;
					width = FILTER_6000HZ_WIDTH;
					break;
				case WIDE_FILTER_7K5:
				case WIDE_FILTER_7K5_AM:
					offset = FILT7500;
					width = FILTER_7500HZ_WIDTH;
					break;
				case WIDE_FILTER_10K:
				case WIDE_FILTER_10K_AM:
				default:
					offset = FILT10000;
					width = FILTER_10000HZ_WIDTH;
					break;
			}
			break;
		default:
			// in case of call with not yet covered parameters we set the widest filter as default
			offset = FILT10000;
			width = FILTER_10000HZ_WIDTH;
			break;
	}


	//
	//
	switch(ts.dmod_mode)	{	// determine if the receiver is set to LSB or USB or FM
		case DEMOD_LSB:
			is_usb = 0;		// it is LSB
			break;
		case DEMOD_CW:
			if(!ts.cw_lsb)	// is this USB RX mode?  (LSB of mode byte was zero)
				is_usb = 1;	// it is USB
			else	// No, it is LSB RX mode
				is_usb = 0;	// it is LSB
			break;
		case DEMOD_USB:
		case DEMOD_DIGI:
		default:
			is_usb = 1;		// it is USB
			break;
	}
	//
	calc = 48000/FILT_DISPLAY_WIDTH;		// magnify mode not on - calculate number of Hz/pixel
		if(ts.iq_freq_mode == 1)			// line is to left if in "RX LO HIGH" mode
			lpos = 98;
		else if(ts.iq_freq_mode == 2)			// line is to right if in "RX LO LOW" mode
			lpos = 162;
		else if(ts.iq_freq_mode == 3)			// line is to left if in "RX LO LOW" mode
			lpos = 66;
		else if(ts.iq_freq_mode == 4)			// line is to right if in "RX LO LOW" mode
			lpos = 194;
		else					// frequency translate mode is off
			lpos = 130;			// line is in center

	//
	offset /= calc;							// calculate filter center frequency offset in pixels
	width /= calc;							// calculate width of line in pixels
	//
	//
	if((ts.dmod_mode == DEMOD_AM))	{	// special cases - AM and FM, which are double-sidebanded
		lpos -= width;					// line starts "width" below center
		width *= 2;						// the width is double in AM, above and below center
	}
	else if(!is_usb)	// not AM, but LSB:  calculate position of line, compensating for both width and the fact that SSB/CW filters are not centered
		lpos -= ((offset - (width/2)) + width);	// if LSB it will be below zero Hz
	else				// USB mode
		lpos += (offset - (width/2));			// if USB it will be above zero Hz

	//
	//	erase old line
	//
	LCD_DrawStraightLine((POS_SPECTRUM_IND_X+SPECTRUM_WIDTH/2), (200), SPECTRUM_WIDTH/2-1, LCD_DIR_HORIZONTAL, Black);
	LCD_DrawStraightLine((POS_SPECTRUM_IND_X+SPECTRUM_WIDTH/2), (200 + 1), SPECTRUM_WIDTH/2-1, LCD_DIR_HORIZONTAL, Black);
	//
	//
	// get color for line
	//
	if(ts.filter_disp_colour == SPEC_GREY)
		clr = Grey;
	else if(ts.filter_disp_colour == SPEC_BLUE)
		clr = Blue;
	else if(ts.filter_disp_colour == SPEC_RED)
		clr = Red;
	else if(ts.filter_disp_colour == SPEC_MAGENTA)
		clr = Magenta;
	else if(ts.filter_disp_colour == SPEC_GREEN)
		clr = Green;
	else if(ts.filter_disp_colour == SPEC_CYAN)
		clr = Cyan;
	else if(ts.filter_disp_colour == SPEC_YELLOW)
		clr = Yellow;
	else if(ts.filter_disp_colour == SPEC_BLACK)
		clr = Black;
	else if(ts.filter_disp_colour == SPEC_ORANGE)
		clr = Orange;
	else
		clr = White;
	//
	// draw line
	//
	LCD_DrawStraightLine((POS_SPECTRUM_IND_X+SPECTRUM_WIDTH/2 + lpos +10), (200), (ushort)width, LCD_DIR_HORIZONTAL, clr);
	LCD_DrawStraightLine((POS_SPECTRUM_IND_X+SPECTRUM_WIDTH/2 + lpos +10), (200 + 1), (ushort)width, LCD_DIR_HORIZONTAL, clr);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateBtmMeter
//* Object              : redraw indicator
//* Input Parameters    : val=indicated value, warn=red warning threshold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateBtmMeter(uchar val, uchar warn) {
	uchar i, v_s;
	int col = Cyan;

	// Do not waste time redrawing if outside of the range
	if (val > 34)
		return;

	// Indicator height
	v_s = 3;

	// Draw indicator
	for (i = 1; i < 34; i++) {
		if (val < i)
			col = Grid;
		else if ((i >= warn) && warn)// is level above "warning" color? (is "warn" is zero, disable warning)
			col = Red2;			// yes - display values above that color in red

		// Lines
		LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 5),
				((POS_SM_IND_Y + 51) - v_s), v_s, LCD_DIR_VERTICAL, col);
		LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 5),
				((POS_SM_IND_Y + 51) - v_s), v_s, LCD_DIR_VERTICAL, col);
		LCD_DrawStraightLine(((POS_SM_IND_X + 20) + i * 5),
				((POS_SM_IND_Y + 51) - v_s), v_s, LCD_DIR_VERTICAL, col);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateTopMeterA
//* Object              : redraw indicator, same like upper implementation
//* Input Parameters    : but no hold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateTopMeterA(uchar val, uchar old) {
	uchar i, v_s;
	int col = Blue2;

	// Do not waste time redrawing if outside of the range or if the meter has not changed
	if ((val > 34) || (val == old))
		return;

	// Indicator height
	v_s = 3;

	// Draw first indicator
	for (i = 1; i < 34; i++) {
		if (val < i)
			col = Grid;

		// Lines
		LCD_DrawStraightLine(((POS_SM_IND_X + 18) + i * 5),
				((POS_SM_IND_Y + 28) - v_s), v_s, LCD_DIR_VERTICAL, col);
		LCD_DrawStraightLine(((POS_SM_IND_X + 19) + i * 5),
				((POS_SM_IND_Y + 28) - v_s), v_s, LCD_DIR_VERTICAL, col);
		LCD_DrawStraightLine(((POS_SM_IND_X + 20) + i * 5),
				((POS_SM_IND_Y + 28) - v_s), v_s, LCD_DIR_VERTICAL, col);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateLcdFreq
//* Object              : this function will split LCD freq display control
//* Object              : and update as it is 7 segments indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverUpdateLcdFreq(ulong dial_freq, ushort color) {
	uchar d_10mhz, d_1mhz;
	uchar d_100khz, d_10khz, d_1khz;
	uchar d_100hz, d_10hz, d_1hz;

#define LARGE_FONT_WIDTH 32
#define OFFSET 48

	char digit[2];

	// Terminate
	digit[1] = 0;

	// -----------------------
	// See if 10 Mhz needs update
	d_10mhz = (dial_freq / 10000000);
	if (d_10mhz != df.dial_010_mhz) {
		//printf("10 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = OFFSET + (d_10mhz & 0x0F);

		// Update segment
		if (d_10mhz)
			LCD_PrintText((POS_TUNE_FREQ_X + 0), POS_TUNE_FREQ_Y, digit,
					Orange, Black, 5);
		else
			LCD_PrintText((POS_TUNE_FREQ_X + 0), POS_TUNE_FREQ_Y, digit,
					Black, Black, 5);	// mask the zero

		// Save value
		df.dial_010_mhz = d_10mhz;
	}

	// -----------------------
	// See if 1 Mhz needs update
	d_1mhz = (dial_freq % 10000000) / 1000000;
	if (d_1mhz != df.dial_001_mhz) {
		//printf("1 mhz diff: %d\n\r",d_1mhz);

		// To string
		digit[0] = OFFSET + (d_1mhz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH), POS_TUNE_FREQ_Y, digit,
				color, Black, 5);

		// Save value
		df.dial_001_mhz = d_1mhz;
	}

	// -----------------------
	// See if 100 khz needs update
	d_100khz = (dial_freq % 1000000) / 100000;
	if (d_100khz != df.dial_100_khz) {
		//printf("100 khz diff: %d\n\r",d_100khz);

		// To string
		digit[0] = OFFSET + (d_100khz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH * 3),
				POS_TUNE_FREQ_Y, digit, color, Black, 5);

		// Save value
		df.dial_100_khz = d_100khz;
	}

	// -----------------------
	// See if 10 khz needs update
	d_10khz = (dial_freq % 100000) / 10000;
	if (d_10khz != df.dial_010_khz) {
		//printf("10 khz diff: %d\n\r",d_10khz);

		// To string
		digit[0] = OFFSET + (d_10khz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH * 4),
				POS_TUNE_FREQ_Y, digit, color, Black, 5);

		// Save value
		df.dial_010_khz = d_10khz;
	}

	// -----------------------
	// See if 1 khz needs update
	d_1khz = (dial_freq % 10000) / 1000;
	if (d_1khz != df.dial_001_khz) {
		//printf("1 khz diff: %d\n\r",d_1khz);

		// To string
		digit[0] = OFFSET + (d_1khz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH * 5),
				POS_TUNE_FREQ_Y, digit, color, Black, 5);

		// Save value
		df.dial_001_khz = d_1khz;
	}

	// -----------------------
	// See if 100 hz needs update
	d_100hz = (dial_freq % 1000) / 100;
	if (d_100hz != df.dial_100_hz) {
		//printf("100 hz diff: %d\n\r",d_100hz);

		// To string
		digit[0] = OFFSET + (d_100hz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH * 7),
				POS_TUNE_FREQ_Y, digit, color, Black, 5);

		// Save value
		df.dial_100_hz = d_100hz;
	}

	// -----------------------
	// See if 10 hz needs update
	d_10hz = (dial_freq % 100) / 10;
	if (d_10hz != df.dial_010_hz) {
		//printf("10 hz diff: %d\n\r",d_10hz);

		// To string
		digit[0] = OFFSET + (d_10hz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH * 8),
				POS_TUNE_FREQ_Y, digit, color, Black, 5);

		// Save value
		df.dial_010_hz = d_10hz;
	}

	// -----------------------
	// See if 1 hz needs update
	d_1hz = (dial_freq % 10) / 1;
	if (d_1hz != df.dial_001_hz) {
		//printf("1 hz diff: %d\n\r",d_1hz);

		// To string
		digit[0] = OFFSET + (d_1hz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH * 9),
				POS_TUNE_FREQ_Y, digit, color, Black, 5);

		// Save value
		df.dial_001_hz = d_1hz;

	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowStep
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverUpdateTopMeterA(uchar val, uchar old);

void UiDriverShowStep(ulong step) {

	ulong line_loc;
	static bool step_line = 0;	// used to indicate the presence of a step line
	ulong color;

	if (ts.tune_step)	// is this a "Temporary" step size from press-and-hold?
		color = Cyan;	// yes - display step size in Cyan
	else
		// normal mode
		color = White;	// step size in white

	if (step_line) {// Remove underline indicating step size if one had been drawn
		LCD_DrawStraightLine((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),
				(POS_TUNE_FREQ_Y + 55), (LARGE_FONT_WIDTH * 7),
				LCD_DIR_HORIZONTAL, Black);
		LCD_DrawStraightLine((POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * 3)),
				(POS_TUNE_FREQ_Y + 56), (LARGE_FONT_WIDTH * 7),
				LCD_DIR_HORIZONTAL, Black);
	}

	// Blank old step size
//	LCD_DrawFullRect(POS_TUNE_STEP_MASK_X, POS_TUNE_STEP_MASK_Y,
//	POS_TUNE_STEP_MASK_H, POS_TUNE_STEP_MASK_W, Black);


	// Create Step Mode
	switch (df.tuning_step) {
	case T_STEP_1HZ:
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 1Hz  ", color, Blue, 1);
		line_loc = 9;
		break;
	case T_STEP_10HZ:
		line_loc = 8;
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 10Hz  ", color, Blue, 1);
		break;
	case T_STEP_100HZ:
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 100Hz ", color, Blue, 1);
		line_loc = 7;
		break;
	case T_STEP_1KHZ:
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 1kHz  ", color, Blue, 1);
		line_loc = 5;
		break;
	case T_STEP_5KHZ:
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 5kHz  ", color, Blue ,1);
		line_loc = 5;
		break;
	case T_STEP_10KHZ:
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 10kHz ", color, Blue, 1);
		line_loc = 4;
		break;
	case T_STEP_100KHZ:
		LCD_PrintText((POS_TUNE_STEP_X + 50),
		POS_TUNE_STEP_Y, " 100kHz", color, Blue, 1);
		line_loc = 3;
		break;
	default:
		break;
	}
	//

	LCD_DrawStraightLine(
			(POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),
			(POS_TUNE_FREQ_Y + 55), (LARGE_FONT_WIDTH), LCD_DIR_HORIZONTAL,
			White);
	LCD_DrawStraightLine(
			(POS_TUNE_FREQ_X + (LARGE_FONT_WIDTH * line_loc)),
			(POS_TUNE_FREQ_Y + 56), (LARGE_FONT_WIDTH), LCD_DIR_HORIZONTAL,
			White);
	step_line = 1;	// indicate that a line under the step size had been drawn

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowBand
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverShowBand(uchar band) {
	// Clear control
	//LCD_DrawFullRect(POS_BAND_MODE_MASK_X,POS_BAND_MODE_MASK_Y,POS_BAND_MODE_MASK_H,POS_BAND_MODE_MASK_W,Black,0);

	// Create Band value
	switch (band) {
	case BAND_MODE_80:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "80m", Orange,
				Black, 1);
		break;

	case BAND_MODE_60:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "60m", Orange,
				Black, 1);
		break;

	case BAND_MODE_40:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "40m", Orange,
				Black, 1);
		break;

	case BAND_MODE_30:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "30m", Orange,
				Black, 1);
		break;

	case BAND_MODE_20:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "20m", Orange,
				Black, 1);
		break;

	case BAND_MODE_17:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "17m", Orange,
				Black, 1);
		break;

	case BAND_MODE_15:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "15m", Orange,
				Black, 1);
		break;

	case BAND_MODE_12:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "12m", Orange,
				Black, 1);
		break;

	case BAND_MODE_10:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "10m", Orange,
				Black, 1);
		break;

	case BAND_MODE_GEN:
		LCD_PrintText(POS_BAND_MODE_X, POS_BAND_MODE_Y, "Gen", Orange,
				Black, 1);
		break;

	default:
		break;
	}
}

void UiDriverShowMode(void) {
	// Clear control
//	LCD_DrawFullRect(POS_DEMOD_MODE_MASK_X, POS_DEMOD_MODE_MASK_Y,
//			POS_DEMOD_MODE_MASK_H, POS_DEMOD_MODE_MASK_W, Blue);

	LCD_PrintText((POS_DEMOD_MODE_X + 8), POS_DEMOD_MODE_Y, "   ",
					Cream, Blue, 1);

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	switch (ts.dmod_mode) {
	case DEMOD_USB:
		LCD_PrintText((POS_DEMOD_MODE_X + 8), POS_DEMOD_MODE_Y, "USB",
				Cream, Blue, 1);
		break;
	case DEMOD_LSB:
		LCD_PrintText((POS_DEMOD_MODE_X + 8), POS_DEMOD_MODE_Y, "LSB",
				Cream, Blue, 1);
		break;
	case DEMOD_AM:
		LCD_PrintText((POS_DEMOD_MODE_X + 12), POS_DEMOD_MODE_Y, "AM",
				Cream, Blue, 1);
		break;
	case DEMOD_CW:
		LCD_PrintText((POS_DEMOD_MODE_X + 12), POS_DEMOD_MODE_Y, "CW",
				Cream, Blue, 1);
		break;
	default:
		break;
	}
}

void UiDriverShowFilter(uchar filter_id) {

	LCD_PrintText(0, 180, "     ", White, Black, 0);

	switch (filter_id) {
	case 1:
		LCD_PrintText(0, 180, "1p8k", Orange, Black, 0);
		break;

	case 2:
		LCD_PrintText(0, 180, "2p3k", Orange, Black, 0);
		break;

	case 3:
		LCD_PrintText(0, 180, "3p6k", Orange, Black, 0);
		break;

	case 4:
		LCD_PrintText(0, 180, "10k", Orange, Black, 0);
		break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDrawSpectrumScopeFrequencyBarText(void)
{
	ulong	freq_calc;
	ulong	i, clr;
	char	txt[16], *c;
	ulong	grat;

#define BAR_POS 180

	if(ts.scope_scale_colour == SPEC_BLACK)		// don't bother updating frequency scale if it is black (invisible)!
		return;

	grat = 6;	// Default - Magnify mode OFF, graticules spaced 6 kHz
	//
	//
	// This function draws the frequency bar at the bottom of the spectrum scope, putting markers every at every graticule and the full frequency
	// (rounded to the nearest kHz) in the "center".  (by KA7OEI, 20140913)
	//
	// get color for frequency scale
	//
	if(ts.scope_scale_colour == SPEC_GREY)
		clr = Grey;
	else if(ts.scope_scale_colour == SPEC_BLUE)
		clr = Blue;
	else if(ts.scope_scale_colour == SPEC_RED)
		clr = Red;
	else if(ts.scope_scale_colour == SPEC_MAGENTA)
		clr = Magenta;
	else if(ts.scope_scale_colour == SPEC_GREEN)
		clr = Green;
	else if(ts.scope_scale_colour == SPEC_CYAN)
		clr = Cyan;
	else if(ts.scope_scale_colour == SPEC_YELLOW)
		clr = Yellow;
	else if(ts.scope_scale_colour == SPEC_BLACK)
		clr = Black;
	else if(ts.scope_scale_colour == SPEC_ORANGE)
		clr = Orange;
	else
		clr = White;

	freq_calc = df.tune_new/4;		// get current frequency in Hz

	if(!sd.magnify)	{		// if magnify is off, way *may* have the graticule shifted.  (If it is on, it is NEVER shifted from center.)
		if(ts.iq_freq_mode == 1)			// Is "RX LO HIGH" translate mode active?
			freq_calc += FREQ_SHIFT_MAG;	// Yes, shift receive frequency to left of center
		else if(ts.iq_freq_mode == 2)		// it is "RX LO LOW" in translate mode
			freq_calc -= FREQ_SHIFT_MAG;	// shift receive frequency to right of center
		else if(ts.iq_freq_mode == 3)		// it is "RX LO LOW" in translate mode
			freq_calc += FREQ_SHIFT_MAG*2;	// shift receive frequency to right of center
		else if(ts.iq_freq_mode == 4)		// it is "RX LO LOW" in translate mode
			freq_calc -= FREQ_SHIFT_MAG*2;	// shift receive frequency to right of center
	}
	freq_calc = (freq_calc + 500)/1000;	// round graticule frequency to the nearest kHz


	// remainder of frequency/graticule markings
	sprintf(txt, "%u ", (unsigned)freq_calc-(4*(unsigned)grat));	// -36 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X +  20),BAR_POS,txt,clr,Black,4);
	//
	sprintf(txt, "%u ", (unsigned)freq_calc-(3*(unsigned)grat));	// -30 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 100-20),BAR_POS,txt,clr,Black,4);
	//
	sprintf(txt, " %u ", (unsigned)freq_calc-(2*(unsigned)grat));	// -24 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 200-20),BAR_POS,txt,clr,Black,4);
	//
	sprintf(txt, " %u ", (unsigned)freq_calc-(1*(unsigned)grat));	// -18 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 300-20),BAR_POS,txt,clr,Black,4);
	//

	sprintf(txt, " %u ", (unsigned)freq_calc+(0*(unsigned)grat));	// -12 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 400-20),BAR_POS,txt,clr,Black,4);
	//
	sprintf(txt, " %u ", (unsigned)freq_calc+(1*(unsigned)grat));	// -6 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 500-20),BAR_POS,txt,clr,Black,4);
	//
	sprintf(txt, " %u ", (unsigned)freq_calc+(2*(unsigned)grat));	// 0 khz
	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 600-20),BAR_POS,txt,clr,Black,4);
	//
	sprintf(txt, " %u", (unsigned)freq_calc+(3*(unsigned)grat));	// 6 khz
	c = &txt[strlen(txt)-2];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 700-20),BAR_POS,txt,clr,Black,4);

	sprintf(txt, " %u", (unsigned)freq_calc+(4*(unsigned)grat));	// 12khz khz
	c = &txt[strlen(txt)-2];  // point at 2nd character from the end
	LCD_PrintText((POS_SPECTRUM_IND_X + 750),BAR_POS,txt,clr,Black,4);

}

void UiInitSpectrumScopeWaterfall(void) {
	UiDriverClearSpectrumDisplay();		// clear display under spectrum scope
	UiDriverCreateSpectrumScope();
	UiDriverInitSpectrumDisplay();
	UiDriverDisplayFilterBW();// Update on-screen indicator of filter bandwidth
}

void UiDriverUpdateFrequencyFast(void) {
	ulong loc_tune_new, dial_freq;
	ushort col = Orange;

	// Get value, while blocking update
	loc_tune_new = df.tune_new;

	// Calculate display frequency
	dial_freq = ((loc_tune_new / 4) + df.transv_freq);

	// Clear not used segments on display frequency

	// Calculate actual tune frequency
	ts.tune_freq = (dial_freq - df.transv_freq) * 4;

	//
	// detect - and eliminate - unnecessary synthesizer frequency changes
	//
	if (ts.tune_freq == ts.tune_freq_old)	// did the frequency NOT change?
		return;		// yes - bail out - no need to do anything!

	ts.tune_freq_old = ts.tune_freq;// frequency change is required - save change detector

	// Set frequency
	if (ui_si570_set_frequency(ts.tune_freq, ts.freq_cal, df.temp_factor, 0)) {
		// Color in red
		col = Red;
	}

	// Update LCD
	UiDriverUpdateLcdFreq(dial_freq, col);

	// Allow clear of spectrum display in its state machine
	sd.dial_moved = 1;

	// Save current freq
	df.tune_old = loc_tune_new;

}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateSecondLcdFreq
//* Object              : second freq indicator
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverUpdateSecondLcdFreq(ulong dial_freq)
{
	uchar		d_100mhz,d_10mhz,d_1mhz;
	uchar		d_100khz,d_10khz,d_1khz;
	uchar		d_100hz,d_10hz,d_1hz;
	static bool	digit_100 = 1, digit_10 = 1;	// set active first time through to make sure that digit is erased, if it exists

	char		digit[2];


	//
	// Terminate
	digit[1] = 0;

	//printf("--------------------\n\r");
	//printf("dial: %dHz\n\r",dial_freq);
	//printf("dial_001_mhz: %d\n\r",df.dial_001_mhz);
	//printf("dial_100_khz: %d\n\r",df.dial_100_khz);
	//printf("dial_010_khz: %d\n\r",df.dial_010_khz);
	//printf("dial_001_khz: %d\n\r",df.dial_001_khz);
	//printf("dial_100_hz:  %d\n\r",df.dial_100_hz);
	//printf("dial_010_hz:  %d\n\r",df.dial_010_hz);
	//printf("dial_001_hz:  %d\n\r",df.dial_001_hz);

	// Second Frequency
	//LCD_PrintText((POS_TUNE_FREQ_X + 175),(POS_TUNE_FREQ_Y + 8),"14.000.000",Grey,Black,0);

	// -----------------------
	// See if 100 Mhz needs update
	d_100mhz = (dial_freq/100000000);
	if(d_100mhz != df.sdial_100_mhz)
	{
		//printf("100 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_100mhz & 0x0F);

		// Update segment
		if(d_100mhz)
			LCD_PrintText((POS_TUNE_SFREQ_X - SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
		else
			LCD_PrintText((POS_TUNE_SFREQ_X - SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// mask the zero

		// Save value
		df.sdial_100_mhz = d_100mhz;
		digit_100 = 1;		// indicate that a 100 MHz digit has been painted
	}
	else if(!d_100mhz)	{	// no digit in the 100's MHz place?
		if(digit_100)	{	// was a digit present there before?
			LCD_PrintText((POS_TUNE_SFREQ_X - SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// yes - mask the leading first digit
			digit_100 = 0;		// clear flag indicating that there was a digit so that we do not "paint" at that location again
		}
	}
	// -----------------------
	// See if 10 Mhz needs update
	d_10mhz = (dial_freq%100000000)/10000000;
	if(d_10mhz != df.sdial_010_mhz)
	{
		//printf("10 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_10mhz & 0x0F);

		// Update segment
		if(d_100mhz)
			LCD_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
		else	{
			if(d_10mhz)
				LCD_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
			else
				LCD_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// mask the zero
		}
		// Save value
		df.sdial_010_mhz = d_10mhz;
		digit_10 = 1;		// indicate that a 10's MHz digit has been displayed
	}
	else if(!d_10mhz)	{	// no digit in the 10's MHz  place?
		if(digit_10)	{	// had a 10's MHz digit been painted?
			LCD_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// yes - mask the leading first digit
			digit_10 = 0;	// clear indicator so that a "blank" digit is not repainted every time
		}
	}

	// -----------------------
	// See if 1 Mhz needs update
	d_1mhz = (dial_freq%10000000)/1000000;
	if(d_1mhz != df.sdial_001_mhz)
	{
		//printf("1 mhz diff: %d\n\r",d_1mhz);

		// To string
		digit[0] = 0x30 + (d_1mhz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_mhz = d_1mhz;
	}

	// -----------------------
	// See if 100 khz needs update
	d_100khz = (dial_freq%1000000)/100000;
	if(d_100khz != df.sdial_100_khz)
	{
		//printf("100 khz diff: %d\n\r",d_100khz);

		// To string
		digit[0] = 0x30 + (d_100khz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*3),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_100_khz = d_100khz;
	}

	// -----------------------
	// See if 10 khz needs update
	d_10khz = (dial_freq%100000)/10000;
	if(d_10khz != df.sdial_010_khz)
	{
		//printf("10 khz diff: %d\n\r",d_10khz);

		// To string
		digit[0] = 0x30 + (d_10khz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*4),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_010_khz = d_10khz;
	}

	// -----------------------
	// See if 1 khz needs update
	d_1khz = (dial_freq%10000)/1000;
	if(d_1khz != df.sdial_001_khz)
	{
		//printf("1 khz diff: %d\n\r",d_1khz);

		// To string
		digit[0] = 0x30 + (d_1khz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*5),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_khz = d_1khz;
	}

	// -----------------------
	// See if 100 hz needs update
	d_100hz = (dial_freq%1000)/100;
	if(d_100hz != df.sdial_100_hz)
	{
		//printf("100 hz diff: %d\n\r",d_100hz);

		// To string
		digit[0] = 0x30 + (d_100hz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*7),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_100_hz = d_100hz;
	}

	// -----------------------
	// See if 10 hz needs update
	d_10hz = (dial_freq%100)/10;
	if(d_10hz != df.sdial_010_hz)
	{
		//printf("10 hz diff: %d\n\r",d_10hz);

		// To string
		digit[0] = 0x30 + (d_10hz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*8),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_010_hz = d_10hz;
	}

	// -----------------------
	// See if 1 hz needs update
	d_1hz = (dial_freq%10)/1;
	if(d_1hz != df.sdial_001_hz)
	{
		//printf("1 hz diff: %d\n\r",d_1hz);

		// To string
		digit[0] = 0x30 + (d_1hz & 0x0F);

		// Update segment
		LCD_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*9),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_hz = d_1hz;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckBand
//* Object              : Checks in which band the current frequency lies
//* Input Parameters    : frequency in Hz * 4, update = TRUE if display should be updated
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar UiDriverCheckBand(ulong freq, ushort update)
{
	uchar	band_scan;
	bool flag;
	static	uchar band_scan_old = 99;

	band_scan = 0;
	flag = 0;
	//
	if(ts.iq_freq_mode == 1)	// is frequency translate active and in "RX LO HIGH" mode?
		freq -= FREQ_SHIFT_MAG * 4;	// yes - subtract offset amount
	else if(ts.iq_freq_mode == 2)	// is frequency translate active and in "RX LO LOW" mode?
		freq += FREQ_SHIFT_MAG * 4;	// yes - add offset amount
	else if(ts.iq_freq_mode == 3)	// is frequency translate active and in "RX LO LOW" mode?
		freq -= FREQ_SHIFT_MAG * 8;	// yes - add offset amount
	else if(ts.iq_freq_mode == 4)	// is frequency translate active and in "RX LO LOW" mode?
		freq += FREQ_SHIFT_MAG * 8;	// yes - add offset amount


	while((!flag) && (band_scan < MAX_BANDS))	{
		if((freq >= tune_bands[band_scan]) && (freq <= (tune_bands[band_scan] + size_bands[band_scan])))	// Is this frequency within this band?
			flag = 1;	// yes - stop the scan
		else	// no - not in this band
			band_scan++;	// scan the next band qqqqq
	}
	//
	if(update)	{		// are we to update the display?
		if(band_scan != band_scan_old)		// yes, did the band actually change?
			UiDriverShowBand(band_scan);	// yes, update the display with the current band
	}
	band_scan_old = band_scan;	// update band change detector

	return band_scan;		// return with the band

}




void UiDriverUpdateFrequency(char skip_encoder_check) {
	ulong		loc_tune_new, dial_freq, second_freq;
	//uchar		old_rf_gain = ts.rf_gain;
	ushort		col = White;

	// On band change we don't have to check the encoder
	if (skip_encoder_check)
		goto skip_check;

	// Check encoder
	if (!UiDriverCheckFrequencyEncoder())
		return;

	skip_check:

	// Get value, while blocking update
	loc_tune_new = df.tune_new;

	// Calculate display frequency
	dial_freq = loc_tune_new / 4;

	// Calculate actual tune frequency
	ts.tune_freq = dial_freq*4;
	second_freq = ts.tune_freq;					// get copy for secondary display

	// Calculate actual tune frequency
	ts.tune_freq = dial_freq * 4;

	if(ts.iq_freq_mode == 1)
		ts.tune_freq += FREQ_SHIFT_MAG * 4;		// magnitude of shift is quadrupled at actual Si570 operating frequency
	else if(ts.iq_freq_mode == 2)
		ts.tune_freq -= FREQ_SHIFT_MAG * 4;
	else if(ts.iq_freq_mode == 3)
		ts.tune_freq += FREQ_SHIFT_MAG * 8;
	else if(ts.iq_freq_mode == 4)
		ts.tune_freq -= FREQ_SHIFT_MAG * 8;

	if((ts.tune_freq == ts.tune_freq_old))
		return;

	ts.tune_freq_old = ts.tune_freq;

#ifdef SI570
	if(ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 1))	{	// did the tuning require that a large tuning step occur?
//		if(ts.sysclock > RX_MUTE_START_DELAY)	{	// has system start-up completed?
//			ads.agc_holder = ads.agc_val;	// grab current AGC value as synthesizer "click" can momentarily desense radio as we tune
			ts.rx_muting = 1;				// yes - mute audio output
//			ts.dsp_inhibit_mute = ts.dsp_inhibit;		// get current status of DSP muting and save for later restoration
//			ts.dsp_inhibit = 1;				// disable DSP during tuning to avoid disruption
//			if(ts.dsp_active & 1)	// is DSP active?
//				ts.rx_blanking_time = ts.sysclock + TUNING_LARGE_STEP_MUTING_TIME_DSP_ON;	// yes - schedule un-muting of audio when DSP is on
//			else
//				ts.rx_blanking_time = ts.sysclock + TUNING_LARGE_STEP_MUTING_TIME_DSP_OFF;	// no - schedule un-muting of audio when DSP is off
	}

	// Set frequency
	if(ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0))
	{
	char test = ui_si570_set_frequency(ts.tune_freq,ts.freq_cal,df.temp_factor, 0);
		if(test == 1)
		    col = Red;	// Color in red if there was a problem setting frequency
		if(test == 2)
		    col = Yellow;	// Color in yellow if there was a problem setting frequency
	}
#endif

	Si5351_set_freq( (ts.tune_freq) /5  , SI5351_PLL_FIXED, SI5351_CLK0);

	// Update LCD
	UiDriverUpdateLcdFreq(dial_freq, col);

	UiDriverUpdateSecondLcdFreq(second_freq/4);
	//
	UiDriverCheckFilter(dial_freq);	// check the filter status with the new frequency update
	UiDriverCheckBand(ts.tune_freq, 1);

	// Allow clear of spectrum display in its state machine
	sd.dial_moved = 1;

	// Save current freq
	df.tune_old = loc_tune_new;

	// Save the tuning step used during the last dial update
	// - really important so we know what segments to clear
	// during tune step change
	df.last_tune_step = df.tunning_step;

	ads.agc_val = ads.agc_holder;	// restore stored AGC value

}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandlePowerSupply
//* Object              : display external voltage and to handle final power-off and delay
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverHandlePowerSupply(void)
{
	ulong	val_p, calib;
	uchar	v10,v100,v1000,v10000;
	//char 	cap1[50];
	char	digit[2];
	int		col;

//	char txt[32];

	pwmt.skip++;
	if(pwmt.skip < POWER_SAMPLES_SKP)
		return;

	pwmt.skip = 0;

	// Collect samples
	if(pwmt.p_curr < POWER_SAMPLES_CNT)
	{
		val_p = ADC_GetConversionValue(ADC1);

		// Add to accumulator
		pwmt.pwr_aver = pwmt.pwr_aver + val_p;
		pwmt.p_curr++;

		return;
	}


	// Get average
	val_p  = pwmt.pwr_aver/POWER_SAMPLES_CNT;

	calib = (ulong)ts.voltmeter_calibrate;	// get local copy of calibration factor
	calib += 900;					// offset to 1000 (nominal)
	val_p = (calib) * val_p;		// multiply by calibration factor, sample count and A/D scale full scale count
	val_p /= (1000);				// divide by 1000 (unity calibration factor), sample count and A/D full scale count

	// Correct for divider
	//val_p -= 550;
	val_p *= 4;

//	debug
//	sprintf(txt, "%d ", (int)(val_p));
//	LCD_PrintText    (POS_PWR_NUM_IND_X, POS_PWR_NUM_IND_Y,txt,Grey,Black,0);

	// Terminate
	digit[1] = 0;

	v10000 = (val_p%100000)/10000;
	v1000 = (val_p%10000)/1000;
	v100 = (val_p%1000)/100;
	v10 = (val_p%100)/10;
	//
	//
	col = COL_PWR_IND;	// Assume normal voltage, so Set normal color
	//
	if(val_p < 6500)		// below 9.5 volts
		col = Red;			// display red digits
	else if(val_p < 7000)	// below 10.5 volts
		col = Orange;		// make them orange
	else if(val_p < 7500)	// below 11.0 volts
		col = Yellow;		// make them yellow

	//
	// did we detect a voltage change?
	//
	if((v10000 != pwmt.v10000) || (v1000 != pwmt.v1000) || (v100 != pwmt.v100) || (v10 != pwmt.v10))	{	// Time to update - or was this the first time it was called?


		// -----------------------
		// 10V update

		//printf("10V diff: %d\n\r",v10000);

		// To string
		digit[0] = 0x30 + (v10000 & 0x0F);

		// Update segment
		if(v10000)
			LCD_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*0),POS_PWR_IND_Y,digit,col,Black,0);
		else
			LCD_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*0),POS_PWR_IND_Y,digit,Black,Black,0);

		// Save value
		pwmt.v10000 = v10000;


		// -----------------------
		// 1V update

		//printf("1V diff: %d\n\r",v1000);

		// To string
		digit[0] = 0x30 + (v1000 & 0x0F);

		// Update segment
		LCD_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*1),POS_PWR_IND_Y,digit,col,Black,0);

		// Save value
		pwmt.v1000 = v1000;

		// -----------------------
		// 0.1V update

		//printf("0.1V diff: %d\n\r",v100);

		// To string
		digit[0] = 0x30 + (v100 & 0x0F);

		// Update segment(skip the dot)
		LCD_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*3),POS_PWR_IND_Y,digit,col,Black,0);

		// Save value
		pwmt.v100 = v100;

		// -----------------------
		// 0.01Vupdate

		//printf("0.01V diff: %d\n\r",v10);

		// To string
		digit[0] = 0x30 + (v10 & 0x0F);

		// Update segment(skip the dot)
		LCD_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*4),POS_PWR_IND_Y,digit,col,Black,0);

		// Save value
		pwmt.v10 = v10;
	}


	// Reset accumulator
	pwmt.p_curr 	= 0;
	pwmt.pwr_aver 	= 0;
}

void UiDriverChangeVariable_1(uchar enabled, char variable, char *label)
{
	char	temp[100];
	ushort 	color = Grey;

	if(enabled)
		color = White;

	LCD_DrawEmptyRect( POS_RIT_IND_X,POS_RIT_IND_Y,13,57,Grey);

	if(enabled)
		LCD_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 1),label,Black,Grey,0);
	else
		LCD_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 1),label,Grey1,Grey,0);

	if(variable >= 0)
		sprintf(temp,"+%i",variable);
	else
		sprintf(temp,"%i", variable);

	LCD_PrintTextRight((POS_RIT_IND_X + 55),(POS_RIT_IND_Y + 1),"000",Black,Black,0); // clear screen
	LCD_PrintTextRight((POS_RIT_IND_X + 55),(POS_RIT_IND_Y + 1), temp,color,Black,0);
}



void UiDriverChangeVariable_2(uchar enabled, char variable, char *label)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	LCD_DrawEmptyRect( POS_KS_IND_X,POS_KS_IND_Y,13,49,Grey);

	if(enabled)
		LCD_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),label,Black,Grey,0);
	else
		LCD_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),label,Grey1,Grey,0);

	memset(temp,0,100);
	sprintf(temp,"%2d",variable);

	LCD_PrintText    ((POS_KS_IND_X + 30),(POS_KS_IND_Y + 1), temp,color,Black,0);

}

void UiDriverIndicadorFiltro(char *text)
{
	ushort color = White;

	// Draw top line
	LCD_DrawStraightLine(POS_PW_IND_X,(POS_PW_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	LCD_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),text,color,Blue,0);

}

void UiCalcAGCDecay(void)
{
	float tcalc;	// temporary holder - used to avoid conflict during operation

	// Set AGC rate - this needs to be moved to its own function (and the one in "ui_menu.c")
	//
	if(ts.agc_mode == AGC_SLOW)
		ads.agc_decay = AGC_SLOW_DECAY;
	else if(ts.agc_mode == AGC_FAST)
		ads.agc_decay = AGC_FAST_DECAY;
	else if(ts.agc_mode == AGC_CUSTOM)	{	// calculate custom AGC setting
		tcalc = (float)ts.agc_custom_decay;
		tcalc += 30;
		tcalc /= 10;
		tcalc = -tcalc;
		ads.agc_decay = powf(10, tcalc);
	}
	else
		ads.agc_decay = AGC_MED_DECAY;
}
