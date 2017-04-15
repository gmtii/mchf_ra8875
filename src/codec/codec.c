/*
 * codec.c - Discovery board codec interface routines
 *
 * Cut from stm32f4_discovery_audio_codec.c
 *
 */

#include "codec.h"
#include "tm_stm32f4_i2c.h"
#include "tm_stm32f4_delay.h"

//		PinS PACK 1		PINS PACK 2		PINS PACK 3
//I2Cx	SCL		SDA		SCL		SDA		SCL	SDA			APB
//I2C1	PB6		PB7		PB8		PB9		PB6	PB9			1
//I2C2	PB10 	PB11 	PF1		PF0		PH4	PH5			1
//I2C3	PA8		PC9		PH7		PH8						1

uint32_t Codec_Init(uint32_t AudioFreq)
{
	/* Configure the Codec related IOs */
	Codec_GPIO_Init();

	/* Initialize the Control interface of the Audio Codec */
	TM_I2C_Init(I2C2, TM_I2C_PinsPack_1, 100000);

	/* Configure the I2S peripheral */
	Codec_AudioInterface_Init(AudioFreq);

	/* Reset the Codec Registers */
	Codec_Reset(AudioFreq,WORD_SIZE_16);

	return 0;
}

#ifdef WM8731

#ifdef CRISTAL
/**
  * @brief  Resets the audio codec. It restores the default configuration of the
  *         codec (this function shall be called before initializing the codec).
  * @note   This function calls an external driver function: The IO Expander driver.
  * @param  None
  * @retval None
  */


void Codec_Reset(uint32_t AudioFreq,int32_t word_size)
{

	// Reset register
	Codec_WriteRegister(W8731_RESET, 0);


	// Reg 00: Left Line In (0dB, mute off)
	Codec_WriteRegister(W8731_LEFT_LINE_IN,0x001F);

	// Reg 01: Right Line In (0dB, mute off)
	Codec_WriteRegister(W8731_RIGHT_LINE_IN,0x001F);

	// Reg 02: Left Headphone out (0dB)
	//Codec_WriteRegister(0x02,0x0079);
	// Reg 03: Right Headphone out (0dB)
	//Codec_WriteRegister(0x03,0x0079);

	Codec_Volume(0);

	// Reg 04: Analog Audio Path Control (DAC sel, ADC line, Mute Mic)
	Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0012);

	// Reg 05: Digital Audio Path Control(all filters disabled)
	// De-emphasis control, bx11x - 48kHz
	//                      bx00x - off
	// DAC soft mute		b1xxx - mute on
	//						b0xxx - mute off
	//
	Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,W8731_DEEMPH_CNTR);

	// Reg 06: Power Down Control (Clk off, Osc off, Mic Off)
	Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0042); // was 62 MIO 0x0042

	// Reg 07: Digital Audio Interface Format (i2s, 16/32 bit, slave)
	if(word_size == WORD_SIZE_16)
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x0042); // MIO: 0x0042
	else
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x000E);


	// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	// master clock: 12.5 Mhz
	if(AudioFreq == I2S_AudioFreq_48k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0040); // 0x0040 mclk=xto/2
	if(AudioFreq == I2S_AudioFreq_32k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0018);
	if(AudioFreq == I2S_AudioFreq_8k ) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x000C);

	// Reg 09: Active Control
	Codec_WriteRegister(W8731_ACTIVE_CNTR,0x0001);

	Codec_Volume(0x1F);
}

#endif



void Codec_Reset(uint32_t AudioFreq,int32_t word_size)
{
	if (!TM_I2C_IsDeviceConnected(I2C2, CODEC_ADDRESS))
	{

//		while(1){
//		GPIO_ToggleBits(GPIOG, GPIO_Pin_13|GPIO_Pin_14);
//		Delay(1e6);
//		}
	}


	// Reset register
	Codec_WriteRegister(W8731_RESET, 0);

	// Reg 00: Left Line In (0dB, mute off)
	Codec_WriteRegister(W8731_LEFT_LINE_IN,0x001F);

	// Reg 01: Right Line In (0dB, mute off)
	Codec_WriteRegister(W8731_RIGHT_LINE_IN,0x001F);

	// Reg 02: Left Headphone out (0dB)
	Codec_WriteRegister(0x02,0x0079);
	// Reg 03: Right Headphone out (0dB)
	Codec_WriteRegister(0x03,0x0079);

	Codec_Volume(0);

	// Reg 04: Analog Audio Path Control (DAC sel, ADC line, Mute Mic)
	Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0012);

	// Reg 05: Digital Audio Path Control(all filters disabled)
	// De-emphasis control, bx11x - 48kHz
	//                      bx00x - off
	// DAC soft mute		b1xxx - mute on
	//						b0xxx - mute off
	//
	Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,W8731_DEEMPH_CNTR);

	// Reg 06: Power Down Control (Clk off, Osc off, Mic Off)
	Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0062); // was 62 MIO 0x0042

	// Reg 07: Digital Audio Interface Format (i2s, 16/32 bit, slave)
	if(word_size == WORD_SIZE_16)
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x0002); // MIO: 0x0042
	else
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x000E);


	// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	// master clock: 12.5 Mhz
	if(AudioFreq == I2S_AudioFreq_96k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x001C);
	if(AudioFreq == I2S_AudioFreq_48k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0000); // 0x0040 mclk=xto/2
	if(AudioFreq == I2S_AudioFreq_32k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0018);
	if(AudioFreq == I2S_AudioFreq_8k ) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x000C);

	// Reg 09: Active Control
	Codec_WriteRegister(W8731_ACTIVE_CNTR,0x0001);

	Codec_Volume(0x1F);
}


void Codec_Volume(uint8_t vol)
{
	uint16_t lv;

	lv = vol+0x2F;

	if(lv < 0x2F) lv = 0x2F;	// limit min value
	if(lv > 0x7F) lv = 0x7F; 	// limit max value

	lv|= 0x80;

	Codec_WriteRegister(W8731_RIGHT_HEADPH_OUT,lv);	// value selected for 0.5VRMS at AGC setting

	// Reg 02: Speaker - variable volume
	Codec_WriteRegister(W8731_LEFT_HEADPH_OUT,lv);
}

void Codec_Line_Gain_Adj(uint8_t gain)
{
	uint16_t l_gain;
	//printf("codec line gain adjust = %d\n\r",gain);

	l_gain = (uint16_t)gain;
	//
	// Use Reg 00: Left Line In, set MSB to adjust gain of both channels simultaneously
	//
	l_gain |= 0x100;	// set MSB of control word for "LRINBOTH" flag
	//
	Codec_WriteRegister(W8731_LEFT_LINE_IN,l_gain);


}

void Codec_RestartI2S()
{
    // Reg 09: Active Control
    Codec_WriteRegister(W8731_ACTIVE_CNTR,0x0000);
    non_os_delay();
    // Reg 09: Active Control
    Codec_WriteRegister(W8731_ACTIVE_CNTR,0x0001);
}

#endif


#ifdef AIC3204

void Codec_Reset(uint32_t AudioFreq,int32_t word_size)
{


	if (!(TM_I2C_IsDeviceConnected(I2C2, CODEC_ADDRESS)))
	{

		while(1){
		GPIO_ToggleBits(GPIOG, GPIO_Pin_13|GPIO_Pin_14);
		Delay(1e6);
		}
	}
	// I2S standard mode
	// 12288000 / ( NADC*MADC*AOSR)= 48 khz = fs
	// BCLK= ( MCLK/2 ) /
	// NADC=NDAC=1 bandwidth -> fs / NADC = fs -> full spectrum
	// MADC=MDAC=2
	// AOSR


	/* Power Down the codec */

	GPIO_ResetBits(CODEC_AIC3204_RESET_GPIO,CODEC_AIC3204_RESET_PIN);

	/* wait for a delay to insure registers erasing */
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);


	/* Power on the codec */
	GPIO_SetBits(CODEC_AIC3204_RESET_GPIO,CODEC_AIC3204_RESET_PIN);

	// Point to page 0
	Codec_WriteRegister(0x00, 0x00);
	Delay(Codec_Pause);

	//software reset codec
	Codec_WriteRegister(0x01, 0x01);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);
	Delay(Codec_Pause);


	//***********************************************************************
	// Power System  Config and Power Up
	//***********************************************************************

	// Point to page 1
	Codec_WriteRegister(0x00, 0x01);
	Delay(Codec_Pause);

	//Disable crude AVDD generation from DVDD
	Codec_WriteRegister(0x01, 0x08);
	Delay(Codec_Pause);

	//Enable Analog Blocks and LDO
	Codec_WriteRegister(0x02, 0x01);
	Delay(Codec_Pause);

	//***********************************************************************
	// PLL and Clocks config and Power Up
	//***********************************************************************
	// Point to page 0
	Codec_WriteRegister(0x00, 0x00);
	Delay(Codec_Pause);

	//For AIC Master Running of 12 Mhz Clock I2C standard mode
	Codec_WriteRegister(0x1b, 0x00); // BCLK and WCLK is set as input to AIC3204(Slave)
	Delay(Codec_Pause);

	// BDIV_CLKIN = DAC_MOD_CLK
	Codec_WriteRegister(0x1d, 0x01);
	Delay(Codec_Pause);

	// MCLK pin is CODEC_CLKIN
	Codec_WriteRegister(0x04, 0x00);
	Delay(Codec_Pause);

	// PLL power down
	Codec_WriteRegister(0x05, 0x00);
	Delay(Codec_Pause);

	// Data offset = 0
	Codec_WriteRegister(0x1c, 0x00);  // Data offset = 0
	Delay(Codec_Pause);

	//For 32 bit clocks per frame in Master mode ONLY
	// BCLK=DAC_CLK/NDAC/N =(12288000/2/4) = 1.536MHz = 32*fs
	Codec_WriteRegister(0x1e, 0x84);
	Delay(Codec_Pause);

	//Hi_Byte(DOSR) for DOSR = 0  decimal or 0x080 DAC oversamppling
	Codec_WriteRegister(0x0d, 0x00);
	Delay(Codec_Pause);

	//Lo_Byte(DOSR) for DOSR = 128 decimal or 0x080
	Codec_WriteRegister(0x0e, 0x80);
	Delay(Codec_Pause);

	//AOSR for AOSR = 128 decimal or 0x0080 for decimation filters 1 to 6
	Codec_WriteRegister(0x14, 0x80);
	Delay(Codec_Pause);

	//Power up NDAC and set NDAC value to 2
	Codec_WriteRegister(0x0b, 0x81);
	Delay(Codec_Pause);

	//Power up MDAC and set MDAC value to 1
	Codec_WriteRegister(0x0c, 0x82);
	Delay(Codec_Pause);

	//Power up NADC and set NADC value to 2
	Codec_WriteRegister(0x12, 0x81);
	Delay(Codec_Pause);

	//Power up MADC and set MADC value to 1
	Codec_WriteRegister(0x13, 0x82);
	Delay(Codec_Pause);


	//***********************************************************************
	// DAC ROUTING and Power Up
	//***********************************************************************
	// Point to page 0

	//Select page 1
	Codec_WriteRegister(0x00, 0x01);
	Delay(Codec_Pause);

	//LDAC AFIR routed to HPL
	Codec_WriteRegister(0x0c, 0x08);
	Delay(Codec_Pause);

	//RDAC AFIR routed to HPR
	Codec_WriteRegister(0x0d, 0x08);
	Delay(Codec_Pause);

	//LDAC AFIR routed to LOL
	Codec_WriteRegister(0x0e, 0x08);
	Delay(Codec_Pause);

	//RDAC AFIR routed to LOR
	Codec_WriteRegister(0x0f, 0x08);
	Delay(Codec_Pause);

	//Select page 0
	Codec_WriteRegister(0x00, 0x00);
	Delay(Codec_Pause);

	//Left vol=right vol
	Codec_WriteRegister(0x40, 0x02);
	Delay(Codec_Pause);

	//Left DAC gain ; Right tracks Left, Zero db gain
	Codec_WriteRegister(0x41, 0x00);
	Delay(Codec_Pause);

	//Power up left,right data paths and set channel
	Codec_WriteRegister(0x3f, 0xd4);
	Delay(Codec_Pause);

	//Select page 1
	Codec_WriteRegister(0x00, 0x01);
	Delay(Codec_Pause);

	//Unmute HPL , set gain = 0 db
	Codec_WriteRegister(0x10, 0);
	Delay(Codec_Pause);

	//Unmute HPR , set gain = 0 db
	Codec_WriteRegister(0x11, 0);
	Delay(Codec_Pause);

	//Unmute LOL , set gain
	Codec_WriteRegister(0x12, 0x00);
	Delay(Codec_Pause);

	//Unmute LOR , set gain
	Codec_WriteRegister(0x13, 0x00);
	Delay(Codec_Pause);

	Codec_WriteRegister(0x3c, 0x80);  //PGAr 0 db Gain
	Delay(Codec_Pause);

	Codec_WriteRegister(0x19, 0x27);  //MAR -30 db Gain
	Delay(Codec_Pause);

	//Power up HPL,HPR,LOL,LOR
	Codec_WriteRegister(0x09, 0x3d); // added MAR
	Delay(Codec_Pause);

	//***********************************************************************
	//* ADC ROUTING and Power Up
	//***********************************************************************
	//Select page 1
	Codec_WriteRegister(0x00, 0x01);
	Delay(Codec_Pause);

	// Disable MICBIAS
	Codec_WriteRegister(0x33, 0x00);
	Delay(Codec_Pause);

	// IN2_L to LADC_P through 40 kohm
	Codec_WriteRegister(0x34, 0x10);
	Delay(Codec_Pause);

	//IN2_R to RADC_P through 40 kohmm
	Codec_WriteRegister(0x37, 0x10);
	Delay(Codec_Pause);

	// CM_1 (common mode) to LADC_M through 10 kohm
	Codec_WriteRegister(0x36, 0x01);
	Delay(Codec_Pause);

	// CM_1 (common mode) to RADC_M through 10 kohm
	Codec_WriteRegister(0x39, 0x01);
	Delay(Codec_Pause);

	//MIC_PGA_L unmute
	Codec_WriteRegister(0x3b, 0x80);  //30 db Gain
	Delay(Codec_Pause);

	//MIC_PGA_R unmute, 20 db Gain
	Codec_WriteRegister(0x3c, 0x80);  //30 db Gain
	Delay(Codec_Pause);

	//Select page 0
	Codec_WriteRegister(0x00, 0x00);
	Delay(Codec_Pause);

	//Powerup Left and Right ADC
	Codec_WriteRegister(0x51, 0xc0);
	Delay(Codec_Pause);

	//Unmute Left and Right ADC
	Codec_WriteRegister(0x52, 0x00);
	Delay(Codec_Pause);

	Set_PGA_Gain(1);
	Codec_HP_Volume(0);


}

void Codec_Volume(int8_t Line_gain)
{


	if (Line_gain > 29)
		Line_gain = 29;

	if (Line_gain < -6)
		Line_gain = -6;

	if (Line_gain < 0) {
		Line_gain = 64- abs(Line_gain);
	}

	Codec_WriteRegister( 0x00, 0x01);
	Delay(Codec_Pause);

	//Unmute HPL , set gain = 0 db
	Codec_WriteRegister(0x12, Line_gain);
	Delay(Codec_Pause);

	//Unmute HPR , set gain = 0 db
	Codec_WriteRegister(0x13, Line_gain);
	Delay(Codec_Pause);

}

void Codec_HP_Volume(int8_t HP_gain)
{

	if (HP_gain < 0) HP_gain += 64;

	Codec_WriteRegister( 0x00, 0x01);
	Delay(Codec_Pause);

	//Unmute HPL , set gain = 0 db
	Codec_WriteRegister(0x10, HP_gain);
	Delay(Codec_Pause);

	//Unmute HPR , set gain = 0 db
	Codec_WriteRegister(0x11, HP_gain);
	Delay(Codec_Pause);

}

void Codec_Line_Gain_Adj(int8_t ADC_gain)
{

	if (ADC_gain > ADC_GAIN_MAX)
		ADC_gain = ADC_GAIN_MAX;
	if (ADC_gain < ADC_GAIN_MIN)
		ADC_gain = ADC_GAIN_MIN;
	if (ADC_gain < 0) ADC_gain += 127;

	Codec_WriteRegister( 0x00, 0x00);
	Delay(Codec_Pause);

	Codec_WriteRegister( 0x53, ADC_gain);
	Delay(Codec_Pause);

	Codec_WriteRegister( 0x54, ADC_gain);
	Delay(Codec_Pause);

}


void Set_PGA_Gain(int PGA_gain)
{

#define PGA_GAIN_MIN 0
#define PGA_GAIN_MAX 95


	if (PGA_gain < PGA_GAIN_MIN)
		PGA_gain = PGA_GAIN_MIN;
	if (PGA_gain > PGA_GAIN_MAX)
		PGA_gain = PGA_GAIN_MAX;

	Codec_WriteRegister( 0x00, 0x00);


	Codec_WriteRegister( 0x3B, PGA_gain);


	Codec_WriteRegister( 0x3C, PGA_gain);

}   // End of Set_PGA_gain

#endif

/**
  * @brief  Writes a Byte to a given register into the audio codec through the
            control interface (I2C)
  * @param  RegisterAddr: The address (location) of the register to be written.
  * @param  RegisterValue: the Byte value to be written into destination register.
  * @retval 0 if correct communication, else wrong communication
  */
void Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue)
{

#ifdef WM8731

	/* Assemble 2-byte data in WM8731 format */
	uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x01);
	uint8_t Byte2 = RegisterValue&0xFF;

#endif

#ifdef AIC3204

	uint8_t Byte1 = RegisterAddr;
	uint8_t Byte2 = RegisterValue&0xFF;

#endif

	TM_I2C_Write(I2C2, CODEC_ADDRESS, Byte1, Byte2);

}

/**
  * @brief  Initializes the Audio Codec audio interface (I2S)
  * @note   This function assumes that the I2S input clock (through PLL_R in
  *         Devices RevA/Z and through dedicated PLLI2S_R in Devices RevB/Y)
  *         is already configured and ready to be used.
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral.
  * @retval None
  */
void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
	I2S_InitTypeDef I2S_InitStructure;

	/* Enable the CODEC_I2S peripheral clock */
	RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

#ifdef AIC3204

	/* CODEC_I2S peripheral configuration for master TX */
	SPI_I2S_DeInit(CODEC_I2S);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx; //I2S_Mode_MasterTx; MIO: I2S_Mode_SlaveTx
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable; // using MCO2
#else

	SPI_I2S_DeInit(CODEC_I2S);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx; //I2S_Mode_MasterTx; MIO: I2S_Mode_SlaveTx
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable; //

#endif


	/* Initialize the I2S main channel for TX */
	I2S_Init(CODEC_I2S, &I2S_InitStructure);

	/* Initialize the I2S extended channel for RX */
	I2S_FullDuplexConfig(CODEC_I2S_EXT, &I2S_InitStructure);
}

/**
  * @brief Initializes IOs used by the Audio Codec (on the control and audio
  *        interfaces).
  * @param  None
  * @retval None
  */
void Codec_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable I2S and I2C GPIO clocks */
	RCC_AHB1PeriphClockCmd( CODEC_I2S_GPIO_CLOCK, ENABLE);

#ifdef AIC3204

	RCC_AHB1PeriphClockCmd( CODEC_AIC3204_GPIO_CLOCK, ENABLE);

	// AIC3204 RESET PIN AS OUTPUT

	GPIO_InitStructure.GPIO_Pin = CODEC_AIC3204_RESET_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(CODEC_AIC3204_RESET_GPIO, &GPIO_InitStructure);

#endif

    // CODEC_I2S output pins configuration: WS, SCK SD0 and SDI pins
    GPIO_InitStructure.GPIO_Pin 	= CODEC_I2S_SCK | CODEC_I2S_SDO | CODEC_I2S_SDI;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
    GPIO_Init(CODEC_I2S_SDO_PIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(CODEC_I2S_WS_PIO, &GPIO_InitStructure);

    // Configure MCO2 (PC9)
    GPIO_InitStructure.GPIO_Pin = CODEC_CLOCK;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(CODEC_CLOCK_PIO, &GPIO_InitStructure);

    // Output I2S PLL via MCO2 pin - 12.288 Mhz
    RCC_MCO2Config(RCC_MCO2Source_PLLI2SCLK, RCC_MCO2Div_3);

    // Connect pins to I2S peripheral
    GPIO_PinAFConfig(CODEC_I2S_WS_PIO,	CODEC_I2S_WS_SOURCE,  CODEC_I2S_GPIO_AF);
    GPIO_PinAFConfig(CODEC_I2S_SDO_PIO, CODEC_I2S_SCK_SOURCE, CODEC_I2S_GPIO_AF);
    GPIO_PinAFConfig(CODEC_I2S_SDO_PIO,	CODEC_I2S_SDO_SOURCE, CODEC_I2S_GPIO_AF);
    GPIO_PinAFConfig(CODEC_I2S_SDO_PIO, CODEC_I2S_SDI_SOURCE, CODEC_I2S_GPIO_AF);
}
