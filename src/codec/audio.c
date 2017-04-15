/*
 *
 */

#include "audio.h"
#include "radio.h"

// Audio filters

#include "filters/iir_300hz.h"
#include "filters/iir_500hz.h"
#include "filters/iir_1_8k.h"
#include "filters/iir_2_3k.h"
#include "filters/iir_2_7k.h"
#include "filters/iir_2_9k.h"
//#include "filters/iir_3k.h"
#include "filters/iir_3_6k.h"
#include "filters/iir_10k.h"

//
#include "filters/fir_rx_decimate_4.h"	// with low-pass filtering
#include "filters/fir_rx_decimate_4_min_lpf.h"	// This has minimized LPF for the 10 kHz filter mode
#include "filters/fir_rx_interpolate_16.h"	// filter for interpolate-by-16 operation
#include "filters/fir_rx_interpolate_16_10kHz.h"	// This has relaxed LPF for the 10 kHz filter mode


// Spectrum display public
SpectrumDisplay	__attribute__ ((section (".ccm"))) 		sd;

// Audio driver publics
AudioDriverState  	ads;

// S meter public
__IO	SMeter					sm;

// Transceiver state public structure
extern __IO TransceiverState 	ts;

// ---------------------------------
// Audio RX filter
__IO arm_fir_instance_f32 	FIR_A;
__IO float32_t 				firState_A[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];

//
// Audio RX - Decimator
static	arm_fir_decimate_instance_f32	DECIMATE_RX;
__IO float32_t			decimState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];
//
// Audio RX - Interpolator
static	arm_fir_interpolate_instance_f32 INTERPOLATE_RX;
__IO float32_t			interpState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];

//
// RX Hilbert transform (90 degree) FIR filters
//

__IO	arm_fir_instance_f32 	FIR_I;
__IO	arm_fir_instance_f32 	FIR_Q;

//
// variables for RX IIR filters
static float32_t		iir_rx_state[FIR_RXAUDIO_BLOCK_SIZE+FIR_RXAUDIO_NUM_TAPS-1];
static arm_iir_lattice_instance_f32	IIR_PreFilter;

//
float32_t	lms1_nr_delay[LMS_NR_DELAYBUF_SIZE_MAX+16];
//
float32_t	lms2_nr_delay[LMS_NOTCH_DELAYBUF_SIZE_MAX + 16];
//
//
float32_t errsig1[65];
float32_t errsig2[65];
float32_t result[65];

//
// LMS Filters for RX
arm_lms_norm_instance_f32	lms1Norm_instance;
arm_lms_instance_f32	lms1_instance;
float32_t	lms1StateF32[192];
float32_t	lms1NormCoeff_f32[192];
//
arm_lms_norm_instance_f32	lms2Norm_instance;
arm_lms_instance_f32	lms2_instance;
float32_t	lms2StateF32[192];
float32_t	lms2NormCoeff_f32[192];

float32_t	agc_delay	[AGC_DELAY_BUFSIZE+16];

/* Stereo buffers */
#define STEREO_BUFSZ (BUFF_LEN/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)

int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ];

float32_t					osc_q[IQ_BUFSZ];
float32_t					osc_i[IQ_BUFSZ];

//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_set_rx_audio_filter
//* Object              :
//* Object              : select audio filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void audio_driver_set_rx_audio_filter(void)
{
	uint32_t	i;
	float	mu_calc;
	bool	dsp_inhibit_temp;

	// Lock - temporarily disable filter
	ads.af_dissabled = 1;

	switch(ts.filter_id)	{
		case AUDIO_300HZ:
		    IIR_PreFilter.numStages = IIR_300hz_numStages;		// number of stages
		    if(ts.filter_300Hz_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_500_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_500_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 2)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_550_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_550_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 3)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_600_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_600_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_650_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_650_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_700_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_700_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 7)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_800_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_800_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 8)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_850_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_850_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 9)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_900_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_900_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 10)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_950_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_950_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_750_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_750_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_500HZ:
		    IIR_PreFilter.numStages = IIR_500hz_numStages;		// number of stages
		    if(ts.filter_500Hz_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_550_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_550_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_500Hz_select == 2)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_650_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_650_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_500Hz_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_850_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_850_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_500Hz_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_950_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_950_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_750_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_750_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_1P8KHZ:
		    IIR_PreFilter.numStages = IIR_1k8_numStages;		// number of stages
		    if(ts.filter_1k8_select == 6)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_LPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_LPF_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k125_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k125_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 2)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k275_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k275_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k575_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k575_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k725_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k725_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k425_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k425_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_2P3KHZ:
		    IIR_PreFilter.numStages = IIR_2k3_numStages;		// number of stages
		    if(ts.filter_2k3_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_LPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_LPF_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_2k3_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k275_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k275_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_2k3_select == 3)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k562_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k562_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_2k3_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k712_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k712_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k412_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k412_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_2P7KHZ:
		    IIR_PreFilter.numStages = IIR_2k7_numStages;		// number of stages
		    if(ts.filter_2k7_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k7_LPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k7_LPF_pvCoeffs;	// point to ladder coefficients
		    }
		    else 	{  // default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k7_BPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k7_BPF_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_2P9KHZ:
		    IIR_PreFilter.numStages = IIR_2k9_numStages;		// number of stages
		    if(ts.filter_2k9_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k9_LPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k9_LPF_pvCoeffs;	// point to ladder coefficients
		    }
		    else 	{  // default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k9_BPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k9_BPF_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_3P6KHZ:
		   	IIR_PreFilter.numStages = IIR_3k6_numStages;		// number of stages
		    if(ts.filter_3k6_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_3k6_LPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_3k6_LPF_pvCoeffs;	// point to ladder coefficients
		    }
		    else 	{  // default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_3k6_BPF_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_3k6_BPF_pvCoeffs;	// point to ladder coefficients
		    }
			break;
//			IIR_PreFilter.numStages = IIR_3k_numStages;		// number of stages
//			IIR_PreFilter.pkCoeffs = (float *)IIR_3k_pkCoeffs;	// point to reflection coefficients
//			IIR_PreFilter.pvCoeffs = (float *)IIR_3k_pvCoeffs;	// point to ladder coefficients
		case AUDIO_WIDE:
		    IIR_PreFilter.numStages = IIR_10k_numStages;		// number of stages
			IIR_PreFilter.pkCoeffs = (float *)IIR_10k_pkCoeffs;	// point to reflection coefficients
			IIR_PreFilter.pvCoeffs = (float *)IIR_10k_pvCoeffs;	// point to ladder coefficients
			break;
		default:
			break;
	}
		//
		// Initialize IIR filter state buffer
	 	//
	    for(i = 0; i < FIR_RXAUDIO_BLOCK_SIZE+FIR_RXAUDIO_NUM_TAPS-1; i++)	{	// initialize state buffer to zeroes
	    	iir_rx_state[i] = 0;
	    }
		IIR_PreFilter.pState = (float32_t *)&iir_rx_state;					// point to state array for IIR filter

		//
		// Initialize LMS (DSP Noise reduction) filter
		// It is (sort of) initalized "twice" since this it what it seems to take for the LMS function to
		// start reliably and consistently!
		//
		uint16_t	calc_taps;
		//
		if((ts.dsp_nr_numtaps < DSP_NR_NUMTAPS_MIN) || (ts.dsp_nr_numtaps > DSP_NR_NUMTAPS_MAX))
			calc_taps = DSP_NR_NUMTAPS_DEFAULT;
		else
			calc_taps = (uint16_t)ts.dsp_nr_numtaps;
		//
		// Load settings into instance structure
		//
		// LMS instance 1 is pre-AGC DSP NR
		// LMS instance 3 is post-AGC DSP NR
		//
		lms1Norm_instance.numTaps = calc_taps;
		lms1Norm_instance.pCoeffs = lms1NormCoeff_f32;
		lms1Norm_instance.pState = lms1StateF32;
		//
		// Calculate "mu" (convergence rate) from user "DSP Strength" setting.  This needs to be significantly de-linearized to
		// squeeze a wide range of adjustment (e.g. several magnitudes) into a fairly small numerical range.
		//
		mu_calc = (float)ts.dsp_nr_strength;		// get user setting
		/*
		mu_calc = DSP_NR_STRENGTH_MAX-mu_calc;		// invert (0 = minimum))
		mu_calc /= 2.6;								// scale calculation
		mu_calc *= mu_calc;							// square value
		mu_calc += 1;								// offset by one
		mu_calc /= 40;								// rescale
		mu_calc += 1;								// prevent negative log result
		mu_calc = log10f(mu_calc);					// de-linearize
		lms1Norm_instance.mu = mu_calc;				//
		*/
		//
		// New DSP NR "mu" calculation method as of 0.0.214
		//
		mu_calc /= 2;	// scale input value
		mu_calc += 2;	// offset zero value
		mu_calc /= 10;	// convert from "bels" to "deci-bels"
		mu_calc = powf(10,mu_calc);		// convert to ratio
		mu_calc = 1/mu_calc;			// invert to fraction
		lms1Norm_instance.mu = mu_calc;

		// Debug display of mu calculation
	//	char txt[16];
	//	sprintf(txt, " %d ", (ulong)(mu_calc * 10000));
	//	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,txt,0xFFFF,0,0);
	//
	//
		for(i = 0; i < LMS_NR_DELAYBUF_SIZE_MAX; i++)	{		// clear LMS delay buffers
			lms1_nr_delay[i] = 0;
		}
		//
		for(i = 0; i < 192; i++)	{		// clear LMS state buffer
			lms1StateF32[i] = 0;			// zero state buffer
			if(ts.reset_dsp_nr)	{			// are we to reset the coefficient buffer as well?
				lms1NormCoeff_f32[i] = 0;		// yes - zero coefficient buffers
			}
		}
		//
		// use "canned" init to initialize the filter coefficients
		//
		arm_lms_norm_init_f32(&lms1Norm_instance, calc_taps, &lms1NormCoeff_f32[0], &lms1StateF32[0], (float32_t)mu_calc, 64);
		//
		//
		if((ts.dsp_nr_delaybuf_len > DSP_NR_BUFLEN_MAX) || (ts.dsp_nr_delaybuf_len < DSP_NR_BUFLEN_MIN))
				ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
		//
		// LMS instance 2 - Automatic Notch Filter
		//
		calc_taps = DSP_NOTCH_NUM_TAPS;
		lms2Norm_instance.numTaps = calc_taps;
		lms2Norm_instance.pCoeffs = lms2NormCoeff_f32;
		lms2Norm_instance.pState = lms2StateF32;
		//
		// Calculate "mu" (convergence rate) from user "Notch ConvRate" setting
		//
		mu_calc = (float)ts.dsp_notch_mu;		// get user setting
		mu_calc += 1;
		mu_calc /= 1500;
		mu_calc += 1;
		mu_calc = log10f(mu_calc);
		//
		// use "canned" init to initialize the filter coefficients
		//
		arm_lms_norm_init_f32(&lms2Norm_instance, calc_taps, &lms2NormCoeff_f32[0], &lms2StateF32[0], (float32_t)mu_calc, 64);

		//
		for(i = 0; i < LMS_NOTCH_DELAYBUF_SIZE_MAX ; i++)		// clear LMS delay buffer
			lms2_nr_delay[i] = 0;
		//
		for(i = 0; i < 192; i++)	{		// clear LMS state and coefficient buffers
			lms2StateF32[i] = 0;			// zero state buffer
			if(ts.reset_dsp_nr)				// are we to reset the coefficient buffer?
				lms2NormCoeff_f32[i] = 0;		// yes - zero coefficient buffer
		}
		//
		if((ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
					ts.dsp_nr_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
		//
		// Adjust decimation rate based on selected filter
		//
		if(ts.filter_id != AUDIO_WIDE)	{		// Not 10 kHz receiver bandwidth
			ads.decimation_rate = RX_DECIMATION_RATE_12KHZ;
			DECIMATE_RX.pCoeffs = &FirRxDecimate[0];		// Filter coefficients for lower-rate (slightly strong LPF)
			INTERPOLATE_RX.pCoeffs = &FirRxInterpolate[0];	// Filter coefficients
		}
		else	{								// This *IS* the 10 kHz receiver bandwidth
			ads.decimation_rate = RX_DECIMATION_RATE_24KHZ;
			DECIMATE_RX.pCoeffs = &FirRxDecimateMinLPF[0];	// Filter coefficients for higher rate (weak LPF:  Hilbert is used for main LPF!)
			INTERPOLATE_RX.pCoeffs = &FirRxInterpolate10KHZ[0];	// Filter coefficients for higher rate (relaxed LPF)
		}
		//
		ads.agc_decimation_scaling = (float)ads.decimation_rate;
		ads.agc_delay_buflen = AGC_DELAY_BUFSIZE/(ulong)ads.decimation_rate;	// calculate post-AGC delay based on post-decimation sampling rate
		//
	    // Set up RX decimation/filter
		DECIMATE_RX.M = ads.decimation_rate;			// Decimation factor  (48 kHz / 4 = 12 kHz)
		DECIMATE_RX.numTaps = RX_DECIMATE_NUM_TAPS;		// Number of taps in FIR filter

		DECIMATE_RX.pState = &decimState[0];			// Filter state variables
		//
		// Set up RX interpolation/filter
		// NOTE:  Phase Length MUST be an INTEGER and is the number of taps divided by the decimation rate, and it must be greater than 1.
		//
		INTERPOLATE_RX.L = ads.decimation_rate;			// Interpolation factor, L  (12 kHz * 4 = 48 kHz)
		INTERPOLATE_RX.phaseLength = RX_INTERPOLATE_NUM_TAPS/ads.decimation_rate;	// Phase Length ( numTaps / L )
		INTERPOLATE_RX.pState = &interpState[0];		// Filter state variables
		//
		for(i = 0; i < FIR_RXAUDIO_BLOCK_SIZE; i++)	{	// Initialize all filter state variables
			decimState[i] = 0;
			interpState[i] = 0;
		}
		//
		ads.dsp_zero_count = 0;		// initialize "zero" count to detect if DSP has crashed
		//
		// Unlock - re-enable filtering
		//
		ads.af_dissabled = 0;
		ts.dsp_inhibit = dsp_inhibit_temp;
		//

}

/**
  * @brief  This function sets up audio processing. 
  * @param  none
  * @retval none
  */
void Audio_Init(void)
{
	uchar x,i;

	ads.agc_val = 1;			// Post AF Filter gain (AGC)
	ads.agc_holder = 1;			// initialize holder for AGC value
	for(x = 0; x < BUFF_LEN; x++)	// initialize running buffer for AGC delay
		ads.agc_valbuf[x] = 1;

	ads.agc_knee = AGC_KNEE/2;
	ads.agc_val_max = AGC_VAL_MAX_REF / MAX_RF_GAIN_DEFAULT+1;

	//

	ads.agc_rf_gain = (float)ts.rf_gain;
	ads.agc_rf_gain -= 20;
	ads.agc_rf_gain /= 10;
	ads.agc_rf_gain = powf(10, ads.agc_rf_gain);

	ads.decimation_rate=1*2;

	ads.agc_delay_buflen = AGC_DELAY_BUFSIZE/(ulong)ads.decimation_rate;

	//
	// Set AGC rate
	//
	if(ts.agc_mode == AGC_SLOW)
		ads.agc_decay = AGC_SLOW_DECAY;
	else if(ts.agc_mode == AGC_FAST)
		ads.agc_decay = AGC_FAST_DECAY;
	else if(ts.agc_mode == AGC_CUSTOM)	{	// calculate custom AGC setting
		ads.agc_decay = (float)ts.agc_custom_decay;
		ads.agc_decay += 30;
		ads.agc_decay /= 10;
		ads.agc_decay = -ads.agc_decay;
		ads.agc_decay = powf(10, ads.agc_decay);
	}
	else
		ads.agc_decay = AGC_MED_DECAY;


	if(ts.max_rf_gain <= MAX_RF_GAIN_MAX)	{
		ads.agc_knee = AGC_KNEE_REF * (float)(ts.max_rf_gain + 1);
		ads.agc_val_max = AGC_VAL_MAX_REF / ((float)(ts.max_rf_gain + 1));
		ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF / (float)(ts.max_rf_gain + 1);
	}
	else	{
		ads.agc_knee = AGC_KNEE_REF * MAX_RF_GAIN_DEFAULT+1;
		ads.agc_val_max = AGC_VAL_MAX_REF / MAX_RF_GAIN_DEFAULT+1;
		ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF /  (float)(ts.max_rf_gain + 1);
	}

	UiCalcNB_AGC();		// set up noise blanker AGC values

	// -------------------
	// Init RX audio filters
	audio_driver_set_rx_audio_filter();

	ads.af_dissabled = 0;

}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_noise_blanker
//* Object              : noise blanker
//* Object              :
//* Input Parameters    : I/Q 16 bit audio data, size of buffer
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_noise_blanker(int16_t *src, int16_t size)
{
	static int16_t	delay_buf[34];
	static uchar	delbuf_inptr = 0, delbuf_outptr = 2;
	ulong	i;
	float	sig;
	float  nb_short_setting;
//	static float avg_sig;
	static	uchar	nb_delay = 0;
	static float	nb_agc = 0;
	//
	if((!ts.nb_setting) || (ts.nb_disable) || (ts.dmod_mode == DEMOD_AM) || (ts.filter_id == AUDIO_WIDE))	{// bail out if noise blanker disabled, in AM mode, or set to 10 kHz
		return;
	}

	nb_short_setting = (float)ts.nb_setting;		// convert and rescale NB1 setting for higher resolution in adjustment
	nb_short_setting /= 2;

	for(i = 0; i < size/2; i+=4)	{		// Noise blanker function - "Unrolled" 4x for maximum execution speed
		//
		sig = (float)fabs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (ads.nb_agc_filt * nb_agc) + (ads.nb_sig_filt * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
		//
		// Next "unrolled" instance
		//
		sig = (float)fabs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (NB_AGC_FILT * nb_agc) + (NB_SIG_FILT * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
		//
		// Next "unrolled" instance
		//
		sig = (float)fabs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (NB_AGC_FILT * nb_agc) + (NB_SIG_FILT * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
		//
		// Last "unrolled" instance
		//
		sig = (float)fabs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (NB_AGC_FILT * nb_agc) + (NB_SIG_FILT * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
	}
}


//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_agc_processor
//* Object              :
//* Object              : Processor for receiver AGC
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_agc_processor(int16_t psize)
{
	static float		agc_calc, agc_var;
	static ulong 		i;
	static ulong		agc_delay_inbuf = 0, agc_delay_outbuf = 0;

	//
	// AGC function - Look-ahead type by KA7OEI, revised September 2015 to eliminate possible low-order, low-frequency instabilities associated with steady-state signals.
	// Note that even though it gets only one AGC value per cycle, it *does* do "psize/2" calculations to iterate out the AGC value more precisely than it would
	// were it called once per DMA cycle.  If it is called once per DMA cycle it will tend to weakly oscillate under certain conditions and possibly overshoot/undershoot.
	//
	for(i = 0; i < psize/2; i++)	{
		if(ts.agc_mode != AGC_OFF)	{
			if(ts.dmod_mode == DEMOD_AM)		// if in AM, get the recovered DC voltage from the detected carrier
				agc_calc = ads.am_agc * ads.agc_val;
			else	{							// not AM - get the amplitude of the recovered audio
				agc_calc = fabs(ads.a_buffer[i]) * ads.agc_val;
				//agc_calc = max_signal * ads.agc_val;	// calculate current level by scaling it with AGC value
			}
			//
			if(agc_calc < ads.agc_knee)	{	// is audio below AGC "knee" value?
				agc_var = ads.agc_knee - agc_calc;	// calculate difference between agc value and "knee" value
				agc_var /= ads.agc_knee;	// calculate ratio of difference between knee value and this value
				ads.agc_val += ads.agc_val*ads.agc_decay * ads.agc_decimation_scaling * agc_var;	// Yes - Increase gain slowly for AGC DECAY - scale time constant with decimation
			}
			else	{
				agc_var = agc_calc - ads.agc_knee;	// calculate difference between agc value and "knee" value
				agc_var /= ads.agc_knee;	// calculate ratio of difference between knee value and this value
				ads.agc_val -= ads.agc_val * AGC_ATTACK * agc_var;	// Fast attack to increase attenuation (do NOT scale w/decimation or else oscillation results)
				if(ads.agc_val <= AGC_VAL_MIN)	// Prevent zero or "negative" gain values
					ads.agc_val = AGC_VAL_MIN;
			}
			if(ads.agc_val >= ads.agc_rf_gain)	{	// limit AGC to reasonable values when low/no signals present
				ads.agc_val = ads.agc_rf_gain;
				if(ads.agc_val >= ads.agc_val_max)	// limit maximum gain under no-signal conditions
					ads.agc_val = ads.agc_val_max;
			}
		}
		else	// AGC Off - manual AGC gain
			ads.agc_val = ads.agc_rf_gain;			// use logarithmic gain value in RF gain control
		//
		ads.agc_valbuf[i] = ads.agc_val;			// store in "running" AGC history buffer for later application to audio data
	}
	//
	// Delay the post-AGC audio slightly so that the AGC's "attack" will very slightly lead the audio being acted upon by the AGC.
	// This eliminates a "click" that can occur when a very strong signal appears due to the AGC lag.  The delay is adjusted based on
	// decimation rate so that it is constant for all settings.
	//
	arm_copy_f32((float32_t *)ads.a_buffer, (float32_t *)&agc_delay[agc_delay_inbuf], psize/2);	// put new data into the delay buffer
	arm_copy_f32((float32_t *)&agc_delay[agc_delay_outbuf], (float32_t *)ads.a_buffer, psize/2);	// take old data out of the delay buffer
	// Update the in/out pointers to the AGC delay buffer
	agc_delay_inbuf += psize/2;						// update circular de-correlation delay buffer
	agc_delay_outbuf = agc_delay_inbuf + psize/2;
	agc_delay_inbuf %= ads.agc_delay_buflen;
	agc_delay_outbuf %= ads.agc_delay_buflen;
	//
	//
	// Now apply pre-calculated AGC values to delayed audio
	//
	arm_mult_f32((float32_t *)ads.a_buffer, (float32_t *)ads.agc_valbuf, (float32_t *)ads.a_buffer, psize/2);		// do vector multiplication to apply delayed "running" AGC data
	//
}


//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_freq_conv
//* Object              : Does I/Q frequency conversion
//* Object              :
//* Input Parameters    : size of array on which to work; dir: determines direction of shift - see below;  Also uses variables in ads structure
//* Output Parameters   : uses variables in ads structure
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_freq_conv(int16_t size, int16_t dir)
{
	ulong 		i;
	float32_t	rad_calc;
	static bool flag = 0;

	// Below is the frequency translation code that uses a "pre-calculated" sine wave - which means that the translation must be done at a sub-
	// multiple of the sample frequency.  This pre-calculation eliminates the processor overhead required to generate a sine wave on the fly.
	// This also makes extensive use of the optimized ARM vector instructions for the calculation of the final I/Q vectors
	//
	// Pre-calculate 6 kHz quadrature sine wave ONCE for the conversion
	//

		uchar multi = 1;

		if((ts.iq_freq_mode == 1 || ts.iq_freq_mode == 2) && multi != 4)
		    {
		    multi = 4; 		//(4 = 6 kHz offset)
		    flag = 0;
		    }
		if((ts.iq_freq_mode == 3 || ts.iq_freq_mode == 4) && multi != 8)
		    {
		    multi = 8; 		// (8 = 12 kHz offset)
		    flag = 0;
		    }

	if(!flag)	{		// have we already calculated the sine wave?
		for(i = 0; i < size/2; i++)	{		// No, let's do it!
			rad_calc = (float32_t)i;		// convert to float the current position within the buffer
			rad_calc /= (size/2);			// make this a fraction
			rad_calc *= (PI * 2);			// convert to radians
			rad_calc *= multi;				// multiply by number of cycles that we want within this block (4 = 6 kHz)
			//
			osc_q[i] = arm_cos_f32(rad_calc);	// get sine and cosine values and store in pre-calculated array
			osc_i[i] = arm_sin_f32(rad_calc);
		}
		flag = 1;	// signal that once we have generated the quadrature sine waves, we should not do it again
	}
	//
	// Do frequency conversion
	//
	if(!dir)	{	// Conversion is "above" on RX (LO needs to be set lower)
		arm_mult_f32(ads.i_buffer, osc_q, ads.a_buffer, size/2);	// multiply products for converted I channel
		arm_mult_f32(ads.q_buffer, osc_i, ads.b_buffer, size/2);
		//
		arm_mult_f32(ads.q_buffer, osc_q, ads.c_buffer, size/2);	// multiply products for converted Q channel
		arm_mult_f32(ads.i_buffer, osc_i, ads.d_buffer, size/2);
		//
		arm_add_f32(ads.a_buffer, ads.b_buffer, ads.i_buffer, size/2);	// summation for I channel
		arm_sub_f32(ads.c_buffer, ads.d_buffer, ads.q_buffer, size/2);	// difference for Q channel
	}
	else	{	// Conversion is "below" on RX (LO needs to be set higher)
		arm_mult_f32(ads.q_buffer, osc_q, ads.a_buffer, size/2);	// multiply products for converted I channel
		arm_mult_f32(ads.i_buffer, osc_i, ads.b_buffer, size/2);
		//
		arm_mult_f32(ads.i_buffer, osc_q, ads.c_buffer, size/2);	// multiply products for converted Q channel
		arm_mult_f32(ads.q_buffer, osc_i, ads.d_buffer, size/2);
		//
		arm_add_f32(ads.a_buffer, ads.b_buffer, ads.q_buffer, size/2);	// summation for I channel
		arm_sub_f32(ads.c_buffer, ads.d_buffer, ads.i_buffer, size/2);	// difference for Q channel
	}
}

//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_demod_am  (rewritten to use optimized ARM complex magnitude function, October 2015 - KA7OEI)
//* Object              : AM demodulator
//* Object              :
//* Input Parameters    : size - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_demod_am(int16_t size)
{
    ulong i;
    float32_t sqrt;
        //
        // uses optimized ARM sqrt function, but not the arm_cmplx_mag, because the latter needs the data in interleaved format!
        // this could possibly make this even faster than first interleaving and then calculating magnitude
    	// (because arm_cmplx_mag uses the same sqrt function )

    for(i = 0; i < size/2; i++) {
    	 arm_sqrt_f32 (ads.i_buffer[i] * ads.i_buffer[i] + ads.q_buffer[i] * ads.q_buffer[i], &sqrt);
    	 ads.a_buffer[i] = sqrt;
    }

    //
    // Now produce signal/carrier level for AGC
    //
    arm_mean_f32(ads.a_buffer, size/2, (float32_t *)&ads.am_agc);	// get "average" value of "a" buffer - the recovered DC (carrier) value - for the AGC (always positive since value was squared!)
    //
    ads.am_agc *= AM_SCALING;	// rescale AM AGC to match SSB scaling so that AGC comes out the same
}

//
//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_lms_notch_filter  [KA7OEI October, 2015]
//* Object              :
//* Object              : automatic notch filter
//* Input Parameters    : psize - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_lms_notch_filter(int16_t psize)
{
	static ulong		lms2_inbuf = 0;
	static ulong		lms2_outbuf = 0;

	// DSP Automatic Notch Filter using LMS (Least Mean Squared) algorithm
	//
	arm_copy_f32((float32_t *)ads.a_buffer, (float32_t *)&lms2_nr_delay[lms2_inbuf], psize/2);	// put new data into the delay buffer
	//
	arm_lms_norm_f32(&lms2Norm_instance, (float32_t *)ads.a_buffer, (float32_t *)&lms2_nr_delay[lms2_outbuf], (float32_t *)errsig2, (float32_t *)ads.a_buffer, psize/2);	// do automatic notch
	// Desired (notched) audio comes from the "error" term - "errsig2" is used to hold the discarded ("non-error") audio data
	//
	lms2_inbuf += psize/2;				// update circular de-correlation delay buffer
	lms2_outbuf = lms2_inbuf + psize/2;
	lms2_inbuf %= ts.dsp_notch_delaybuf_len;
	lms2_outbuf %= ts.dsp_notch_delaybuf_len;
	//
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_lms_noise_reduction  [KA7OEI October, 2015]
//* Object              :
//* Object              : DSP noise reduction using LMS algorithm
//* Input Parameters    : psize - size of buffer on which to operate
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_lms_noise_reduction(int16_t psize)
{
	static ulong		lms1_inbuf = 0, lms1_outbuf = 0;

	arm_copy_f32((float32_t *)ads.a_buffer, (float32_t *)&lms1_nr_delay[lms1_inbuf], psize/2);	// put new data into the delay buffer
	//
	arm_lms_norm_f32(&lms1Norm_instance, (float32_t *)ads.a_buffer, (float32_t *)&lms1_nr_delay[lms1_outbuf], (float32_t *)ads.a_buffer, (float32_t *)errsig1 ,psize/2);	// do noise reduction
	//
	// Detect if the DSP output has gone to (near) zero output - a sign of it crashing!
	//
	if((((ulong)fabs(ads.a_buffer[0])) * DSP_ZERO_DET_MULT_FACTOR) < DSP_OUTPUT_MINVAL)	{	// is DSP level too low?
		// For some stupid reason we can't just compare above to a small fractional value  (e.g. "x < 0.001") so we must multiply it first!
		if(ads.dsp_zero_count < MAX_DSP_ZERO_COUNT)	{
			ads.dsp_zero_count++;
		}
	}
	else
		ads.dsp_zero_count = 0;
	//
	ads.dsp_nr_sample = ads.a_buffer[0];		// provide a sample of the DSP output for crash detection
	//
	lms1_inbuf += psize/2;	// bump input to the next location in our de-correlation buffer
	lms1_outbuf = lms1_inbuf + psize/2;	// advance output to same distance ahead of input
	lms1_inbuf %= ts.dsp_nr_delaybuf_len;
	lms1_outbuf %= ts.dsp_nr_delaybuf_len;
}


//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_processor
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_processor(int16_t *src, int16_t *dst, int16_t size)
{

	ulong 				i;
	float32_t			post_agc_gain_scaling=1;
	//
	int16_t				psize;		// processing size, with decimation
	//

	psize = size/(int16_t)ads.decimation_rate;	// rescale sample size inside decimated portion based on decimation factor

	audio_rx_noise_blanker(src, size);		// do noise blanker function

	for(i = 0; i < size/2; i++)
	{
		//
		// Collect I/Q samples
		if(sd.state == 0)
		{
			sd.FFT_Samples[sd.samp_ptr] = *(src + 1);
			sd.samp_ptr++;
			sd.FFT_Samples[sd.samp_ptr] = *(src);
			sd.samp_ptr++;

			// On overload, update state machine,
			// reset pointer and wait
			if(sd.samp_ptr >= FFT_IQ_BUFF_LEN*2)
			{
				sd.samp_ptr = 0;
				sd.state    = 1;
			}
		}
		//
		if(*src > ADC_CLIP_WARN_THRESHOLD/4)	{		// This is the release threshold for the auto RF gain
			ads.adc_quarter_clip = 1;
			if(*src > ADC_CLIP_WARN_THRESHOLD/2)	{		// This is the trigger threshold for the auto RF gain
					ads.adc_half_clip = 1;
					if(*src > ADC_CLIP_WARN_THRESHOLD)			// This is the threshold for the red clip indicator on S-meter
						ads.adc_clip = 1;
			}
		}
		//
		// 16 bit format - convert to float and increment
		ads.i_buffer[i] = (float32_t)*src++;
		ads.q_buffer[i] = (float32_t)*src++;
		//
	}

	//
	// Apply gain corrections for I/Q gain balancing
	//
	arm_scale_f32((float32_t *)ads.i_buffer, (float32_t)ts.rx_adj_gain_var_i, (float32_t *)ads.i_buffer, size/2);
	//
	arm_scale_f32((float32_t *)ads.q_buffer, (float32_t)ts.rx_adj_gain_var_q, (float32_t *)ads.q_buffer, size/2);
	//
	//
	if(ts.iq_freq_mode)	{		// is receive frequency conversion to be done?
		if(ts.iq_freq_mode == 1 || ts.iq_freq_mode == 3)				// Yes - "RX LO LOW" mode
			audio_rx_freq_conv(size, 1);
		else								// it is in "RX LO LOW" mode
			audio_rx_freq_conv(size, 0);
	}

	// shift 0 degrees FIR
	arm_fir_f32(&FIR_I,(float32_t *)(ads.i_buffer),(float32_t *)(ads.i_buffer),size/2);
	// shift +90 degrees FIR
	arm_fir_f32(&FIR_Q,(float32_t *)(ads.q_buffer),(float32_t *)(ads.q_buffer),size/2);

	switch(ts.dmod_mode)	{
		case DEMOD_LSB:
			arm_sub_f32((float32_t *)ads.i_buffer, (float32_t *)ads.q_buffer, (float32_t *)ads.a_buffer, size/2);	// difference of I and Q - LSB
			break;
		case DEMOD_CW:
			if(!ts.cw_lsb)	// is this USB RX mode?  (LSB of mode byte was zero)
				arm_add_f32((float32_t *)ads.i_buffer, (float32_t *)ads.q_buffer, (float32_t *)ads.a_buffer, size/2);	// sum of I and Q - USB
			else	// No, it is LSB RX mode
				arm_sub_f32((float32_t *)ads.i_buffer, (float32_t *)ads.q_buffer, (float32_t *)ads.a_buffer, size/2);	// difference of I and Q - LSB
			break;
		case DEMOD_AM:
			audio_demod_am(size);
			break;
		case DEMOD_USB:
		case DEMOD_DIGI:
		default:
			arm_add_f32((float32_t *)ads.i_buffer, (float32_t *)ads.q_buffer, (float32_t *)ads.a_buffer, size/2);	// sum of I and Q - USB
			break;
	}

	//
	// Do decimation down to lower rate for heavy-duty processing to reduce processor load
	//
	arm_fir_decimate_f32(&DECIMATE_RX, ads.a_buffer, ads.a_buffer, size/2);		// LPF built into decimation (Yes, you can decimate-in-place!)

	//
	// DSP Automatic Notch Filter using LMS (Least Mean Squared) algorithm
	//
	if((ts.dsp_active & 4) && (!ads.af_dissabled))	{	// No notch in CW mode
		audio_lms_notch_filter(psize);		// Do notch filter
	}

	//
	// DSP noise reduction using LMS (Least Mean Squared) algorithm
	// This is the pre-filter/AGC instance
	//
	if((ts.dsp_active & 1) && (!(ts.dsp_active & 2)) && (!ads.af_dissabled))	{	// Do this if enabled and "Pre-AGC" DSP NR enabled
		audio_lms_noise_reduction(psize);
	}

	if((!ads.af_dissabled)	&& (ts.filter_id != AUDIO_WIDE))	{	// we don't need to filter if running in 10 kHz mode (Hilbert filter does the job!)
		// IIR ARMA-type lattice filter
		arm_iir_lattice_f32(&IIR_PreFilter, (float *)ads.a_buffer, (float *)ads.a_buffer, psize/2);
	}
	//
	// now process the samples and perform the receiver AGC function
	//
	audio_rx_agc_processor(psize);

	//
	// DSP noise reduction using LMS (Least Mean Squared) algorithm
	// This is the post-filter, post-AGC instance
	//
	if((ts.dsp_active & 1) && (ts.dsp_active & 2) && (!ads.af_dissabled))	{	// Do DSP NR if enabled and if post-DSP NR enabled
		audio_lms_noise_reduction(psize);
	}

	if(ts.filter_id != AUDIO_WIDE)
		post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_4;
	else
		post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_2;

	if(ts.dmod_mode == DEMOD_AM)
			arm_scale_f32(ads.a_buffer,(float32_t)(ads.post_agc_gain * post_agc_gain_scaling * (AM_SCALING * AM_AUDIO_SCALING)), ads.a_buffer, psize/2);	// apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
		else
			arm_scale_f32(ads.a_buffer,(float32_t)(ads.post_agc_gain * post_agc_gain_scaling), ads.a_buffer, psize/2);	// apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
	//
	// resample back to original sample rate while doing low-pass filtering to minimize aliasing effects
	arm_fir_interpolate_f32(&INTERPOLATE_RX, ads.a_buffer, ads.b_buffer, psize/2);
	//
	//

	if(ts.rx_muting)	{	// fill audio buffers with zeroes if we are to mute the receiver completely while still processing data OR it is in FM and squelched
		arm_fill_f32(0, (float32_t *)ads.a_buffer, size/2);
		arm_fill_f32(0, (float32_t *)ads.b_buffer, size/2);
	}

	arm_scale_f32(ads.b_buffer, LINE_OUT_SCALING_FACTOR, ads.a_buffer, size/2);		// Do fixed scaling of audio for LINE OUT and copy to "a" buffer
	//
	// AF gain in "ts.audio_gain-active"
	//  0 - 16: via codec command
	// 17 - 20: soft gain after decoder
	//

	//
	// Transfer processed audio to DMA buffer
	//
	i = 0;			// init sample transfer counter
	while(i < size/2)	{						// transfer to DMA buffer and do conversion to INT - Unrolled to speed it up
		*dst++ = (int16_t)ads.a_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)

		*dst++ = (int16_t)ads.a_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)

		*dst++ = (int16_t)ads.a_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)

		*dst++ = (int16_t)ads.a_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)
	}

	ads.pre_filter_gain = 1;
}

/**
  * @brief  Split interleaved stereo into two separate buffers
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  src - pointer to source buffer
  * @param  ldst - pointer to left dest buffer (even samples)
  * @param  rdst - pointer to right dest buffer (odd samples)
  * @retval none
  */
static void audio_split_stereo(int16_t size, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(size)
	{

		if(sd.state == 0)
		{

			sd.FFT_Samples[sd.samp_ptr] = (float32_t)(*(src + 1));	// get float32_ting point data for FFT for spectrum scope/waterfall display
			sd.samp_ptr++;
			sd.FFT_Samples[sd.samp_ptr] = (float32_t)(*(src));
			sd.samp_ptr++;

			if(sd.samp_ptr == FFT_IQ_BUFF_LEN)
			{
				sd.samp_ptr = 0;
				sd.state    = 1;
			}
		}


		*ldst++ = *src++;
		size--;
		*rdst++ = *src++;
		size--;
	}
}

/**
  * @brief  combine two separate buffers into interleaved stereo
  * @param  size -  samples per output buffer (divisible by 2)
  * @param  dst - pointer to source buffer
  * @param  lsrc - pointer to left dest buffer (even samples)
  * @param  rsrc - pointer to right dest buffer (odd samples)
  * @retval none
  */
static void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = *rsrc++;
		sz--;
	}
}

/**
  * @brief  This function handles I2S RX buffer processing. 
  * @param  src - pointer to source buffer
  * @param  dst - pointer to dest buffer
  * @param  size -  samples per buffer
  * @retval none
  */

#ifdef USE_24_BITS
void I2S_RX_CallBack(int32_t *src, int32_t *dst, int16_t size, uint16_t ht)
#else
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t size, uint16_t ht)
#endif
{

	//static uchar lcd_dim = 0, lcd_dim_prescale = 0;

	audio_rx_processor(src, dst,size);

	/*

	if(!lcd_dim_prescale)	{	// Only update dimming PWM counter every fourth time through to reduce frequency below that of audible range
		if(lcd_dim < ts.lcd_backlight_brightness)
			LCD_BACKLIGHT_PIO->BSRRH = LCD_BACKLIGHT;	// LCD backlight off
		else
			LCD_BACKLIGHT_PIO->BSRRL = LCD_BACKLIGHT;	// LCD backlight on
		//
		lcd_dim++;
		lcd_dim &= 3;	// limit brightness PWM count to 0-3
	}
	lcd_dim_prescale++;
	lcd_dim_prescale &= 3;	// limit prescale count to 0-3
	*/

	/* Split Stereo */
	//audio_split_stereo(size, src, left_buffer, right_buffer);

	/* Combine stereo */
	//audio_comb_stereo(size, dst, left_buffer, right_buffer);

}
