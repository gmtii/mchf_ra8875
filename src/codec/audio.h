/*
 * audio.h - audio processing routines
 */

#ifndef __AUDIO_H
#define __AUDIO_H

#include "radio.h"
#include "arm_math.h"

// -----------------------------
// With 128 words buffer we have
// 120uS processing time in the
// IRQ and 530uS idle time
// (48kHz sampling - USB)
//
#define BUFF_LEN 			128
//
// -----------------------------
// Half of total buffer
#define	IQ_BUFSZ 			(BUFF_LEN/2)

// Audio filter
#define FIR_RXAUDIO_BLOCK_SIZE		1
#define FIR_RXAUDIO_NUM_TAPS		48

#define FFT_IQ_BUFF_LEN				2048

// FFT will work with quadrature signals
#define FFT_QUADRATURE_PROC			1

#define DEMOD_USB					0
#define DEMOD_LSB					1
#define DEMOD_CW					2
#define DEMOD_AM					3
#define DEMOD_DIGI					4
#define DEMOD_MAX_MODE				4

#define	WFALL_MEDIUM_ADDITIONAL	12
//
// AGC Time constants
// C. Turner, KA7OEI
//
#define	AGC_KNEE	1000//4000	// ADC "knee" threshold for AGC action
//
#define	AGC_KNEE_REF		1000
#define	AGC_VAL_MAX_REF		131072//4096
#define	POST_AGC_GAIN_SCALING_REF	1.333

#define	AGC_ATTACK		0.033	// Attack time multiplier for AGC
//
#define AGC_FAST_DECAY	0.0002	// Decay rate multiplier for "Fast" AGC
#define AGC_MED_DECAY	0.00006	// Decay rate multiplier for "Medium" AGC
#define AGC_SLOW_DECAY	0.00001 // Decay rate for multiplier  "Slow" AGC

#define	AGC_VAL_MIN		0.02	// Minimum AGC gain multiplier (e.g. gain reduction of 34dB)
//#define AGC_VAL_MAX		4096//1024	// Maximum AGC gain multiplier (e.g. gain multiplication of 60dB)

#define	AGC_PREFILTER_MAXGAIN	5 	// Scaling factor for RF gain adjustment (e.g. factor by which RFG will be multiplied to yield actual RFG multiplier
#define AGC_PREFILTER_MINGAIN	0.5	// Minimum "RFG" gain multiplier (e.g. gain reduction of 6 dB)
//
#define AGC_PREFILTER_HISIG_THRESHOLD	0.1	// Threshold at which adjustment of RFGAIN (pre-filter) gain adjustment will occur
#define AGC_PREFILTER_LOWSIG_THRESHOLD	1.0	// Threshold at which adjustment of RFGAIN (pre-filter) gain adjustment will occur
#define AGC_PREFILTER_ATTACK_RATE		0.0002	// Attack rate for RFG reduction
#define AGC_PREFILTER_DECAY_RATE		0.000002	// Decay rate for RFG gain recovery
//
#define AGC_PREFILTER_MAX_SIGNAL	1		// maximum level of pre-filtered signal
//
#define POST_AGC_GAIN_SCALING	1.333//0.333	// Used to rescale the post-filter audio level to a value suitable for the codec.  This sets the line level output
									// to approx. 1000mV peak-peak.
//
#define	POST_AGC_GAIN_SCALING_DECIMATE_4	3.46	// Used to scale audio from the decimation/interpolation-by-4 process (based square root of decimation factor)
//
#define	POST_AGC_GAIN_SCALING_DECIMATE_2	(POST_AGC_GAIN_SCALING_DECIMATE_4 * 0.6)	// Scales audio from decimation/interpolation-by-2 process
//
#define	AM_SCALING		2		// Amount of gain multiplication to apply to audio and AGC to make recovery equal to that of SSB
#define	AM_AUDIO_SCALING	1.4	// Additional correction factor applied to audio demodulation to make amplitude equal to that of SSB demodulation
//
#define	AGC_GAIN_CAL	15500	//22440		// multiplier value (linear, not DB) to calibrate the S-Meter reading to the AGC value
//
#define	AUTO_RFG_DECREASE_LOCKOUT	1
#define	AUTO_RFG_INCREASE_TIMER		5//10
//
#define	AGC_SLOW			0		// Mode setting for slow AGC
#define	AGC_MED				1		// Mode setting for medium AGC
#define	AGC_FAST			2		// Mode setting for fast AGC
#define	AGC_CUSTOM			3		// Mode setting for custom AGC
#define	AGC_OFF				4		// Mode setting for AGC off
#define	AGC_MAX_MODE		4		// Maximum for mode setting for AGC
#define	AGC_DEFAULT			AGC_MED	// Default!
//
#define	AGC_CUSTOM_MAX		30		// Maximum (slowest) setting for custom AGC
#define	AGC_CUSTOM_DEFAULT		12		// Default custom AGC setting (approx. equal to "medium")
#define AGC_CUSTOM_FAST_WARNING		2	// Value at or below which setting the custom AGC is likely to degrade audio
//
#define	MAX_RF_GAIN_MAX			6		// Maximum setting for "Max RF gain"
#define	MAX_RF_GAIN_DEFAULT		3

#define	AGC_DELAY_BUFSIZE		(BUFF_LEN/2)*5	// Size of AGC delaying audio buffer - Must be a multiple of BUFF_LEN/2.
											// This is divided by the decimation rate so that the time delay is constant.
//
#define	ALC_DELAY_BUFSIZE		(BUFF_LEN/2)*5		// Size of AGC delaying audio buffer - Must be a multiple of BUFF_LEN/2.

#define	RX_DECIMATION_RATE_12KHZ		4		// Decimation/Interpolation rate in receive function for 12 kHz rates
#define	RX_DECIMATION_RATE_24KHZ		2		// Decimation/Interpolation rate in receive function for 24 kHz rates

// ************
// DSP system parameters
//
// Noise reduction
//
#define	LMS_NR_DELAYBUF_SIZE_MAX		512	// maximum size of LMS delay buffer for the noise reduction
//
//
#define	DSP_NR_STRENGTH_MAX		55//35	// Maximum menu setting for DSP "Strength"
#define	DSP_NR_STRENGTH_DEFAULT	40	// Default setting
//
#define	DSP_STRENGTH_YELLOW		25	// Threshold at and above which DSP number is yellow
#define	DSP_STRENGTH_ORANGE		35	// Threshold at and above which DSP number is orange
#define DSP_STRENGTH_RED		45	// Threshold at and above which DSP number is red
//
//
#define	DSP_NR_BUFLEN_MIN		48
#define	DSP_NR_BUFLEN_MAX		LMS_NR_DELAYBUF_SIZE_MAX
#define	DSP_NR_BUFLEN_DEFAULT	192
//
#define DSP_NR_NUMTAPS_MIN		32
#define	DSP_NR_NUMTAPS_MAX		128
#define	DSP_NR_NUMTAPS_DEFAULT	96
//
#define	MAX_DSP_ZERO_COUNT		2048
#define	DSP_ZERO_COUNT_ERROR	512
#define	DSP_ZERO_DET_MULT_FACTOR	10000000
#define	DSP_OUTPUT_MINVAL		1
#define	DSP_HIGH_LEVEL			10000
#define	DSP_CRASH_COUNT_THRESHOLD	35
//
// Automatic Notch Filter
//
#define	LMS_NOTCH_DELAYBUF_SIZE_MAX	1024
//
#define	DSP_NOTCH_NUM_TAPS		64		// number of taps for the DSP NOTCH FIR
//
#define	DSP_NOTCH_BUFLEN_MIN	96
#define	DSP_NOTCH_BUFLEN_MAX	LMS_NOTCH_DELAYBUF_SIZE_MAX
#define	DSP_NOTCH_DELAYBUF_DEFAULT	192
//
#define	DSP_NOTCH_MU_MAX		35
#define	DSP_NOTCH_MU_DEFAULT	10

#define	ADC_CLIP_WARN_THRESHOLD	4096

#define	FFT_IQ_BUFF_M1_HALF		(FFT_IQ_BUFF_LEN-1)/2

#define	FREQ_SHIFT_MAG	6000		// Amount of frequency shift, in Hz, when software frequency shift is enabled (exactly 1/8th of the sample rate prior to decimation!)
//
#define	FREQ_IQ_CONV_MODE_OFF		0	// No frequency conversion
#define FREQ_IQ_CONV_P6KHZ			1	// LO is 6KHz above receive frequency in RX mode
#define	FREQ_IQ_CONV_M6KHZ			2	// LO is 6KHz below receive frequency in RX mode
#define FREQ_IQ_CONV_P12KHZ			3	// LO is 12KHz above receive frequency in RX mode
#define	FREQ_IQ_CONV_M12KHZ			4	// LO is 12KHz below receive frequency in RX mode

//
#define	FREQ_IQ_CONV_MODE_DEFAULT	FREQ_IQ_CONV_M12KHZ		//FREQ_IQ_CONV_MODE_OFF
#define	FREQ_IQ_CONV_MODE_MAX		4

// Noise blanker constants
//
#define	NBLANK_AGC_ATTACK	0.33	// Attack time multiplier for AGC
//
#define NBLANK_AGC_DECAY	0.002	// Decay rate multiplier for "Fast" AGC
//
#define	MAX_NB_SETTING		20
#define	NB_WARNING1_SETTING	8		// setting at or above which NB warning1 (yellow) is given
#define	NB_WARNING2_SETTING	12		// setting at or above which NB warning2 (orange) is given
#define	NB_WARNING3_SETTING	16		// setting at or above which NB warning3 (red) is given
#define	NB_DURATION			4
//
#define	NB_AGC_FILT			0.999	// Component of IIR filter for recyling previous AGC value
#define	NB_SIG_FILT			0.001	// Component of IIR filter for present signal value's contribution to AGC
//
#define	NB_AVG_WEIGHT		0.80	// Weighting applied to average based on past signal for NB2
#define	NB_SIG_WEIGHT		0.20	// Weighting applied to present signal for NB2
//
//
#define	NB_MAX_AGC_SETTING	35		// maximum setting for noise blanker setting
#define	NB_AGC_DEFAULT		20		// Default setting for noise blanker AGC time constant adjust

#define	MIN_CUST_AGC_VAL	10	// Minimum and maximum RF gain settings
#define	MAX_CUST_AGC_VAL	30
#define	CUST_AGC_OFFSET_VAL	30	// RF Gain offset value used in calculations
#define	CUST_AGC_VAL_DEFAULT	17.8	// Value for "medium" AGC value
//
#define	LINE_OUT_SCALING_FACTOR	10		// multiplication of audio for fixed LINE out level (nominally 1vpp)

#define NUMBER_WATERFALL_COLOURS	64

//
// Audio driver publics
typedef struct AudioDriverState
{
	// Stereo buffers
	float32_t					i_buffer[IQ_BUFSZ+1];
	float32_t 					q_buffer[IQ_BUFSZ+1];
	float32_t 					a_buffer[IQ_BUFSZ+1];
	float32_t 					b_buffer[IQ_BUFSZ+1];
	float32_t					c_buffer[IQ_BUFSZ+1];
	float32_t					d_buffer[IQ_BUFSZ+1];
	float32_t					e_buffer[IQ_BUFSZ+1];
	float32_t					f_buffer[IQ_BUFSZ+1];
	//
	// Lock audio filter flag
	uchar					af_dissabled;

	uchar					tx_filter_adjusting;	// used to disable TX I/Q filter during phase adjustment

	// AGC and audio related variables

	float 					agc_val;			// "live" receiver AGC value
	float					agc_valbuf[BUFF_LEN];	// holder for "running" AGC value
	float					agc_holder;			// used to hold AGC value during transmit and tuning
	float					agc_decay;			// decay rate (speed) of AGC
	float					agc_rf_gain;
	float					agc_knee;			// "knee" for AGC operation
	float					agc_val_max;		// maximum AGC gain (at minimum signal)
	float					am_agc;				// Signal level reading in AM demod mode
	//
	float					pre_filter_gain;
	uchar					codec_gain;
	float					codec_gain_calc;
	bool					adc_clip;
	bool					adc_half_clip;
	bool					adc_quarter_clip;
	float					peak_audio;			// used for audio metering to detect the peak audio level

	float					alc_val;			// "live" transmitter ALC value
	float					alc_decay;			// decay rate (speed) of ALC
	float					post_agc_gain;		// post AGC gain scaling

	uchar					decimation_rate;		// current decimation/interpolation rate
	ulong					agc_delay_buflen;		// AGC delay buffer length
	float					agc_decimation_scaling;	// used to adjust AGC timing based on sample rate
	//
	float					nb_agc_filt;			// used for the filtering/determination of the noise blanker AGC level
	float					nb_sig_filt;
	ulong					dsp_zero_count;			// used for detecting zero output from DSP which can occur if it crashes
	float					dsp_nr_sample;			// used for detecting a problem with the DSP (e.g. crashing)
	//
	float					Osc_Cos;
	float					Osc_Sin;
	float					Osc_Vect_Q;
	float					Osc_Vect_I;
	float					Osc_Q;
	float					Osc_I;
	float					Osc_Gain;


} AudioDriverState;

// Spectrum display
typedef struct SpectrumDisplay
{
	// Samples buffer
	float32_t	FFT_Samples[FFT_IQ_BUFF_LEN];
	float32_t	FFT_Windat[FFT_IQ_BUFF_LEN/2];
	float32_t	FFT_MagData[FFT_IQ_BUFF_LEN/2];
	float32_t	FFT_AVGData[FFT_IQ_BUFF_LEN/2];		// IIR low-pass filtered FFT buffer data

	q15_t		FFT_BkpData[FFT_IQ_BUFF_LEN];
	q15_t		FFT_DspData[FFT_IQ_BUFF_LEN];		// Rescaled and de-linearized display data
	q15_t   	FFT_TempData[FFT_IQ_BUFF_LEN];

	// Current data ptr
	ulong	samp_ptr;

	// Skip flag for FFT processing
	ulong	skip_process;

	// Addresses of vertical grid lines on x axis
	ushort  vert_grid_id[7];

	// Addresses of horizontal grid lines on x axis
	ushort  horz_grid_id[3];

	// State machine current state
	uchar 	state;

	// Init done flag
	uchar 	enabled;

	// Flag to indicate frequency change,
	// we need it to clear spectrum control
	uchar	dial_moved;

	// Variables used in spectrum display AGC
	//
	//
	float mag_calc;		// spectrum display rescale control

	uchar	use_spi;	// TRUE if display uses SPI mode

	uchar	magnify;	// TRUE if in magnify mode

	float	rescale_rate;	// this holds the rate at which the rescaling happens when the signal appears/disappears
	float	display_offset;									// "vertical" offset for spectral scope, gain adjust for waterfall
	float	agc_rate;		// this holds AGC rate for the Spectrum Display
	float	db_scale;		// scaling factor for dB/division
	ushort	wfall_line_update;	// used to set the number of lines per update on the waterfall
	float	wfall_contrast;	// used to adjust the contrast of the waterfall display

	ushort	waterfall_colours[NUMBER_WATERFALL_COLOURS+1];	// palette of colors for waterfall data
	//float32_t	wfall_temp[512/2];					// temporary holder for rescaling screen
	//ushort	waterfall[SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL +16][512/2];	// circular buffer used for storing waterfall data - remember to increase this if the waterfall is made larger!
	ushort	waterfall[70 + WFALL_MEDIUM_ADDITIONAL][800];	// circular buffer used for storing waterfall data - remember to increase this if the waterfall is made larger!

	uchar	wfall_line;										// pointer to current line of waterfall data
	uchar 	wfall_size;					// vertical size of the waterfall
	uchar	wfall_height;
	uchar	wfall_ystart;

	uint8_t     first_run;

} SpectrumDisplay;

// -----------------------------
// S meter sample time
#define S_MET_SAMP_CNT		512

// S meter drag delay
#define S_MET_UPD_SKIP		5000	//10000

// S meter public
typedef struct SMeter
{
	ulong	skip;

	ulong	s_count;
	float	gain_calc;
	int		curr_max;

	uchar	old;
	//uchar	max_upd;

} SMeter;

void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz, uint16_t ht);
void audio_driver_set_rx_audio_filter(void);
void Audio_Init(void);

#endif

