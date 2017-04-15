#include "radio.h"
#include "tm_stm32f4_ili9341_button.h"
#include "ui_ra8875_fonts.h"
#include "boton.h"

// Radio driver public
__IO TransceiverState ts;

// ------------------------------------------------
// Frequency public
__IO DialFrequency df;

// Spectrum display public
extern __IO SpectrumDisplay sd;

// Audio driver publics
extern __IO AudioDriverState ads;

// S meter public
extern __IO SMeter sm;

__IO FilterCoeffs fc;

extern __IO Si5351Variables vfo;

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

extern sFONT GL_Font16x24;

char temp[30];
uchar fingers;
uint16_t touch_skip = 0;
uchar modesw_state = 0;
int8_t buttonPressed;

/* Buttons handler */
void BUTTON1_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON2_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON3_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON4_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON5_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON6_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON7_EventHandler(TM_BUTTON_PressType_t type);
void BUTTON8_EventHandler(TM_BUTTON_PressType_t type);

void radio_init(void);
void InitBoard(void);
static void mchf_board_adc1_init(void);
static void ui_buttons(void);
void ui_touch(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 2000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


int main(int argc, char* argv[]) {

	//Initialize system

	SystemInit();

	//Init on board GPIO, LEDS and LPF Softrock filter control pins.
	// Si la desactivas falla la pantalla.

	InitBoard();

	while (1) {

		TM_BUTTON_Update();					// Actualiza botones
		UiDriverUpdateFrequency(0);			// Lee encoder

		//UiDriverReDrawSpectrumDisplay();// Spectrum Display enabled - do that!

		UiDriverReDrawWaterfallDisplay();// yes - call waterfall update instead

		UiDriverHandleSmeter();				// S-meter

		//UiDriverHandlePowerSupply();

		UiDriverCheckEncoderTwo();

		ui_touch();

		ts.rx_muting = 0;

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitBoard(void) {

	// Enable clock on all ports
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	LCD_InitLCD();
	LCD_PrepareDMA();

	// touch GSL1680 code

	LCD_PrintText(450, 0, "GSL1680 ..", Grey, Black, 0);

	if (gsl1680_init_chip())
		LCD_PrintText(450, 0, "GSL1680 OK", Green, Black, 0);
	else
		LCD_PrintText(450, 0, "GSL1680 ERROR", Red, Black, 0);

	if (TM_SDRAM_Init())
		LCD_PrintText(450, 15, "SDRAM OK - 8 Mbytes", Green, Black, 0);
	else
		LCD_PrintText(450, 15, "SDRAM ERROR", Red, Black, 0);

	// Inicia los botones

	ui_buttons();

	// For Discovery boards
	// GPIOA, pin 0, high (1) when pressed
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_3, 0, BUTTON1_EventHandler);
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_4, 0, BUTTON2_EventHandler);
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_10, 0, BUTTON3_EventHandler);
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_11, 0, BUTTON4_EventHandler);
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_12, 0, BUTTON5_EventHandler);
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_13, 0, BUTTON6_EventHandler);
	TM_BUTTON_Init(GPIOE, GPIO_PIN_0, 0, BUTTON7_EventHandler);
//	TM_BUTTON_Init(GPIOE, GPIO_PIN_15, 0, BUTTON8_EventHandler);

	//ui_si570_calc_startupfrequency();
	//UiLcdHy28_ShowStartUpScreen(100);

//	Si5351_init(SI5351_CRYSTAL_LOAD_8PF, 27000000);
//	Si5351_set_correction(6650);
//	vfo.ref_correction=6650;
//	Si5351_set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
//	Si5351_set_freq(14000000ULL/5, SI5351_PLL_FIXED, SI5351_CLK0);
//	Si5351_set_freq(20000000ULL, 0ULL, SI5351_CLK1);

	// Initialize delay
	TM_DELAY_Init();

	delay();	// needed to allow codec to settle?

	Codec_Init(I2S_AudioFreq_48k);

	delay();	// needed to allow codec to settle?

	I2S_Block_Init();

	delay();	// needed to allow codec to settle?

	radio_init();
	Audio_Init();

	delay();	// needed to allow codec to settle?

	UiCalcRxIqGainAdj();
	UiCalcRxPhaseAdj();		// Fundamental para iniciar las FIR de Hilberts

	//mchf_board_adc1_init();

	ui_driver_init();

	delay();	// needed to allow codec to settle?

	I2S_Block_Process((uint32_t) &tx_buffer, (uint32_t) &rx_buffer, BUFF_LEN);// Arranca el proceso por DMA

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON1_EventHandler(TM_BUTTON_PressType_t type) {

	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		Codec_Line_Gain_Adj(0);

		if (ts.band_mode > (MIN_BANDS - 1))
			UiDriverChangeBand(0);

		UiDriverUpdateFrequency(1);
		sd.dial_moved = 1;
		UiDriverDisplayFilterBW();

	} else {

		// Ajuste de brillo del LCD

//		if (ts.lcd_backlight_brightness<3)
//			ts.lcd_backlight_brightness++;
//		else ts.lcd_backlight_brightness=0;

		if ((!(ts.dsp_active & 1)) && (!(ts.dsp_active & 4)))// both NR and notch are inactive
			ts.dsp_active |= 1;									// turn on NR
		else if ((ts.dsp_active & 1) && (!(ts.dsp_active & 4))) {// NR active, notch inactive
			if (ts.dmod_mode != DEMOD_CW) {	// NOT in CW mode
				ts.dsp_active |= 4;								// turn on notch
				ts.dsp_active &= 0xfe;							// turn off NR
			} else {// CW mode - do not select notches, skip directly to "off"
				ts.dsp_active &= 0xfa;	// turn off NR and notch
			}
		} else if ((!(ts.dsp_active & 1)) && (ts.dsp_active & 4))//	NR inactive, notch active
			if ((ts.dmod_mode == DEMOD_AM) && (ts.filter_id == AUDIO_WIDE))	// was it AM with a 10 kHz filter selected?

				ts.dsp_active &= 0xfa;// it was AM + 10 kHz - turn off NR and notch
			else
				ts.dsp_active |= 1;				// no - turn on NR
		//
		else {
			ts.dsp_active &= 0xfa;						// turn off NR and notch
		}
		//
		ts.dsp_active_toggle = ts.dsp_active;// save update in "toggle" variable
		//
		ts.reset_dsp_nr = 1;				// reset DSP NR coefficients
		audio_driver_set_rx_audio_filter();	// update DSP settings
		ts.reset_dsp_nr = 0;
		UiDriverChangeDSPMode();			// update on-screen display

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON2_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		Codec_Line_Gain_Adj(0);
		if (ts.band_mode < (MAX_BANDS - 1))
			UiDriverChangeBand(1);

		UiDriverUpdateFrequency(1);
		sd.dial_moved = 1;
		UiDriverDisplayFilterBW();

	} else {

		ts.filter_id++;
		//
		if (ts.filter_id >= AUDIO_MAX_FILTER)
			ts.filter_id = AUDIO_MIN_FILTER;

		UiDriverChangeFilter(0);
		UiCalcRxPhaseAdj();		// set gain and phase values according to mode
		UiCalcRxIqGainAdj();
		UiDriverChangeDSPMode();	// Change DSP display setting as well

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON3_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		if (ts.menu_mode) {

			ts.menu_item++;
			if (ts.menu_item > 13)
				ts.menu_item = 0;

			switch (ts.menu_item) {
			case 0:
				LCD_PrintText(0, 12, "RF Gain    ", White, Black, 0);
				break;

			case 1:
				LCD_PrintText(0, 12, "DSP Noise R", White, Black, 0);
				break;

			case 2:
				LCD_PrintText(0, 12, "Audio Gain ", White, Black, 0);
				break;

			case 3:
				LCD_PrintText(0, 12, "Noise Blank", White, Black, 0);
				break;

			case 4:
				LCD_PrintText(0, 12, "Codec Gain ", White, Black, 0);
				break;

			case 5:
				LCD_PrintText(0, 12, "Scope Noise", White, Black, 0);
				break;

			case 6:
				LCD_PrintText(0, 12, "SSB 2k3 Fil", White, Black, 0);
				break;

			case 7:
				LCD_PrintText(0, 12, "AM WIDE Fil", White, Black, 0);
				break;

			case 8:
				LCD_PrintText(0, 12, "AGC Mode   ", White, Black, 0);
				break;

			case 9:
				LCD_PrintText(0, 12, "FFT Window ", White, Black, 0);
				break;

			case 10:
				LCD_PrintText(0, 12, "FFT Gain   ", White, Black, 0);
				break;

			case 11:
				LCD_PrintText(0, 12, "DSP Enable ", White, Black, 0);
				break;

			case 12:
				LCD_PrintText(0, 12, "Spect. Filt", White, Black, 0);
				break;

			case 13:
				LCD_PrintText(0, 12, "Spectrum En", White, Black, 0);
				break;

			default:
				LCD_PrintText(0, 12, "VOID     ", White, Black, 0);
				break;
			}
		} else {
			ts.dmod_mode++;

			if (ts.dmod_mode > DEMOD_MAX_MODE - 1)
				ts.dmod_mode = DEMOD_USB;

			UiDriverShowMode();
			UiCalcRxPhaseAdj();	// set gain and phase values according to mode
			UiCalcRxIqGainAdj();
			UiDriverDisplayFilterBW();
		}

	} else {

//		sd.magnify^=1;
//		UiDrawSpectrumScopeFrequencyBarText();
//		UiDriverDisplayFilterBW();

		ts.menu_mode ^= 1;

		if (ts.menu_mode)
			LCD_PrintText(0, 0, "MENU", Yellow, Black, 0);
		else {
			LCD_PrintText(0, 0, "      ", White, Black, 0);
			LCD_PrintText(0, 12, "            ", White, Black, 0);
		}

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON4_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		UiDriverChangeTunningStep(1);
		UiDriverShowStep(df.tuning_step);

	} else {

		// pulsacion larga

		UiDriverChangeTunningStep(0);
		UiDriverShowStep(df.tuning_step);

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON5_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		if (ts.menu_mode) {

			ts.menu_item++;
			if (ts.menu_item > 11)
				ts.menu_item = 0;

			switch (ts.menu_item) {
			case 0:
				LCD_PrintText(0, 12, "RF Gain    ", White, Black, 0);
				break;

			case 1:
				LCD_PrintText(0, 12, "DSP Noise R", White, Black, 0);
				break;

			case 2:
				LCD_PrintText(0, 12, "Audio Gain ", White, Black, 0);
				break;

			case 3:
				LCD_PrintText(0, 12, "Noise Blank", White, Black, 0);
				break;

			case 4:
				LCD_PrintText(0, 12, "Codec Gain ", White, Black, 0);
				break;

			case 5:
				LCD_PrintText(0, 12, "Scope Noise", White, Black, 0);
				break;

			case 6:
				LCD_PrintText(0, 12, "SSB 2k3 Flt", White, Black, 0);
				break;

			case 7:
				LCD_PrintText(0, 12, "AM WIDE Flt", White, Black, 0);
				break;

			case 8:
				LCD_PrintText(0, 12, "AGC Mode   ", White, Black, 0);
				break;

			case 9:
				LCD_PrintText(0, 12, "FFT Window ", White, Black, 0);
				break;

			case 10:
				LCD_PrintText(0, 12, "FFT Gain   ", White, Black, 0);
				break;

			case 11:
				LCD_PrintText(0, 12, "DSP Enable ", White, Black, 0);
				break;

			default:
				LCD_PrintText(0, 12, "VOID     ", White, Black, 0);
				break;
			}
		}

	} else {

		ts.menu_mode ^= 1;

		if (ts.menu_mode)
			LCD_PrintText(0, 0, "MENU", Green, Black, 0);
		else {
			LCD_PrintText(0, 0, "    ", White, Black, 0);
			LCD_PrintText(0, 12, "         ", White, Black, 0);
		}

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON6_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		ts.filter_id++;
		//
		if (ts.filter_id >= AUDIO_MAX_FILTER)
			ts.filter_id = AUDIO_MIN_FILTER;

		UiDriverChangeFilter(0);
		UiCalcRxPhaseAdj();		// set gain and phase values according to mode
		UiCalcRxIqGainAdj();
		UiDriverChangeDSPMode();	// Change DSP display setting as well

	} else {

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON7_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		GPIO_ToggleBits(GPIOC, GPIO_Pin_12);

		ts.dmod_mode++;

		if (ts.dmod_mode > DEMOD_MAX_MODE - 1)
			ts.dmod_mode = DEMOD_USB;

		UiDriverShowMode();
		UiCalcRxPhaseAdj();		// set gain and phase values according to mode
		UiCalcRxIqGainAdj();
		UiDriverDisplayFilterBW();

	} else {

		if (ts.misc_flags1 & 128)
			ts.misc_flags1 &= 127;
		else
			ts.misc_flags1 |= 128;

		UiInitSpectrumScopeWaterfall();

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BUTTON8_EventHandler(TM_BUTTON_PressType_t type) {
	/* Check button */
	if (type == TM_BUTTON_PressType_OnPressed) {
		//TM_USART_Puts(USART2, "Button 1 onPressed\n");
	} else if (type == TM_BUTTON_PressType_Normal) {

		ts.dmod_mode++;

		if (ts.dmod_mode > DEMOD_MAX_MODE - 1)
			ts.dmod_mode = DEMOD_USB;

		UiDriverShowMode();
		UiCalcRxPhaseAdj();		// set gain and phase values according to mode
		UiCalcRxIqGainAdj();
		UiDriverDisplayFilterBW();

	} else {

		if ((!(ts.dsp_active & 1)) && (!(ts.dsp_active & 4)))// both NR and notch are inactive
			ts.dsp_active |= 1;									// turn on NR
		else if ((ts.dsp_active & 1) && (!(ts.dsp_active & 4))) {// NR active, notch inactive
			if (ts.dmod_mode != DEMOD_CW) {	// NOT in CW mode
				ts.dsp_active |= 4;								// turn on notch
				ts.dsp_active &= 0xfe;							// turn off NR
			} else {// CW mode - do not select notches, skip directly to "off"
				ts.dsp_active &= 0xfa;	// turn off NR and notch
			}
		} else if ((!(ts.dsp_active & 1)) && (ts.dsp_active & 4))//	NR inactive, notch active
			if ((ts.dmod_mode == DEMOD_AM) && (ts.filter_id == AUDIO_WIDE))	// was it AM with a 10 kHz filter selected?

				ts.dsp_active &= 0xfa;// it was AM + 10 kHz - turn off NR and notch
			else
				ts.dsp_active |= 1;				// no - turn on NR
		//
		else {
			ts.dsp_active &= 0xfa;						// turn off NR and notch
		}
		//
		ts.dsp_active_toggle = ts.dsp_active;// save update in "toggle" variable
		//
		ts.reset_dsp_nr = 1;				// reset DSP NR coefficients
		audio_driver_set_rx_audio_filter();	// update DSP settings
		ts.reset_dsp_nr = 0;
		UiDriverChangeDSPMode();			// update on-screen display

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void radio_init(void) {

	sd.enabled = 1;

	ts.rx_muting = 0;

	ts.lcd_backlight_brightness = 3;

	ts.iq_freq_mode = FREQ_IQ_CONV_M12KHZ;
	ts.spectrum_scope_nosig_adjust = 40;

	ts.agc_mode = AGC_FAST;

	ts.dmod_mode = DEMOD_LSB;
	ts.filter_id = AUDIO_2P3KHZ;

	ts.filter_band = 0;

	ts.rf_gain = 50;

	df.tuning_step = 1000;

	ts.freq_step_config = 0;
	ts.tune_step = 0;

	ts.tune_freq = 0;
	ts.tune_freq_old = 0;
	ts.freq_cal = 0;

	df.transv_freq = 0;

	ts.rf_codec_gain = 9;

	//

	ts.scope_trace_colour = SPEC_COLOUR_TRACE_DEFAULT;// default colour for the spectrum scope trace
	ts.scope_grid_colour = SPEC_COLOUR_GRID_DEFAULT;// default colour for the spectrum scope grid
	ts.scope_grid_colour_active = Grid;
	ts.scope_centre_grid_colour = SPEC_COLOUR_GRID_DEFAULT;	// color of center line of scope grid
	ts.scope_centre_grid_colour_active = Grid;

	ts.band = BAND_MODE_20;
	ts.band_mode = BAND_MODE_20;
	ts.dmod_mode = DEMOD_USB;

	ts.scope_agc_rate = 25;		// load default spectrum scope AGC rate

	ts.filter_300Hz_select = FILTER_300HZ_DEFAULT;
	ts.filter_500Hz_select = FILTER_500HZ_DEFAULT;
	ts.filter_1k8_select = FILTER_1K8_DEFAULT;
	ts.filter_2k3_select = FILTER_2K3_DEFAULT;
	ts.filter_3k6_select = FILTER_3K6_DEFAULT;
	ts.filter_wide_select = WIDE_FILTER_6K_AM;

	ts.dsp_active = 0;			// TRUE if DSP noise reduction is to be enabled
	ts.dsp_active_toggle = 0xff;// used to hold the button G2 "toggle" setting.
	ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
	ts.dsp_nr_strength = 10;	// "Strength" of DSP noise reduction (0 = weak)
	ts.dsp_notch_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
	ts.dsp_inhibit = 1;	// TRUE if DSP is to be inhibited - power up with DSP disabled
	ts.dsp_timed_mute = 0;		// TRUE if DSP is to be muted for a timed amount
	ts.dsp_inhibit_timing = 0;// used to time inhibiting of DSP when it must be turned off for some reason
	ts.reset_dsp_nr = 0;// TRUE if DSP NR coefficients are to be reset when "audio_driver_set_rx_audio_filter()" is called
	ts.dsp_notch_mu = DSP_NOTCH_MU_DEFAULT;

	ts.nb_agc_time_const = NB_AGC_DEFAULT;
	ts.nb_setting = 0;
	ts.nb_disable = 0;

	ts.scope_agc_rate = SPECTRUM_SCOPE_AGC_DEFAULT;

	ts.cw_lsb = 0;

	ts.rx_iq_am_gain_balance = 0;
	ts.rx_iq_usb_gain_balance = 0;
	ts.rx_iq_lsb_gain_balance = 0;

	ts.rx_iq_usb_phase_balance = 0;
	ts.rx_iq_lsb_phase_balance = 0;

	ts.waterfall_size = WATERFALL_NORMAL;

	ts.misc_flags1 = 0;

	ts.waterfall_contrast = WATERFALL_CONTRAST_DEFAULT;
	ts.waterfall_vert_step_size = WATERFALL_STEP_SIZE_DEFAULT;

	ts.audio_gain = 30;

	ts.menu_mode = 0;

	ts.audio_max_volume = MAX_VOLUME_MIN;

	ts.fft_window_type = FFT_WINDOW_DEFAULT;

	ts.scope_trace_colour = SPEC_COLOUR_TRACE_DEFAULT;

	ts.voltmeter_calibrate = POWER_VOLTMETER_CALIBRATE_DEFAULT;

	ts.filter_disp_colour = SPEC_RED;

	sd.first_run = 3;

	ts.spectrum_filter=SPECTRUM_FILTER_DEFAULT;

}

static void mchf_board_adc1_init(void) {

	// pin 6
#define ADC1_PWR			GPIO_Pin_6
#define ADC1_PWR_SOURCE		GPIO_PinSource6
#define ADC1_PWR_PIO       	GPIOA

	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable ADC3 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// Configure ADC Channel 6 as analog input
	GPIO_InitStructure.GPIO_Pin = ADC1_PWR;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC1_PWR_PIO, &GPIO_InitStructure);

	// Common Init
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// Configuration
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	// Regular Channel Config
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_3Cycles);

	// Enable
	ADC_Cmd(ADC1, ENABLE);

	// ADC2 regular Software Start Conv
	ADC_SoftwareStartConv(ADC1);
}

void ui_touch() {

	touch_skip++;

	if (GPIO_ReadInputDataBit(GSL1680_INT_PORT,
	GSL1680_INT_PIN) && touch_skip > S_MET_UPD_SKIP*4) {

		if (modesw_state == 0) {

			touch_skip = 0;
			fingers = gsl1680_read_data();
//        	sprintf(temp, "Fingers: %u  ", fingers);
//        	LCD_PrintText(350, 20,temp,Green,Black,0);

			buttonPressed = LCD_Button_Touch();

			if (buttonPressed >= 0) {
//				sprintf(temp, "Button: %d  ", buttonPressed);
//				LCD_PrintText(450, 20,temp,Green,Black,0);

				LCD_Buttons[buttonPressed].background = Black;
				LCD_Button_Draw(buttonPressed);

				switch (buttonPressed) {

				case 0: {
					Codec_Line_Gain_Adj(0);

					if (ts.band_mode > (MIN_BANDS - 1))
						UiDriverChangeBand(0);

					UiDriverUpdateFrequency(1);
					sd.dial_moved = 1;
					UiDriverDisplayFilterBW();
				}
					break;

				case 1: {
					Codec_Line_Gain_Adj(0);

					if (ts.band_mode > (MIN_BANDS - 1))
						UiDriverChangeBand(1);

					UiDriverUpdateFrequency(1);
					sd.dial_moved = 1;
					UiDriverDisplayFilterBW();
				}
					break;

				case 2: {
					UiDriverChangeTunningStep(0);
					UiDriverShowStep(df.tuning_step);
				}
					break;

				case 3: {
					UiDriverChangeTunningStep(1);
					UiDriverShowStep(df.tuning_step);
				}
					break;

				case 4: {
					LCD_Buttons[buttonPressed].led = 1;

					if (ts.agc_mode < AGC_MAX_MODE)
						ts.agc_mode += 1;
					else
						ts.agc_mode = 0;

					switch (ts.agc_mode) {
					case AGC_SLOW:
						sprintf(temp, "Slow");
						break;
					case AGC_MED:
						sprintf(temp, "Med");
						break;
					case AGC_FAST:
						sprintf(temp, "Fast");
						break;
					case AGC_CUSTOM:
						sprintf(temp, "Custom");
						break;
					case AGC_OFF:
						sprintf(temp, "Off");
						LCD_Buttons[buttonPressed].led = 0;
						break;

					}
					UiCalcAGCDecay();

					LCD_Buttons[buttonPressed].label = temp;
					LCD_Button_Draw(buttonPressed);

				}
					break;

				case 7:

				{

					ts.dmod_mode++;

					if (ts.dmod_mode > DEMOD_MAX_MODE - 1)
						ts.dmod_mode = DEMOD_USB;

					UiDriverShowMode();
					UiCalcRxPhaseAdj();	// set gain and phase values according to mode
					UiCalcRxIqGainAdj();
					UiDriverDisplayFilterBW();

					switch (ts.dmod_mode) {
					case DEMOD_USB:
						sprintf(temp, "USB");
						break;
					case DEMOD_LSB:
						sprintf(temp, "LSB");
						break;
					case DEMOD_AM:
						sprintf(temp, "AM");
						break;
					case DEMOD_CW:
						sprintf(temp, "CW");
						break;
					default:
						break;
					}

					LCD_Buttons[buttonPressed].label = temp;
					LCD_Button_Draw(buttonPressed);

				}
					break;

				case 8: {

					if (ts.dmod_mode != DEMOD_AM) {
						ts.filter_id++;
						//
						if (ts.filter_id >= AUDIO_MAX_FILTER)
							ts.filter_id = AUDIO_MIN_FILTER;
					} else {

						ts.filter_id = AUDIO_WIDE;
						ts.filter_wide_select++;
						//
						if (ts.filter_wide_select >= WIDE_FILTER_10K)
							ts.filter_wide_select = WIDE_FILTER_10K_AM;
					}

					//UiDriverChangeFilter(0);
					audio_driver_set_rx_audio_filter();
					UiDriverDisplayFilterBW();
					UiCalcRxPhaseAdj();	// set gain and phase values according to mode
					UiCalcRxIqGainAdj();

					switch (ts.filter_id) {
					case AUDIO_300HZ:
						sprintf(temp, "300Hz");
						break;
					case AUDIO_500HZ:
						sprintf(temp, "500Hz");
						break;
					case AUDIO_1P8KHZ:
						sprintf(temp, "1.8k");

						break;
					case AUDIO_2P3KHZ:
						sprintf(temp, "2.3k");
						break;
					case AUDIO_2P7KHZ:
						sprintf(temp, "2.7k");
						break;
					case AUDIO_2P9KHZ:
						sprintf(temp, "2.9k");
						break;
					case AUDIO_3P6KHZ:
						sprintf(temp, "3.6k");
						break;
					case AUDIO_WIDE:
						switch (ts.filter_wide_select) {
						case WIDE_FILTER_5K:
						case WIDE_FILTER_5K_AM:
							sprintf(temp, "5k");
							break;
						case WIDE_FILTER_6K:
						case WIDE_FILTER_6K_AM:
							sprintf(temp, "6k");
							break;
						case WIDE_FILTER_7K5:
						case WIDE_FILTER_7K5_AM:
							sprintf(temp, "7.5k");
							break;
						case WIDE_FILTER_10K:
						case WIDE_FILTER_10K_AM:
						default:
							sprintf(temp, "10k");
							break;
						}
						break;
					default:
						sprintf(temp, "Off");
						break;
					}

					LCD_Buttons[buttonPressed].label = temp;
					LCD_Button_Draw(buttonPressed);

				}
					break;

				case 9: {

					if ((!(ts.dsp_active & 1)) && (!(ts.dsp_active & 4)))// both NR and notch are inactive
						ts.dsp_active |= 1;					// turn on NR
					else if ((ts.dsp_active & 1) && (!(ts.dsp_active & 4))) {// NR active, notch inactive
						if (ts.dmod_mode != DEMOD_CW) {	// NOT in CW mode
							ts.dsp_active |= 4;				// turn on notch
							ts.dsp_active &= 0xfe;			// turn off NR
						} else {// CW mode - do not select notches, skip directly to "off"
							ts.dsp_active &= 0xfa;	// turn off NR and notch
						}
					} else if ((!(ts.dsp_active & 1)) && (ts.dsp_active & 4))//	NR inactive, notch active
						if ((ts.dmod_mode == DEMOD_AM)
								&& (ts.filter_id == AUDIO_WIDE))// was it AM with a 10 kHz filter selected?

							ts.dsp_active &= 0xfa;// it was AM + 10 kHz - turn off NR and notch
						else
							ts.dsp_active |= 1;			// no - turn on NR
					//
					else {
						ts.dsp_active &= 0xfa;		// turn off NR and notch
					}
					//
					ts.dsp_active_toggle = ts.dsp_active;// save update in "toggle" variable
					//
					ts.reset_dsp_nr = 1;		// reset DSP NR coefficients
					audio_driver_set_rx_audio_filter();	// update DSP settings
					ts.reset_dsp_nr = 0;
					//UiDriverChangeDSPMode();			// update on-screen display

					LCD_Buttons[buttonPressed].led = 1;

					if ((ts.dsp_active & 1) && (ts.dsp_active & 4)
							&& (ts.dmod_mode != DEMOD_CW)) {
						sprintf(temp, "NR+NOT");
						LCD_Buttons[buttonPressed].background = Blue;
						LCD_Buttons[buttonPressed].led = 1;
					} else if (ts.dsp_active & 1) {
						sprintf(temp, "NR");
						LCD_Buttons[buttonPressed].background = Blue;
					} else if (ts.dsp_active & 4) {
						sprintf(temp, "NOTCH");
						LCD_Buttons[buttonPressed].background = Blue;
					} else {
						sprintf(temp, "OFF");
						LCD_Buttons[buttonPressed].background =
						Grey_Recovery;

						LCD_Buttons[buttonPressed].led = 0;
					}

					LCD_Buttons[buttonPressed].label = temp;
					LCD_Button_Draw(buttonPressed);

				}
					break;

				case 5: {
					Codec_RestartI2S();
				}
					break;

				case 6: {

					if (ts.menu_item) {
						ts.menu_mode = 1;
						LCD_Buttons[buttonPressed].led = 1;
					} else {
						ts.menu_mode = 0;
						LCD_Buttons[buttonPressed].led = 0;
						LCD_PrintText(600, 0, "           ", White, Black, 0);
					}

					ts.menu_item++;
					if (ts.menu_item > 13)
						ts.menu_item = 0;

					switch (ts.menu_item) {
					case 0:
						LCD_PrintText(600, 0, "RF Gain    ", White, Black, 0);
						break;

					case 1:
						LCD_PrintText(600, 0, "DSP Noise R", White, Black, 0);
						break;

					case 2:
						LCD_PrintText(600, 0, "Audio Gain ", White, Black, 0);
						break;

					case 3:
						LCD_PrintText(600, 0, "Noise Blank", White, Black, 0);
						break;

					case 4:
						LCD_PrintText(600, 0, "Codec Gain ", White, Black, 0);
						break;

					case 5:
						LCD_PrintText(600, 0, "Scope Noise", White, Black, 0);
						break;

					case 6:
						LCD_PrintText(600, 0, "SSB 2k3 Fil", White, Black, 0);
						break;

					case 7:
						LCD_PrintText(600, 0, "AM WIDE Fil", White, Black, 0);
						break;

					case 8:
						LCD_PrintText(600, 0, "AGC Mode   ", White, Black, 0);
						break;

					case 9:
						LCD_PrintText(600, 0, "FFT Window ", White, Black, 0);
						break;

					case 10:
						LCD_PrintText(600, 0, "FFT Gain   ", White, Black, 0);
						break;

					case 11:
						LCD_PrintText(600, 0, "DSP Enable ", White, Black, 0);
						break;

					case 12:
						LCD_PrintText(600, 0, "Spect. Filt", White, Black, 0);
						break;

					case 13:
						LCD_PrintText(600, 0, "Spectrum En", White, Black, 0);
						break;

					default:
						LCD_PrintText(600, 0, "VOID     ", White, Black, 0);
						break;
					}
				}

					LCD_Button_Draw(buttonPressed);
					break;

				}

				LCD_Buttons[buttonPressed].background = Grey_Recovery;

				LCD_Button_Draw(buttonPressed);

				modesw_state = 1; // flag switch is pressed

			}

		} else
			modesw_state = 0;
	}
}

static void ui_buttons() {

	LCD_Button_t button;
	int8_t button1, button2, button3,button4, button5, button6, button7, button8, button9, button10;

	// Bucle principal

	//

	button.x = 0;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "-BND";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;
	//Add button
	button1 = LCD_Button_Add(&button);

	button.x = 114;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "+BND";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;
	//Add button
	button2 = LCD_Button_Add(&button);

	button.x = 228;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "-STEP";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;
	//Add button
	button3 = LCD_Button_Add(&button);

	button.x = 342;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "+STEP";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;
	//Add button
	button4 = LCD_Button_Add(&button);

	button.x = 456;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "AGC";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER | TM_BUTTON_FLAG_LED;
	button.led = 1;
	//Add button
	button5 = LCD_Button_Add(&button);

	button.x = 570;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "I2S_R";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;
	//Add button
	button6 = LCD_Button_Add(&button);

	button.x = 684;
	button.y = 430;
	button.width = 100;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = Grey_Recovery;
	button.label = "MENU";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER | TM_BUTTON_FLAG_LED;
	button.led = 0;
	//Add button
	button7 = LCD_Button_Add(&button);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Controles

	button.x = 0;
	button.y = 120;
	button.width = 98;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = White;
	button.label = "USB";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;

	//Add button
	button8 = LCD_Button_Add(&button);

	button.x = 100;
	button.y = 120;
	button.width = 98;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = White;
	button.label = "2.3k";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER;

	//Add button
	button9 = LCD_Button_Add(&button);
//
	button.x = 200;
	button.y = 120;
	button.width = 98;
	button.height = 50;
	button.background = Grey_Recovery;
	button.borderColor = White;
	button.label = "NR";
	button.color = White;
	button.font = &GL_Font16x24;
	button.image = GFX_boton; //Variable stored in
	button.flags = TM_BUTTON_FLAG_NOBORDER | TM_BUTTON_FLAG_LED;
	button.led = 0;
	//Add button
	button10 = LCD_Button_Add(&button);

//		button.x = 346;
//		button.y = 200;
//		button.width = 110;
//		button.height = 50;
//		button.background = Grey_Recovery;
//		button.borderColor = Black;
//		button.label = "I2S Rst";
//		button.color = White;
//		button.font = &GL_Font16x24;
//		button.image = GFX_boton; //Variable stored in
//		button.flags =  TM_BUTTON_FLAG_NOBORDER;
//		//Add button
//		button11 = LCD_Button_Add(&button);
//
//		button.x = 456;
//		button.y = 200;
//		button.width = 110;
//		button.height = 50;
//		button.background = Grey_Recovery;
//		button.borderColor = Black;
//		button.label = "TEST";
//		button.color = White;
//		button.font = &GL_Font16x24;
//		button.image = GFX_boton; //Variable stored in
//		button.flags =  TM_BUTTON_FLAG_NOBORDER;
//		//Add button
//		button12 = LCD_Button_Add(&button);

	LCD_Button_DrawAll();

}





