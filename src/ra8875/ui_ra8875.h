#ifndef __UI_RA8875_H
#define __UI_RA8875_H

#include "ui.h"
#include "radio.h"
#include "lcd_ra8875_registers.h"

//define X and Y sizes

#define MAX_X  800
#define MAX_Y  480

//write matrix[x][y] = val;
#define SDRAM_16_set(x, y, val) TM_SDRAM_Write16( (y * MAX_Y + x)*2 , val)

//read: val = matrix[x][y];
#define SDRAM_16_get(x, y) TM_SDRAM_Read16((y * MAX_Y + x)*2 )


// DMA configuracion

#define DMA_STREAM               DMA2_Stream7
#define DMA_CHANNEL              DMA_Channel_0
#define DMA_STREAM_CLOCK         RCC_AHB1Periph_DMA2
#define DMA_STREAM_IRQ           DMA2_Stream7_IRQn
#define DMA_STREAM_STATUS_BIT	 DMA_IT_TCIF7

// Maximum DMA timeout value
#define TIMEOUT_MAX              10000

// Color definitions

#define LCD_COLOR_BROWN		(0x40C0)
#define LCD_COLOR_BLACK		(0x0000)
#define LCD_COLOR_WHITE		(0xFFFF)

#define LCD_COLOR_ra8875_red		(0xF800)
#define LCD_COLOR_ra8875_green		(0x07E0)
#define LCD_COLOR_ra8875_blue		(0x001F)
#define LCD_COLOR_YELLOW	(LCD_COLOR_ra8875_red | LCD_COLOR_ra8875_green)
#define LCD_COLOR_CYAN		(LCD_COLOR_ra8875_green | LCD_COLOR_ra8875_blue)
#define LCD_COLOR_MAGENTA	(LCD_COLOR_ra8875_red | LCD_COLOR_ra8875_blue)
#define LCD_COLOR_PURPLE	(LCD_COLOR_ra8875_red | LCD_COLOR_ra8875_blue)

#define LCD_SQUARE			1
#define LCD_LINE			2

#define RA8875_CS_PORT       	GPIOA
#define RA8875_CS_PIN        	GPIO_Pin_3

#define RA8875_SCK_PORT      	GPIOA
#define RA8875_LCD_SCK       	GPIO_Pin_5
#define RA8875_SCK_SOURCE		GPIO_PinSource5

#define RA8875_MOSI_PORT     	GPIOA
#define RA8875_LCD_MOSI      	GPIO_Pin_7
#define RA8875_MOSI_SOURCE		GPIO_PinSource7

#define RA8875_MISO_PORT     	GPIOA
#define RA8875_LCD_MISO      	GPIO_Pin_6
#define RA8875_MISO_SOURCE		GPIO_PinSource6

#define RA8875_RST_PORT			GPIOC
#define RA8875_RST_PIN			GPIO_Pin_5

#define RA8875_RST_SET			GPIO_SetBits(RA8875_RST_PORT, RA8875_RST_PIN)
#define RA8875_RST_RESET		GPIO_ResetBits(RA8875_RST_PORT, RA8875_RST_PIN)
#define RA8875_CS_SET			GPIO_SetBits(RA8875_CS_PORT, RA8875_CS_PIN)
#define RA8875_CS_RESET			GPIO_ResetBits(RA8875_CS_PORT, RA8875_CS_PIN)

#define RA8875_WAIT_PORT		GPIOC
#define RA8875_WAIT_PIN			GPIO_Pin_4

// Colors definitions, go to http://www.color-hex.com/
// choose a new one and declare here
//
#define White          		0xFFFF
#define Black          		0x0000
#define Grey           		RGB(0xb8,0xbc,0xa8)	//=0xBDF5
#define Blue           		RGB(0x50,0x50,0xFF)	// Brighter Blue	//Original - 0x001F =#0000FF
#define Blue2          		0x051F				// =#00A0FF
#define Red            		RGB(0xFF,0x38,0x38)	// Brighter (easier to see) Red	//Original 0xF800 =#FF0000
#define	Red2				RGB(0xFF,0x80,0x80)	// Even "brighter" red (almost pink)
#define Red3				RGB(0xFF,0xC0,0xC0) // A sort of pink-ish, pale red
#define Magenta        		RGB(0xFF,0x30,0xFF)	// Brighter Magenta	//Original 0xF81F =#FF00FF
#define Green          		0x07E0				// =#00FF00
#define Cyan           		0x7FFF
#define Yellow         		RGB(0xFF,0xFF,0x20)	// "Yellower" and brighter	//Original - 0xFFE0 =#FFFF00

#define Orange				RGB(0xFF,0xA8,0x20)	//"Orange-er" and brighter	//Original - RGB(0xF6,0xA0,0x1A)
#define Cream				RGB(0xED,0xE7,0xD7)

#define Grey1				RGB(0x80,0x80,0x80)
#define Grey2				RGB(0xC0,0xC0,0xC0)
#define Grey3				RGB(0xA6,0xA8,0xAD)
#define Grey4				RGB(0x40,0x40,0x40)
#define	Grey6				RGB(0x78,0x78,0x78)
//
#define	RX_Grey				RGB(0xb8,0xdb,0xa8)	// slightly green grey
#define TX_Grey				RGB(0xe8,0xad,0xa0)	// slightly red(ish) grey (more magenta, actually...)

#define	Grey_Recovery		RGB(30,30,30);

#define Segment_color		RGB(253,246,23)

// Dark grey colour used for spectrum scope grid
#define Grid				RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)		// COL_SPECTRUM_GRAD = 0x40

#define LCD_DIR_HORIZONTAL	0x0001
#define LCD_DIR_VERTICAL	0x0000

#define GRADIENT_STEP			8

#define RGB(red,green,blue)(uint16_t)(((red>>3)<<11)|((green>>2)<<5)|(blue>> 3))

// exports

typedef int16_t q15_t;

/* Typedefs ------------------------------------------------------------------*/
typedef enum {
	ENLARGE_1X = 0, ENLARGE_2X, ENLARGE_3X, ENLARGE_4X
} LCD_FontEnlargement;

typedef enum {
	NOT_TRANSPARENT = 0, TRANSPARENT
} LCD_Transparency;

typedef enum {
	ELLIPSE = 0, CIRCLE, SQUARE, LINE, TRIANGLE
} LCD_DrawType;

typedef enum {
	NOT_FILLED = 0, FILLED
} LCD_Fill;

typedef struct {
	uint16_t hue;
	float saturation;
	float brightness;
} HSB_TypeDef;

typedef struct {
	uint8_t ra8875_red; /* 5-bits used for ra8875_red */
	uint8_t ra8875_green; /* 6-bits used for ra8875_green */
	uint8_t ra8875_blue; /* 5-bits used for ra8875_blue */
} RGB565_TypeDef;

#define LCD_RAM       (*((volatile unsigned short *) 0x60000000))
#define LCD_REG       (*((volatile unsigned short *) 0x60020000))

// functions

void LCD_ParallelInit(void);
void LCD_FSMCConfig(void);
void LCD_InitLCD(void);
void LCD_SpiInit(void);
void LCD_LcdClear(ushort Color);
void LCD_DrawSpectrum_Interleaved(q15_t *fft_old, q15_t *fft_new,
		ushort color_old, ushort color_new, ushort shift);
void LCD_DrawStraightLine(ushort x, ushort y, ushort Length, uchar Direction,
		ushort color);
void LCD_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,
		ushort color);
void LCD_DrawHorizLineWithGrad(ushort x, ushort y, ushort Length,
		ushort gradient_start);
void LCD_DrawFullRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,
		ushort color);
void LCD_PrintText(ushort Xpos, ushort Ypos, char *str, ushort Color,
		ushort bkColor, uchar font);
void LCD_PrintTextCentered(const uint16_t bbX, const uint16_t bbY,
		const uint16_t bbW, const char* txt, uint32_t clr_fg, uint32_t clr_bg,
		uint8_t font);
void LCD_DrawBottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,
		ushort color);
void LCD_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height);
void LCD_BulkWrite(uint16_t* pixel, uint32_t len);
void LCD_CloseBulkWrite(void);
void LCD_setRotation(uint8_t m);
void LCD_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_ClearScreen(uint16_t Color);
void LCD_DrawSpectrum(q15_t *fft, ushort color, ushort shift);
void LCD_UiSpectrumDrawSpectrum(q15_t *fft_old, q15_t *fft_new,
		const ushort color_old, const ushort color_new, const ushort shift,
		const ushort impar);
void LCD_DrawStraightLine_spectrum(ushort x, ushort y, ushort Length,
		uchar Direction, ushort color);
void LCD_waterfall(uint16_t line);
void LCD_AnalogMeter(int8_t new, int8_t old);
void UiSpectrumDrawSpectrum(q15_t *fft_old, q15_t *fft_new,
		const ushort color_old, const ushort color_new, const ushort shift);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void LCD_drawVU(void);


void LCD_PrepareDMA(void);
void LCD_TransferDataDMA(void *buffer, uint32_t count);



#endif
