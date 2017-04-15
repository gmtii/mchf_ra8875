/*
 º * ui_ili9488.c
 *
 *  Created on: 22/1/2016
 *      Author: itx
 */

#include "radio.h"
#include "ui_ra8875.h"
#include "ui_ra8875_fonts.h"
#include "vu.h"

// Saved fonts
extern sFONT GL_Font8x8;
extern sFONT GL_Font8x12;
extern sFONT GL_Font8x12_bold;
extern sFONT GL_Font12x12;
extern sFONT GL_Font16x24;
extern sFONT GL_SevenSegNum;
extern sFONT GL_Big;

static sFONT *fontList[] = { &GL_Font8x12_bold, &GL_Font16x24, &GL_Font12x12,
		&GL_Font8x12, &GL_Font8x8, &GL_SevenSegNum, &GL_Big, };

// Transceiver state public structure
__IO TransceiverState ts;

// Spectrum display

extern SpectrumDisplay sd;

extern __IO OscillatorState os;	// oscillator values - including Si570 startup frequency, displayed on splash screen

// Local Variables

static volatile bool isDmaTransferOk=true;

// Local functions

static inline void LCD_SendCommand(uint16_t command);
static inline void LCD_SendData(uint16_t data);
static inline void LCD_SendRegister(uint16_t cmd, uint16_t data);
static void LCD_CheckBusy(void);

static void LCD_Delay(ulong delay);

void LCD_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
		uint16_t YBottom);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LCD_ParallelInit(void) {

	/** FMC GPIO Configuration
	 PE7   ------> FMC_D4
	 PE8   ------> FMC_D5
	 PE9   ------> FMC_D6
	 PE10   ------> FMC_D7
	 PE11   ------> FMC_D8
	 PE12   ------> FMC_D9
	 PE13   ------> FMC_D10
	 PE14   ------> FMC_D11
	 PE15   ------> FMC_D12
	 PD8   ------> FMC_D13
	 PD9   ------> FMC_D14
	 PD10   ------> FMC_D15
	 PD11   ------> FMC_A16
	 PD14   ------> FMC_D0
	 PD15   ------> FMC_D1
	 PD0   ------> FMC_D2
	 PD1   ------> FMC_D3
	 PD4   ------> FMC_NOE
	 PD5   ------> FMC_NWE
	 PD7   ------> FMC_NE1
	 */

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);     // D2
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);     // D3
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FMC);     // NOE -> RD
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FMC);     // NWE -> WR
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FMC);     // NE1 -> CS
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);     // D13
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);     // D14
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);    // D15
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FMC);    // A16 -> RS A16
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);    // D0
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);    // D1
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FMC);     // D4
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FMC);     // D5
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FMC);     // D6
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FMC);    // D7
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FMC);    // D8
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FMC);    // D9
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FMC);    // D10
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FMC);    // D11
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FMC);    // D12

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4
			| GPIO_Pin_5 | GPIO_Pin_7 |
			GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14
			| GPIO_Pin_15;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9
			| GPIO_Pin_10 | GPIO_Pin_11 |
			GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// Configure GPIO PIN for Reset
	GPIO_InitStructure.GPIO_Pin = RA8875_RST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RA8875_RST_PORT, &GPIO_InitStructure);

	// Configure GPIO PIN for WAIT
	GPIO_InitStructure.GPIO_Pin = RA8875_WAIT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(RA8875_WAIT_PORT, &GPIO_InitStructure);

}

//*----------------------------------------------------------------------------
//* Function Name       : LCD_FMCConfig
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void LCD_FMCConfig(void)

{
	FMC_NORSRAMInitTypeDef FMC_NORSRAMInitDef;
	FMC_NORSRAMTimingInitTypeDef FMC_NORSRAMTimingInitDef_Write,
			FMC_NORSRAMTimingInitDef_Read;

	/* Enable FMC clock */

	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

	/* FMC SSD1963 as a NORSRAM device initialization sequence ---*/

	/* Step 1 ----------------------------------------------------------*/

	/* Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2) */

	FMC_NORSRAMTimingInitDef_Write.FMC_AddressSetupTime = 1;      //0x0F
	FMC_NORSRAMTimingInitDef_Write.FMC_AddressHoldTime = 0x00;
	FMC_NORSRAMTimingInitDef_Write.FMC_DataSetupTime = 1;      //0x0F
	FMC_NORSRAMTimingInitDef_Write.FMC_BusTurnAroundDuration = 0;
	FMC_NORSRAMTimingInitDef_Write.FMC_CLKDivision = 0x00;  //0x01
	FMC_NORSRAMTimingInitDef_Write.FMC_DataLatency = 0x00;
	FMC_NORSRAMTimingInitDef_Write.FMC_AccessMode = FMC_AccessMode_A;

	/* Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2) */

	FMC_NORSRAMTimingInitDef_Read.FMC_AddressSetupTime = 0x0A;  //0x0A
	FMC_NORSRAMTimingInitDef_Read.FMC_AddressHoldTime = 0x00;
	FMC_NORSRAMTimingInitDef_Read.FMC_DataSetupTime = 0x0A;      //0x0A
	FMC_NORSRAMTimingInitDef_Read.FMC_BusTurnAroundDuration = 0;
	FMC_NORSRAMTimingInitDef_Read.FMC_CLKDivision = 0x01;  //0x01
	FMC_NORSRAMTimingInitDef_Read.FMC_DataLatency = 0x00;
	FMC_NORSRAMTimingInitDef_Read.FMC_AccessMode = FMC_AccessMode_A;

	/* FMC SDRAM control configuration */

	FMC_NORSRAMInitDef.FMC_Bank = FMC_Bank1_NORSRAM1;
	FMC_NORSRAMInitDef.FMC_DataAddressMux = FMC_DataAddressMux_Disable;
	FMC_NORSRAMInitDef.FMC_MemoryType = FMC_MemoryType_SRAM;
	FMC_NORSRAMInitDef.FMC_MemoryDataWidth = FMC_NORSRAM_MemoryDataWidth_16b;
	FMC_NORSRAMInitDef.FMC_BurstAccessMode = FMC_BurstAccessMode_Disable;
	FMC_NORSRAMInitDef.FMC_AsynchronousWait = FMC_AsynchronousWait_Disable;
	FMC_NORSRAMInitDef.FMC_WaitSignalPolarity = FMC_WaitSignalPolarity_Low;
	FMC_NORSRAMInitDef.FMC_WrapMode = FMC_WrapMode_Disable;
	FMC_NORSRAMInitDef.FMC_WaitSignalActive =
	FMC_WaitSignalActive_BeforeWaitState;
	FMC_NORSRAMInitDef.FMC_WriteOperation = FMC_WriteOperation_Enable;
	FMC_NORSRAMInitDef.FMC_WaitSignal = FMC_WaitSignal_Disable;
	FMC_NORSRAMInitDef.FMC_ExtendedMode = FMC_ExtendedMode_Disable;
	FMC_NORSRAMInitDef.FMC_WriteBurst = FMC_WriteBurst_Disable;
	FMC_NORSRAMInitDef.FMC_ContinousClock = FMC_CClock_SyncOnly; //FMC_CClock_SyncAsync
	FMC_NORSRAMInitDef.FMC_ReadWriteTimingStruct =
			&FMC_NORSRAMTimingInitDef_Read;
	FMC_NORSRAMInitDef.FMC_WriteTimingStruct = &FMC_NORSRAMTimingInitDef_Write;

	/* DISABLING THE NORSRAM BANK WHILE CHANGING SPEED */

	FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, DISABLE);

	/* FMC NORSRAM bank initialization */

	FMC_NORSRAMInit(&FMC_NORSRAMInitDef);

	/* FMC NORSRAM bank enable */

	FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, ENABLE);

}

void LCD_FMCConfig_Slow(void) {

	FMC_NORSRAMInitTypeDef FMC_NORSRAMInitDef;
	FMC_NORSRAMTimingInitTypeDef FMC_NORSRAMTimingInitDef;

	/* Enable FMC clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

	/* FMC RA8875 as a NORSRAM device initialization sequence ---*/
	/* Step 1 ----------------------------------------------------------*/
	/* Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2) */
	FMC_NORSRAMTimingInitDef.FMC_AddressSetupTime = 0x0F;
	FMC_NORSRAMTimingInitDef.FMC_AddressHoldTime = 0x00;
	FMC_NORSRAMTimingInitDef.FMC_DataSetupTime = 0xFF;
	FMC_NORSRAMTimingInitDef.FMC_BusTurnAroundDuration = 0x00;
	FMC_NORSRAMTimingInitDef.FMC_CLKDivision = 0x00;
	FMC_NORSRAMTimingInitDef.FMC_DataLatency = 0x00;
	FMC_NORSRAMTimingInitDef.FMC_AccessMode = FMC_AccessMode_A;

	/* FMC SDRAM control configuration */
	FMC_NORSRAMInitDef.FMC_Bank = FMC_Bank1_NORSRAM1;
	FMC_NORSRAMInitDef.FMC_DataAddressMux = FMC_DataAddressMux_Disable;
	FMC_NORSRAMInitDef.FMC_MemoryType = FMC_MemoryType_SRAM;
	FMC_NORSRAMInitDef.FMC_MemoryDataWidth = FMC_NORSRAM_MemoryDataWidth_16b;
	FMC_NORSRAMInitDef.FMC_BurstAccessMode = FMC_BurstAccessMode_Disable;
	FMC_NORSRAMInitDef.FMC_AsynchronousWait = FMC_AsynchronousWait_Disable;
	FMC_NORSRAMInitDef.FMC_WaitSignalPolarity = FMC_WaitSignalPolarity_Low;
	FMC_NORSRAMInitDef.FMC_WrapMode = FMC_WrapMode_Disable;
	FMC_NORSRAMInitDef.FMC_WaitSignalActive = 	FMC_WaitSignalActive_BeforeWaitState;
	FMC_NORSRAMInitDef.FMC_WriteOperation = FMC_WriteOperation_Enable;
	FMC_NORSRAMInitDef.FMC_WaitSignal = FMC_WaitSignal_Disable;
	FMC_NORSRAMInitDef.FMC_ExtendedMode = FMC_ExtendedMode_Disable;
	FMC_NORSRAMInitDef.FMC_WriteBurst = FMC_WriteBurst_Disable;
	FMC_NORSRAMInitDef.FMC_ContinousClock = FMC_CClock_SyncOnly; //FMC_CClock_SyncAsync
	FMC_NORSRAMInitDef.FMC_ReadWriteTimingStruct = &FMC_NORSRAMTimingInitDef;
	FMC_NORSRAMInitDef.FMC_WriteTimingStruct = &FMC_NORSRAMTimingInitDef;
	/* FMC NORSRAM bank initialization */
	FMC_NORSRAMInit(&FMC_NORSRAMInitDef);
	/* FMC NORSRAM bank enable */
	FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, ENABLE);

}

static DMA_InitTypeDef DMA_InitStructure;

void LCD_PrepareDMA() {

	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable DMA lock
	RCC_AHB1PeriphClockCmd( DMA_STREAM_CLOCK, ENABLE);

	// Reset DMA Stream registers (for debug purpose)
	DMA_DeInit( DMA_STREAM);
	DMA_Cmd(DMA_STREAM, DISABLE);
	DMA_StructInit(&DMA_InitStructure);

	// Configure DMA Stream
	DMA_InitStructure.DMA_Channel = DMA_CHANNEL;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) 0;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) (&(LCD_RAM));
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;

	DMA_Init( DMA_STREAM, &DMA_InitStructure);

	// Enable DMA Stream Transfer Complete interrupt
	DMA_ITConfig( DMA_STREAM, DMA_IT_TC, ENABLE);

	// Enable the DMA Stream IRQ Channel
	NVIC_InitStructure.NVIC_IRQChannel = DMA_STREAM_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ClearITPendingBit( DMA_STREAM, DMA_STREAM_STATUS_BIT);

} // LCD_TransferDataDMA

/////////////////////////////////////////////////////////////

void DMA2_Stream7_IRQHandler(void) {

	// Test on DMA Stream Transfer Complete interrupt

	if (DMA_GetITStatus(DMA_STREAM, DMA_STREAM_STATUS_BIT) == SET) {

		// Clear DMA Stream Transfer Complete interrupt pending bit

		DMA_ClearITPendingBit( DMA_STREAM, DMA_STREAM_STATUS_BIT);
	}
}

void LCD_TransferDataDMA(void *buffer, uint32_t size)
{
    if (size % 2 == 0)  {

    	size /= 2;

    	while (DMA_STREAM->CR & DMA_SxCR_EN) { asm(""); }

    	DMA_STREAM->PAR = (uint32_t)buffer;
        DMA_SetCurrDataCounter(DMA_STREAM,size);
        DMA_Cmd(DMA_STREAM, ENABLE);               //Enable the DMA stream
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief	Check busy
 * @param	None
 * @retval	None
 */
static void CheckBusy() {

	while (!(GPIO_ReadInputDataBit(RA8875_WAIT_PORT, RA8875_WAIT_PIN))) {
		LCD_Delay(10);
	}
}

static inline void LCD_SendCommand(uint16_t cmd) {
	LCD_REG = cmd;
}
static inline void LCD_SendData(uint16_t data) {
	LCD_RAM = data;
}

static inline void LCD_SendRegister(uint16_t cmd, uint16_t data) {
	LCD_CheckBusy();
	LCD_REG = cmd;
	LCD_RAM = data;
}

static inline uint16_t LCD_StatusRead() {
	return LCD_REG;
}

static inline uint16_t LCD_DataRead() {
	return LCD_RAM;
}

static void LCD_CheckBusy() {

	uint16_t temp;
	do {
		temp = LCD_StatusRead();
	} while ((temp & 0x80) == 0x80);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LCD_InitLCD(void) {

	LCD_ParallelInit();
	LCD_FMCConfig_Slow();

	RA8875_RST_SET;
	LCD_Delay(10000);
	RA8875_RST_RESET;	// 20 metrOs
	LCD_Delay(50000);
	RA8875_RST_SET;
	LCD_Delay(10000);

	/* Software reset the LCD */
	LCD_SendRegister(LCD_PWRR, 0x01);
	LCD_Delay(100000);
	LCD_SendRegister(LCD_PWRR, 0x00);
	LCD_Delay(100000);

	LCD_SendRegister(LCD_PLLC1, 0x0a);
	LCD_Delay(100000);
	LCD_SendRegister(LCD_PLLC2, 0x02);
	LCD_Delay(100000);

	LCD_SendCommand(0x10);	 //SYSR   bit[4:3] color  bit[2:1]=  MPU interface

	LCD_SendData(0x0F);   //            65K 16 bit 8080 mpu interface

	LCD_SendCommand(LCD_PCSR);    //PCLK
	LCD_SendData(0x80);   // 00b: PCLK period = System Clock period.
	LCD_Delay(100000);

	//Horizontal set
	LCD_SendCommand(0x14); //HDWR//Horizontal Display Width Setting Bit[6:0]
	LCD_SendData(0x63); //Horizontal display width(pixels) = (HDWR + 1)*8
	LCD_SendCommand(0x15); //Horizontal Non-Display Period Fine Tuning Option Register (HNDFTR)
	LCD_SendData(0x00); //Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]
	LCD_SendCommand(0x16); //HNDR//Horizontal Non-Display Period Bit[4:0]
	LCD_SendData(0x03); //Horizontal Non-Display Period (pixels) = (HNDR + 1)*8
	LCD_SendCommand(0x17); //HSTR//HSYNC Start Position[4:0]
	LCD_SendData(0x03); //HSYNC Start Position(PCLK) = (HSTR + 1)*8
	LCD_SendCommand(0x18); //HPWR//HSYNC Polarity ,The period width of HSYNC.
	LCD_SendData(0x0B); //HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8
	//Vertical set
	LCD_SendCommand(0x19); //VDHR0 //Vertical Display Height Bit [7:0]
	LCD_SendData(0xdf); //Vertical pixels = VDHR + 1
	LCD_SendCommand(0x1a); //VDHR1 //Vertical Display Height Bit [8]
	LCD_SendData(0x01); //Vertical pixels = VDHR + 1
	LCD_SendCommand(0x1b); //VNDR0 //Vertical Non-Display Period Bit [7:0]
	LCD_SendData(0x20); //Vertical Non-Display area = (VNDR + 1)
	LCD_SendCommand(0x1c); //VNDR1 //Vertical Non-Display Period Bit [8]
	LCD_SendData(0x00); //Vertical Non-Display area = (VNDR + 1)
	LCD_SendCommand(0x1d); //VSTR0 //VSYNC Start Position[7:0]
	LCD_SendData(0x16); //VSYNC Start Position(PCLK) = (VSTR + 1)
	LCD_SendCommand(0x1e); //VSTR1 //VSYNC Start Position[8]
	LCD_SendData(0x00); //VSYNC Start Position(PCLK) = (VSTR + 1)
	LCD_SendCommand(0x1f); //VPWR //VSYNC Polarity ,VSYNC Pulse Width[6:0]
	LCD_SendData(0x01); //VSYNC Pulse Width(PCLK) = (VPWR + 1)

	LCD_SetActiveWindow(0, 799, 0, 479);

	LCD_SendCommand(0x8a); //PWM setting
	LCD_SendData(0x80);
	LCD_SendCommand(0x8a); //PWM setting
	LCD_SendData(0x81); //open PWM
	LCD_SendCommand(0x8b); //Backlight brightness setting
	LCD_SendData(0x1f); //Brightness parameter 0xff-0x00

	LCD_SendRegister(0X01, 0X80); //display on

	//LCD_SendRegister(LCD_DPCR,0b00001111); // rotacion 180º

	LCD_ClearScreen(0);

	LCD_SendRegister(LCD_MCLR, 0x80);

	LCD_FMCConfig();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LCD_SetCursorA(int16_t Xpos, int16_t Ypos) {

	uint16_t temp;

	/* Horizontal start */
	temp = Xpos;
	LCD_SendRegister(LCD_CURH0, temp);
	temp = Xpos >> 8;
	LCD_SendRegister(LCD_CURH1, temp);

	/* Horizontal end */
	temp = Ypos;
	LCD_SendRegister(LCD_CURV0, temp);
	temp = Ypos >> 8;
	LCD_SendRegister(LCD_CURV1, temp);

}

void LCD_DrawColorPoint(unsigned short Xpos, unsigned short Ypos,
		unsigned short point) {

	LCD_SetCursorA(Xpos, Ypos);
	LCD_SendCommand(LCD_MRWC);
	LCD_RAM = point;
}

void LCD_SetForegroundColor(uint16_t Color) {
	LCD_SendRegister(LCD_FGCR0, (uint16_t) (Color >> 11)); /* ra8875_red */
	LCD_SendRegister(LCD_FGCR1, (uint16_t) (Color >> 5)); /* ra8875_green */
	LCD_SendRegister(LCD_FGCR2, (uint16_t) (Color)); /* ra8875_blue */
}

void LCD_DrawStraightLine(ushort x, ushort y, ushort Length, uchar Direction,
		ushort color) {

	int16_t temp;

	LCD_SetForegroundColor(color);

	if (Direction == LCD_DIR_VERTICAL)

	{
		/* Horizontal start */
		temp = x;
		LCD_SendRegister(LCD_DLHSR0, temp);
		temp = x >> 8;
		LCD_SendRegister(LCD_DLHSR1, temp);

		/* Horizontal end */
		temp = x;
		LCD_SendRegister(LCD_DLHER0, temp);
		temp = x >> 8;
		LCD_SendRegister(LCD_DLHER1, temp);

		/* Vertical start */
		temp = y;
		LCD_SendRegister(LCD_DLVSR0, temp);
		temp = y >> 8;
		LCD_SendRegister(LCD_DLVSR1, temp);

		/* Vertical end */
		temp = (y + Length);
		LCD_SendRegister(LCD_DLVER0, temp);
		temp = (y + Length) >> 8;
		LCD_SendRegister(LCD_DLVER1, temp);
	} else {

		/* Horizontal start */
		temp = x;
		LCD_SendRegister(LCD_DLHSR0, temp);
		temp = x >> 8;
		LCD_SendRegister(LCD_DLHSR1, temp);

		/* Horizontal end */
		temp = (x + Length);
		LCD_SendRegister(LCD_DLHER0, temp);
		temp = (x + Length) >> 8;
		LCD_SendRegister(LCD_DLHER1, temp);

		/* Vertical start */
		temp = y;
		LCD_SendRegister(LCD_DLVSR0, temp);
		temp = y >> 8;
		LCD_SendRegister(LCD_DLVSR1, temp);

		/* Vertical end */
		temp = y;
		LCD_SendRegister(LCD_DLVER0, temp);
		temp = y >> 8;
		LCD_SendRegister(LCD_DLVER1, temp);
	}

	LCD_SendRegister(LCD_DCR, 0x80);

}

void LCD_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	int16_t temp;

	LCD_SetForegroundColor(color);

	/* Horizontal start */
	temp = x;
	LCD_SendRegister(LCD_DLHSR0, temp);
	temp = x >> 8;
	LCD_SendRegister(LCD_DLHSR1, temp);

	/* Horizontal end */
	temp = (x + w);
	LCD_SendRegister(LCD_DLHER0, temp);
	temp = (x + w) >> 8;
	LCD_SendRegister(LCD_DLHER1, temp);

	/* Vertical start */
	temp = y;
	LCD_SendRegister(LCD_DLVSR0, temp);
	temp = y >> 8;
	LCD_SendRegister(LCD_DLVSR1, temp);

	/* Vertical end */
	temp = (y + h);
	LCD_SendRegister(LCD_DLVER0, temp);
	temp = (y + h) >> 8;
	LCD_SendRegister(LCD_DLVER1, temp);

	LCD_SendRegister(LCD_DCR, 0xB0);
}



void LCD_DrawFullRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,
		ushort color) {

	int16_t temp;

	LCD_SetForegroundColor(color);

	/* Horizontal start */
	temp = Xpos;
	LCD_SendRegister(LCD_DLHSR0, temp);
	temp = Xpos >> 8;
	LCD_SendRegister(LCD_DLHSR1, temp);

	/* Horizontal end */
	temp = (Xpos + Width);
	LCD_SendRegister(LCD_DLHER0, temp);
	temp = (Xpos + Width) >> 8;
	LCD_SendRegister(LCD_DLHER1, temp);

	/* Vertical start */
	temp = Ypos;
	LCD_SendRegister(LCD_DLVSR0, temp);
	temp = Ypos >> 8;
	LCD_SendRegister(LCD_DLVSR1, temp);

	/* Vertical end */
	temp = (Ypos + Height);
	LCD_SendRegister(LCD_DLVER0, temp);
	temp = (Ypos + Height) >> 8;
	LCD_SendRegister(LCD_DLVER1, temp);

	LCD_SendRegister(LCD_DCR, 0x90);
}

void LCD_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop,
		uint16_t YBottom) {

	uint16_t temp;

	/* setting active window X */
	temp = XLeft;
	LCD_SendRegister(LCD_HSAW0, temp);
	temp = XLeft >> 8;
	LCD_SendRegister(LCD_HSAW1, temp);

	temp = XRight;
	LCD_SendRegister(LCD_HEAW0, temp);
	temp = XRight >> 8;
	LCD_SendRegister(LCD_HEAW1, temp);

	/* setting active window Y */
	temp = YTop;
	LCD_SendRegister(LCD_VSAW0, temp);
	temp = YTop >> 8;
	LCD_SendRegister(LCD_VSAW1, temp);

	temp = YBottom;
	LCD_SendRegister(LCD_VEAW0, temp);
	temp = YBottom >> 8;
	LCD_SendRegister(LCD_VEAW1, temp);

}

/* BTE - Block Transfer Engine -----------------------------------------------*/
/**
 * @brief	BTE area size settings
 * @param	Width: The width
 * @param	Height: The height
 * @retval	None
 */
void LCD_BTESize(uint16_t Width, uint16_t Height) {
	uint16_t temp;
	temp = Width;
	/* BTE Width */
	LCD_SendRegister(LCD_BEWR0, temp);
	temp = Width >> 8;
	LCD_SendRegister(LCD_BEWR1, temp);

	temp = Height;
	/* BTE Height */
	LCD_SendRegister(LCD_BEHR0, temp);
	temp = Height >> 8;
	LCD_SendRegister(LCD_BEHR1, temp);
}

/**
 * @brief	Set points for source and destination for BTE
 * @param	SourceX: X-coordinate for the source
 * @param	SourceY: Y-coordinate for the source
 * @param	DestinationX: X-coordinate for the destination
 * @param	DestinationY: Y-coordinate for the destination
 * @retval	None
 */
void LCD_BTESourceDestinationPoints(uint16_t SourceX, uint16_t SourceY,
		uint16_t DestinationX, uint16_t DestinationY) {
	uint16_t temp, temp1;

	/* Horizontal Source Point of BTE */
	temp = SourceX;
	LCD_SendRegister(LCD_HSBE0, temp);
	temp = SourceX >> 8;
	LCD_SendRegister(LCD_HSBE1, temp);

	/* Vertical Source Point of BTE */
	temp = SourceY;
	LCD_SendRegister(LCD_VSBE0, temp);
	temp = SourceY >> 8;
	LCD_SendCommand(LCD_VSBE1);
	temp1 = LCD_DataRead();
	temp1 &= 0x80; /* Get Source layer */
	temp = temp | temp1;
	LCD_SendRegister(LCD_VSBE1, temp);

	/* Horizontal Destination Point of BTE */
	temp = DestinationX;
	LCD_SendRegister(LCD_HDBE0, temp);
	temp = DestinationX >> 8;
	LCD_SendRegister(LCD_HDBE1, temp);

	/* Vertical Destination Point of BTE */
	temp = DestinationY;
	LCD_SendRegister(LCD_VDBE0, temp);
	temp = DestinationY >> 8;
	LCD_SendCommand(LCD_VDBE1);
	temp1 = LCD_DataRead();
	temp1 &= 0x80; /* Get Source layer */
	temp = temp | temp1;
	LCD_SendRegister(LCD_VDBE1, temp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PIXELBUFFERSIZE 2049
#define PIXELBUFFERCOUNT 1
static uint16_t pixelbuffer[PIXELBUFFERCOUNT][PIXELBUFFERSIZE];
static uint16_t pixelcount = 0;
static uint16_t pixelbufidx = 0;

static inline void LCD_BulkPixel_BufferInit() {
	pixelbufidx = (pixelbufidx + 1) % PIXELBUFFERCOUNT;
	pixelcount = 0;
}

static inline void LCD_BulkPixel_BufferFlush() {
	LCD_BulkWrite(pixelbuffer[pixelbufidx], pixelcount);
	LCD_BulkPixel_BufferInit();
}

inline void LCD_BulkPixel_Put(uint16_t pixel) {
	pixelbuffer[pixelbufidx][pixelcount++] = pixel;
	if (pixelcount == PIXELBUFFERSIZE) {
		LCD_BulkPixel_BufferFlush();
	}
}

inline void LCD_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height) {

	LCD_SetActiveWindow(x, x + width - 1, y, y + height - 1);
	LCD_SetCursorA(x, y);
	LCD_SendCommand(LCD_MRWC);
}

inline void LCD_BulkWrite(uint16_t* pixel, uint32_t len) {

	LCD_TransferDataDMA((uint16_t*) pixel, len);
}

inline void LCD_CloseBulkWrite(void) {

	while (DMA_STREAM->CR & DMA_SxCR_EN) { asm(""); }
	LCD_SetActiveWindow(0, 799, 0, 479);
	LCD_SendRegister(LCD_MWCR0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LCD_DrawChar(ushort x, ushort y, char symb, ushort Color, ushort bkColor,
		const sFONT *cf) {
	ulong i, j;
	ushort a, b;
	const uchar fw = cf->Width;
	const short *ch = (const short *) (&cf->table[(symb - 32) * cf->Height]);

	LCD_OpenBulkWrite(x, cf->Width, y, cf->Height);
	LCD_BulkPixel_BufferInit();

	for (i = 0; i < cf->Height; i++) {
		// Draw line
		for (j = 0; j < cf->Width; j++) {
			a = ((ch[i] & ((0x80 << ((fw / 12) * 8)) >> j)));
			b = ((ch[i] & (0x01 << j)));
			//
			LCD_BulkPixel_Put(
					((!a && (fw <= 12)) || (!b && (fw > 12))) ?
							bkColor : Color);
		}
	}
	LCD_BulkPixel_BufferFlush();
	// flush all not yet  transferred pixel to display.
	LCD_CloseBulkWrite();
}

void LCD_DrawCharSegment(ushort x, ushort y, char symb, ushort Color,
		ushort bkColor, const sFONT32 *cf) {

	ulong i, j;
	uint a, b;
	const uchar fw = cf->Width;
	const uint *ch = (const uint *) (&cf->table[(symb - 48) * cf->Height]);

	LCD_OpenBulkWrite(x, cf->Width, y, cf->Height);
	LCD_BulkPixel_BufferInit();

	for (i = 0; i < cf->Height; i++) {
		// Draw line
		for (j = 0; j < cf->Width; j++) {

			a = ch[i] & (0x80000000 >> j);
			//b = ((ch[i] & (0x01 << j)));

			LCD_BulkPixel_Put((!a) ? bkColor : Color);
		}
	}
	LCD_BulkPixel_BufferFlush();
	// flush all not yet  transferred pixel to display.
	LCD_CloseBulkWrite();

}

//void LCD_DrawCharSegment(ushort x, ushort y, char symb, ushort Color,
//		ushort bkColor, const sFONT32 *cf) {
//	ulong i, j;
//	ushort x_addr = x;
//	uint a, b;
//	uchar fw = cf->Width;
//	const uint *ch = (const uint *) (&cf->table[(symb - 48) * cf->Height]);
//
//	LCD_SetCursorA(x, y);
//
//	for (i = 0; i < cf->Height; i++) {
//		// Draw line
//		for (j = 0; j < cf->Width; j++) {
//			LCD_SendCommand(LCD_MRWC);
////			   a = ch[i] & (uint)(0x80000000 >> j);
//			a = ch[i] & (0x80000000 >> j);
//			b = ((ch[i] & (0x01 << j)));
//
//			if (!a)
//				LCD_SendData(bkColor);
//			else
//				LCD_SendData(Color);
//
//			x_addr++;
//			LCD_SetCursorA(x_addr, y);
//		}
//
//		y++;
//		x_addr = x;
//		LCD_SetCursorA(x_addr, y);
//	}
//
//}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LCD_setRotation(uint8_t m) {

}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {

	LCD_SetCursorA(x, y);
	LCD_SendCommand(LCD_MRWC);
	LCD_SendData(color);

}

void LCD_ClearScreen(uint16_t Color) {
	LCD_SendRegister(LCD_BGCR0, (uint16_t) (Color >> 11)); /* ra8875_red */
	LCD_SendRegister(LCD_BGCR1, (uint16_t) (Color >> 5)); /* ra8875_green */
	LCD_SendRegister(LCD_BGCR2, (uint16_t) (Color)); /* ra8875_blue */
}

void LCD_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,
		ushort color) {
	int16_t temp;

	LCD_SetForegroundColor(color);

	/* Horizontal start */
	temp = Xpos;
	LCD_SendRegister(LCD_DLHSR0, temp);
	temp = Xpos >> 8;
	LCD_SendRegister(LCD_DLHSR1, temp);

	/* Horizontal end */
	temp = (Xpos + Width);
	LCD_SendRegister(LCD_DLHER0, temp);
	temp = (Xpos + Width) >> 8;
	LCD_SendRegister(LCD_DLHER1, temp);

	/* Vertical start */
	temp = Ypos;
	LCD_SendRegister(LCD_DLVSR0, temp);
	temp = Ypos >> 8;
	LCD_SendRegister(LCD_DLVSR1, temp);

	/* Vertical end */
	temp = (Ypos + Height);
	LCD_SendRegister(LCD_DLVER0, temp);
	temp = (Ypos + Height) >> 8;
	LCD_SendRegister(LCD_DLVER1, temp);

	LCD_SendRegister(LCD_DCR, 0x90);
}

void LCD_DrawHorizLineWithGrad(ushort x, ushort y, ushort Length,
		ushort gradient_start) {
	uint32_t i = 0, j = 0;
	ushort k = gradient_start;

	for (i = 0; i < Length; i++) {
		LCD_SetCursorA(x, y);

		LCD_SendCommand(LCD_CGSR);
		LCD_SendData(RGB(k, k, k));

		j++;
		if (j == GRADIENT_STEP) {
			if (i < (Length / 2))
				k += (GRADIENT_STEP / 2);
			else
				k -= (GRADIENT_STEP / 2);

			j = 0;
		}

		x++;
	}
}

void LCD_PrintText(ushort Xpos, ushort Ypos, char *str, ushort Color,
		ushort bkColor, uchar font) {
	uint8_t TempChar;
	const sFONT *cf;

	switch (font) {
	case 1:
		cf = &GL_Font16x24;
		break;
	case 2:
		cf = &GL_Font12x12;
		break;
	case 3:
		cf = &GL_Font8x12;
		break;
	case 4:
		cf = &GL_Font8x8;
		break;
	case 5:
		cf = &GL_SevenSegNum;
		break;
	case 6:
		cf = &GL_Big;
		break;
	default:
		cf = &GL_Font8x12_bold;
		break;
	}

	do {
		TempChar = *str++;

		if (font >= 5)
			LCD_DrawCharSegment(Xpos, Ypos, TempChar, Color, bkColor, cf);
		else
			LCD_DrawChar(Xpos, Ypos, TempChar, Color, bkColor, cf);

		if (Xpos < (MAX_X - cf->Width)) {
			Xpos += cf->Width;

			// Mod the 8x8 font - the shift is too big
			// because of the letters
			if (font == 4) {
				if (*str > 0x39)
					Xpos -= 1;
				else
					Xpos -= 2;
			}
		} else if (Ypos < (MAX_Y - cf->Height)) {
			Xpos = 0;
			Ypos += cf->Height;
		} else {
			Xpos = 0;
			Ypos = 0;
		}

	} while (*str != 0);
}

const sFONT *LCD_Font(uint8_t font) {
	const sFONT *cf;

	if (font > 4) {
		cf = fontList[0];
	} else {
		cf = fontList[font];
	}

	return cf;
}

uint16_t LCD_TextHeight(uint8_t font) {

	const sFONT *cf = LCD_Font(font);
	return cf->Height;
}

uint16_t LCD_TextWidth(const char *str, uchar font) {

	uint16_t Xpos = 0;

	const sFONT *cf = LCD_Font(font);
	if (str != NULL) {
		while (*str != 0) {
			Xpos += cf->Width;
			if (font == 4) {
				if (*str > 0x39)
					Xpos -= 1;
				else
					Xpos -= 2;
			}
			str++;
		}
	}
	return Xpos;
}

void LCD_PrintTextRight(ushort Xpos, ushort Ypos, char *str, ushort Color,
		ushort bkColor, uchar font) {

	uint16_t Xwidth = LCD_TextWidth(str, font);
	if (Xpos < Xwidth) {
		Xpos = 0; // TODO: Overflow is not handled too good, just start at beginning of line and draw over the end.
	} else {
		Xpos -= Xwidth;
	}
	LCD_PrintText(Xpos, Ypos, str, Color, bkColor, font);
}

// This version of "Draw Spectrum" is revised from the original in that it interleaves the erasure with the drawing
// of the spectrum to minimize visible flickering  (KA7OEI, 20140916, adapted from original)
//
// 20141004 NOTE:  This has been somewhat optimized to prevent drawing vertical line segments that would need to be re-drawn:
//  - New lines that were shorter than old ones are NOT erased
//  - Line segments that are to be erased are only erased starting at the position of the new line segment.
//
//  This should reduce the amount of CGRAM access - especially via SPI mode - to a minimum.
//

static inline bool UiSpectrum_Draw_IsVgrid(const uint16_t x,
		const uint16_t color_new, uint16_t* clr_ptr,
		const uint16_t x_center_line) {
	bool repaint_v_grid = false;

	if (x == x_center_line) {
		*clr_ptr = ts.scope_centre_grid_colour_active;
		repaint_v_grid = true;
	} else {
		int k;
		// Enumerate all saved x positions
		for (k = 0; k < 7; k++) {
			// Exit on match
			if (x == sd.vert_grid_id[k]) {
				*clr_ptr = ts.scope_grid_colour_active;
				repaint_v_grid = true;
				break;
				// leave loop, found match
			}
		}
	}
	return repaint_v_grid;
}

static uint16_t UiSpectrum_Draw_GetCenterLineX() {
	static uint16_t idx;

	static const uint16_t center[FREQ_IQ_CONV_MODE_MAX + 1] = { 3, 2, 4, 1, 5 };
	// list the idx for the different modes (which are numbered from 0)
	// the list static const in order to have it in flash
	// it would be faster in ram but this is not necessary


		idx = center[ts.iq_freq_mode];

	return sd.vert_grid_id[idx];
}

void UiSpectrumDrawSpectrum(q15_t *fft_old, q15_t *fft_new,
		const ushort color_old, const ushort color_new, const ushort shift) {

#define SPEC_LIGHT_MORE_POINTS 0
#define LIGHT	true
//#define RA8875_SPECTRUM_HEIGHT 300
#define SPECTRUM_START_X 0
#define SPECTRUM_START_Y 50
#define POS_SPECTRUM_IND_X 0

	int spec_height = RA8875_SPECTRUM_HEIGHT + SPEC_LIGHT_MORE_POINTS + 100; //x
	int spec_start_y = SPECTRUM_START_Y + SPEC_LIGHT_MORE_POINTS;

	//static uint16_t pixel_buf[RA8875_SPECTRUM_HEIGHT + SPEC_LIGHT_MORE_POINTS];

	uint16_t i, k, x, y_old, y_new, y1_old, y1_new, len_old, sh, clr;
	uint16_t y1_new_minus = 0;
	uint16_t y1_old_minus = 0;
	uint16_t idx = 0;

	bool repaint_v_grid = false;
	clr = color_new;

	uint16_t x_center_line = UiSpectrum_Draw_GetCenterLineX();

	if (shift)
		sh = (SPECTRUM_WIDTH / 2) + 1;   // Shift to fill gap in center
	else
		sh = 1;                  // Shift to fill gap in center

	if (sd.first_run > 0) {
		idx = 0;
		for (x = (SPECTRUM_START_X + sh + 0);
				x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH / 2 + sh); x++) {
			y_new = fft_new[idx++];

			if (y_new > (spec_height - 7))
				y_new = (spec_height - 7);
			y1_new = (spec_start_y + spec_height - 1) - y_new;
			if (!(LIGHT)) {
				LCD_DrawStraightLine(x, y1_new, y_new, LCD_DIR_VERTICAL,
						color_new);
			}
		}
		sd.first_run--;
	} else {

		idx = 0;

		for (x = (SPECTRUM_START_X + sh + 0);
				x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH / 2 + sh); x++) {

			if (LIGHT)

			{
				if ((idx > 1) && (idx < 399)) //
						{
					// moving window - weighted average of 5 points of the spectrum to smooth spectrum in the frequency domain
					// weights:  x: 50% , x-1/x+1: 36%, x+2/x-2: 14%
					y_old = (fft_old[idx] * 0.5 + fft_old[idx - 1] * 0.18
							+ fft_old[idx - 2] * 0.07 + fft_old[idx + 1] * 0.18
							+ fft_old[idx + 2] * 0.07);
					y_new = (fft_new[idx] * 0.5 + fft_new[idx - 1] * 0.18
							+ fft_new[idx - 2] * 0.07 + fft_new[idx + 1] * 0.18
							+ fft_new[idx + 2] * 0.07);
				} else {
					y_old = fft_old[idx];
					y_new = fft_new[idx];
				}

				idx++;

			} else {
				y_old = *fft_old++;
				y_new = *fft_new++;
			}

			// Limit vertical
			if (y_old > (spec_height - 7))
				y_old = (spec_height - 7);

			if (y_new > (spec_height - 7))
				y_new = (spec_height - 7);

			// Data to y position and length
			y1_old = (spec_start_y + spec_height - 1) - y_old;
			len_old = y_old;

			y1_new = (spec_start_y + spec_height - 1) - y_new;

			//            if (y1_old != y1_new && (true) && x != (POS_SPECTRUM_IND_X + 32*ts.c_line + 1))

//			if (x == SPECTRUM_START_X + 1) // special case of first x position of spectrum
//					{
//				y1_old_minus = y1_old;
//				y1_new_minus = y1_new;
//			}
//
//			if (x == SPECTRUM_START_X + (SPECTRUM_WIDTH / 2) + 1) // special case of first line of right part of spectrum
//					{
//				y1_old_minus = (spec_start_y + spec_height - 1)
//						- sd.FFT_BkpData[255];
//				y1_new_minus = (spec_start_y + spec_height - 1)
//						- sd.FFT_DspData[255];
//			}


			if (x >=  399 && x<= 401) // special case of first x position of spectrum
					{
				y1_old_minus = y1_old+2;
				y1_new_minus = y1_new+2;
			}

			if ((LIGHT) && x != (POS_SPECTRUM_IND_X + 32 * ts.c_line + 1)) {
				// x position is not on vertical centre line (the one that indicates the receive frequency)

				// here I would like to draw a line if y1_new and the last drawn pixel (y1_new_minus) are more than 1 pixel apart in the vertical axis
				// makes the spectrum display look more complete . . .
				//

				if (y1_old - y1_old_minus > 1) // && x !=(SPECTRUM_START_X + sh))
						{ // plot line upwards
					LCD_DrawStraightLine(x, y1_old_minus + 1,
							y1_old - y1_old_minus, LCD_DIR_VERTICAL, color_old);
				} else if (y1_old - y1_old_minus < -1) // && x !=(SPECTRUM_START_X + sh))
						{ // plot line downwards
					LCD_DrawStraightLine(x, y1_old,
							y1_old_minus - y1_old, LCD_DIR_VERTICAL, color_old);
				} else {
					LCD_DrawColorPoint(x, y1_old, color_old);
				}

				if (y1_new - y1_new_minus > 1) // && x !=(SPECTRUM_START_X + sh))
						{ // plot line upwards
					LCD_DrawStraightLine(x, y1_new_minus + 1,
							y1_new - y1_new_minus, LCD_DIR_VERTICAL, color_new);

				} else if (y1_new - y1_new_minus < -1) // && x !=(SPECTRUM_START_X + sh))
						{ // plot line downwards
					LCD_DrawStraightLine(x, y1_new,
							y1_new_minus - y1_new, LCD_DIR_VERTICAL, color_new);

				} else {
					LCD_DrawColorPoint(x, y1_new, color_new);
				}

			}
			y1_new_minus = y1_new;
			y1_old_minus = y1_old;

			if (!(LIGHT)) {
				if (y_old <= y_new) {
					// is old line going to be overwritten by new line, anyway?
					// ----------------------------------------------------------
					//
					if (y_old != y_new) {
						LCD_DrawStraightLine(x, y1_new, y_new - y_old,
						LCD_DIR_VERTICAL, color_new);
					}
				} else {

					uint16_t spectrum_pixel_buf[RA8875_SPECTRUM_HEIGHT+SPEC_LIGHT_MORE_POINTS];

					// Repaint vertical grid on clear
					// Basically paint over the grid is allowed
					// but during spectrum clear instead of masking
					// grid lines with black - they are repainted
					// TODO: This code is  always executed, since this function is always called with color_old == Black
					repaint_v_grid = UiSpectrum_Draw_IsVgrid(x, color_new, &clr,
							x_center_line);
					//
					LCD_OpenBulkWrite(x, 1, y1_old, len_old);

					idx = 0;
					// Draw vertical line, starting only with position of where new line would be!
					for (i = y_new; i < len_old; i++) {
						// Do not check for horizontal grid when we have vertical masking
						if (!repaint_v_grid) {
							clr = color_old;

							// Are we trying to paint over horizontal grid line ?
							// prevent that by changing this points color to the grid color
							// TODO: This code is  always executed, since this function is always called with color_old == Black
							// This code does not make sense to me: it should check if the CURRENT y value (stored in i) is a horizontal
							// grid, not the y1_old.
							// Enumerate all saved y positions
//							char upperline;
//							if (ts.spectrum_size)//set range big/normal for redraw
//								upperline = 5;
//							else
//								upperline = 3;
//
//							for (k = 0; k < upperline; k++) {
//								if (y1_old == sd.horz_grid_id[k]) {
//									clr = ts.scope_grid_colour_active;
//									break;
//								}
//							}

						}

						spectrum_pixel_buf[idx++] = clr;
						// Track absolute position
						y1_old++;
					}
					LCD_BulkWrite(spectrum_pixel_buf, len_old - y_new);
					LCD_CloseBulkWrite();
					// Reset flag
					if (repaint_v_grid)
						repaint_v_grid = 0;
					LCD_DrawStraightLine(sd.vert_grid_id[ts.c_line - 1],
							(POS_SPECTRUM_IND_Y - 4),
							(POS_SPECTRUM_IND_H - 15),
							LCD_DIR_VERTICAL,
							ts.scope_centre_grid_colour_active);
				}
			}
		}
	}
}

void LCD_SetTextWritePosition(uint16_t XPos, uint16_t YPos) {
	uint16_t temp;
	temp = XPos;
	LCD_SendRegister(LCD_F_CURXL, temp);
	temp = XPos >> 8;
	LCD_SendRegister(LCD_F_CURXH, temp);

	temp = YPos;
	LCD_SendRegister(LCD_F_CURYL, temp);
	temp = YPos >> 8;
	LCD_SendRegister(LCD_F_CURYH, temp);
}

void LCD_textColor(uint16_t foreColor, uint16_t bgColor) {
	/* Set Fore Color */
	LCD_SendCommand(0x63);
	LCD_SendData((foreColor & 0xf800) >> 11);
	LCD_SendCommand(0x64);
	LCD_SendData((foreColor & 0x07e0) >> 5);
	LCD_SendCommand(0x65);
	LCD_SendData((foreColor & 0x001f));

	/* Set Background Color */
	LCD_SendCommand(0x60);
	LCD_SendData((bgColor & 0xf800) >> 11);
	LCD_SendCommand(0x61);
	LCD_SendData((bgColor & 0x07e0) >> 5);
	LCD_SendCommand(0x62);
	LCD_SendData((bgColor & 0x001f));

	/* Clear transparency flag */
	LCD_SendCommand(0x22);
	uint8_t temp = LCD_RAM;
	temp &= ~(1 << 6); // Clear bit 6
	LCD_SendData(temp);
}

void LCD_WriteString(uint8_t *String, LCD_Transparency TransparentBackground,
		LCD_FontEnlargement Enlargement) {
	/* Set to text mode with invisible cursor */
	LCD_SendRegister(LCD_MWCR0, 0x80);

	/* Set background transparency and font size */
	uint8_t fontControlValue = 0;
	if (TransparentBackground)
		fontControlValue |= 0x40;
	if (Enlargement == ENLARGE_2X)
		fontControlValue |= 0x05;
	else if (Enlargement == ENLARGE_3X)
		fontControlValue |= 0x0A;
	else if (Enlargement == ENLARGE_4X)
		fontControlValue |= 0x0F;
	LCD_SendRegister(LCD_FNCR1, fontControlValue);

	/* Write to memory */
	LCD_SendCommand(LCD_MRWC);
	while (*String != '\0') {
		LCD_SendData(*String);
		++String;
		LCD_CheckBusy();
	}

	LCD_SendRegister(LCD_MWCR0, 0x00);
}

#define VU_HEIGHT	98
#define VU_WIDE		320

#define VUPOS_X		0
#define VUPOS_Y		0

void LCD_drawVU(void) {

	LCD_OpenBulkWrite(VUPOS_X, VU_WIDE, VUPOS_Y, VU_HEIGHT);

	LCD_BulkWrite(vu2, VU_WIDE * VU_HEIGHT);

	LCD_CloseBulkWrite();

}

/**************************************************************************/
float LCD_cosDeg_helper(float angle) {
	float radians = angle / (float) 360 * 2 * PI;
	return cos(radians);
}

float LCD_sinDeg_helper(float angle) {
	float radians = angle / (float) 360 * 2 * PI;
	return sin(radians);
}


void LCD_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, ushort color) {

	int16_t temp;

	LCD_SetForegroundColor(color);

	/* Horizontal start */
	temp = x0;
	LCD_SendRegister(LCD_DLHSR0, temp);
	temp = x0 >> 8;
	LCD_SendRegister(LCD_DLHSR1, temp);

	/* Horizontal end */
	temp = x1;
	LCD_SendRegister(LCD_DLHER0, temp);
	temp = x1 >> 8;
	LCD_SendRegister(LCD_DLHER1, temp);

	/* Vertical start */
	temp = y0;
	LCD_SendRegister(LCD_DLVSR0, temp);
	temp = y0 >> 8;
	LCD_SendRegister(LCD_DLVSR1, temp);

	/* Vertical end */
	temp = y1;
	LCD_SendRegister(LCD_DLVER0, temp);
	temp = y1 >> 8;
	LCD_SendRegister(LCD_DLVER1, temp);

	LCD_SendRegister(LCD_DCR, 0x80);
}



void LCD_drawLineAngle(int16_t x, int16_t y, int16_t angle, uint16_t length,
		uint16_t color, int offset) {

	if (length < 2) {
		LCD_DrawPixel(x, y, color);
	} else {
		length--; //n
		LCD_drawLine(x, y,
				x + round((length) * LCD_cosDeg_helper(angle + offset)),
				y + round((length) * LCD_sinDeg_helper(angle + offset)), color);
	}
}

uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min,
		uint16_t out_max) {

	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#define PASOS 3
static uint8_t rebote;
static uint16_t valor = 0, angulo = 270;
static uint8_t sentido = 0;

void LCD_AnalogMeter(int8_t new, int8_t old)

{

	uint16_t val_temp;
	char buf[10];

	LCD_SetActiveWindow( VUPOS_X, VUPOS_X + VU_WIDE - 1, VUPOS_Y,
	VUPOS_Y + VU_HEIGHT - 1);
	LCD_SetCursorA(VUPOS_X, VUPOS_Y);

	if (!sentido && (new > valor)) {
		valor = new;
		sentido = 1;
		rebote = PASOS;

	}

	if (!sentido && (new < valor)) {
		valor = new;
		sentido = 2;
		rebote = PASOS;

	}

	if (new > valor)
		valor = new;

	val_temp = map(valor, 1, 34, 238, 302);

	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 2, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, 0, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 1, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, 0, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, 0, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 1, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, 0, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 2, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, 0, 0);

	if (sentido == 1 && angulo > val_temp)
		angulo--;

	if (sentido == 2 && angulo < val_temp)
		angulo++;

	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 2, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 1, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 1, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
	LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 2, VUPOS_Y + 250, angulo,
			195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);

	if (angulo == valor) {
		rebote = 0;
	}

	if (rebote)
		rebote--;
	else {


		sentido = 0;
		rebote = PASOS;

		if (angulo > 238 && valor != new) {

			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 2, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, 0, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 1, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, 0, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, 0, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 1, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, 0, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 2, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, 0, 0);
			angulo--;
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 2, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 - 1, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 1, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);
			LCD_drawLineAngle(VUPOS_X + VU_WIDE / 2 + 2, VUPOS_Y + 250, angulo,
					195 + abs(angulo - 270) / 2, LCD_COLOR_ra8875_red, 0);

			val_temp = map(new, 1, 25, 1, 12);


			if (val_temp > 9) {
				if (val_temp == 10)
					LCD_PrintText((362), (90), "+20dB", Orange, Black, 1);
				else if (val_temp == 11)
					LCD_PrintText((362), (90), "+40dB", Orange, Black, 1);
				else if (val_temp == 12)
					LCD_PrintText((362), (90), "+60dB", Orange, Black, 1);
				val_temp = 9;
			} else
				LCD_PrintText((362), (90),"   ", Orange, Black, 1);

			sprintf(buf, "S%d", (char) val_temp);// Display auto RFG for debug
			LCD_PrintText((350), (17), buf, Orange, Black, 6);
		}
	}

	LCD_SetActiveWindow(0, 799, 0, 479);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LCD_PrintTextCentered(const uint16_t bbX, const uint16_t bbY,
		const uint16_t bbW, const char* txt, uint32_t clr_fg, uint32_t clr_bg,
		uint8_t font) {
	const uint16_t bbH = LCD_TextHeight(0);
	const uint16_t txtW = LCD_TextWidth(txt, 0);
	const uint16_t bbOffset = txtW > bbW ? 0 : ((bbW - txtW) + 1) / 2;

	// we draw the part of  the box not used by text.
	LCD_DrawFullRect(bbX, bbY, bbH, bbOffset, clr_bg);
	LCD_PrintText((bbX + bbOffset), bbY, txt, clr_fg, clr_bg, 0);

	// if the text is smaller than the box, we need to draw the end part of the
	// box
	if (txtW < bbW) {
		LCD_DrawFullRect(bbX + txtW + bbOffset, bbY, bbH,
				bbW - (bbOffset + txtW), clr_bg);
	}
}

#pragma GCC optimize("O0")
//*----------------------------------------------------------------------------
//* Function Name       : LCD_Delay
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void LCD_Delay(ulong delay) {
	ulong i, k;

	for (k = 0; (k < delay); k++)
		for (i = 0; (i < US_DELAY); i++) {
			asm("");
		}
}

