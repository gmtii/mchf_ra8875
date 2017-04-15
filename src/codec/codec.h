/*
 * codec.h - stm32f405_codec board codec interface routines
 *
 * Cut from stm32f4_discovery_audio_codec.c
 *
 */

#ifndef __codec_h
#define __codec_h

#include "radio.h"

#define WM8731
//#define AIC3204

#define WORD_SIZE_16 					0
#define WORD_SIZE_32 					1


/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED                       100000

// Mask for the bit EN of the I2S CFGR register
#define I2S_ENABLE_MASK                 0x0400

#define CODEC_STANDARD                	0x04
#define I2S_STANDARD                   	I2S_Standard_Phillips

/* The 7 bits Codec address (sent through I2C interface) */

#ifdef WM8731
#define CODEC_ADDRESS           		(W8731_ADDR_0<<1)
#define W8731_ADDR_0 					0x1A
#define W8731_ADDR_1 					0x1B
#endif

#ifdef AIC3204
#define AIC3204_ADDR 					0x18
#define CODEC_ADDRESS           		(AIC3204_ADDR<<1)

// AIC3204 pause delay
#define Codec_Pause						400
#define ADC_GAIN_MIN 					-24
#define ADC_GAIN_MAX 					40

#define CODEC_AIC3204_RESET_PIN            GPIO_Pin_8		// RESET AIC3204
#define CODEC_AIC3204_RESET_GPIO           GPIOC			// RESET AIC3204
#define CODEC_AIC3204_RESET_PINSRC         GPIO_PinSource8	// RESET AIC3204
#define CODEC_AIC3204_GPIO_CLOCK           RCC_AHB1Periph_GPIOC

#endif


// pin 10
#define CODEC_I2S_SCK 			GPIO_Pin_10						// BCLK
#define CODEC_I2S_SCK_SOURCE	GPIO_PinSource10
#define CODEC_I2S_SCK_PIO       GPIOC
// pin 12
#define CODEC_I2S_SDO 			GPIO_Pin_12						// DIN
#define CODEC_I2S_SDO_SOURCE	GPIO_PinSource12
#define CODEC_I2S_SDO_PIO       GPIOC
// pin 15
#define CODEC_I2S_WS			GPIO_Pin_15						// WCLK
#define CODEC_I2S_WS_SOURCE		GPIO_PinSource15
#define CODEC_I2S_WS_PIO  		GPIOA
// pin 11
#define CODEC_I2S_SDI 			GPIO_Pin_11						// DOUT
#define CODEC_I2S_SDI_SOURCE	GPIO_PinSource11
#define CODEC_I2S_SDI_PIO       GPIOC


// pin 9
#define CODEC_CLOCK 			GPIO_Pin_9
#define CODEC_CLOCK_SOURCE		GPIO_PinSource9
#define CODEC_CLOCK_PIO     	GPIOC



/*-----------------------------------
Hardware Configuration defines parameters
-----------------------------------------*/
/* I2S peripheral configuration defines */
#define CODEC_I2S                      SPI3
#define CODEC_I2S_EXT                  I2S3ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              (SPI3_BASE    + 0x0C)
#define CODEC_I2S_EXT_ADDRESS          (I2S3ext_BASE + 0x0C)
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_EXT_IRQ              SPI3_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOA)

#define AUDIO_I2S_IRQHandler           SPI3_IRQHandler
#define AUDIO_I2S_EXT_IRQHandler       SPI3_IRQHandler

#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord
#define DMA_MAX_SZE                    0xFFFF

/* I2S DMA Stream definitions */
#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream5
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF5

#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream2
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_2
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream2_IRQn
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF2
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF2
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF2

// --------------------------------------------------
// Registers
#define W8731_LEFT_LINE_IN				0x00		// 0000000
#define W8731_RIGHT_LINE_IN				0x01		// 0000001

#define W8731_LEFT_HEADPH_OUT			0x02		// 0000010
#define W8731_RIGHT_HEADPH_OUT			0x03		// 0000011

#define W8731_ANLG_AU_PATH_CNTR			0x04		// 0000100
#define W8731_DIGI_AU_PATH_CNTR			0x05		// 0000101

#define W8731_POWER_DOWN_CNTR			0x06		// 0000110
#define W8731_DIGI_AU_INTF_FORMAT		0x07		// 0000111

#define W8731_SAMPLING_CNTR				0x08		// 0001000
#define W8731_ACTIVE_CNTR				0x09		// 0001001
#define W8731_RESET						0x0F		// 0001111

// -------------------------------------------------

//#define W8731_DEEMPH_CNTR 				0x06		// WM8731 codec De-emphasis enabled
#define W8731_DEEMPH_CNTR 				0x00		// WM8731 codec De-emphasis disabled


/* High Layer codec functions */
uint32_t Codec_Init(uint32_t AudioFreq);

/* Low layer codec functions */
void    Codec_AudioInterface_Init(uint32_t AudioFreq);
void 	Codec_Reset(uint32_t AudioFreq,int32_t word_size);
void	Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue);
void    Codec_GPIO_Init(void);

#ifdef WM8731

void Codec_Volume(uint8_t vol);
void Codec_Line_Gain_Adj(uint8_t gain);

#endif

#ifdef AIC3204
void 	Codec_Line_Gain_Adj(int8_t gain);
void 	Codec_Volume(int8_t vol);
void 	Set_PGA_Gain(int PGA_gain);
void 	Codec_HP_Volume(int8_t HP_gain);
#endif

#endif

