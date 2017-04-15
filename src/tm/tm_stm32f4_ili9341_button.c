/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32f4_ili9341_button.h"
#include "gsl1680.h"
#include "ui_ra8875.h"
#include "boton.h"

extern struct _ts_event ts_event;

LCD_Button_t LCD_Buttons[LCD_BUTTON_MAX_BUTTONS];

int8_t LCD_Button_Add(LCD_Button_t* button) {
	uint8_t id = 0;
	while ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_USED) && (id < LCD_BUTTON_MAX_BUTTONS)) {
		id++;
	}

	if (id == LCD_BUTTON_MAX_BUTTONS) {
		//Max button reached
		return -1;
	}
	LCD_Buttons[id].x = button->x;
	LCD_Buttons[id].y = button->y;
	LCD_Buttons[id].width = button->width;
	LCD_Buttons[id].height = button->height;
	LCD_Buttons[id].background = button->background;
	LCD_Buttons[id].borderColor = button->borderColor;
	LCD_Buttons[id].flags = button->flags | TM_BUTTON_FLAG_USED | TM_BUTTON_FLAG_ENABLED;
	LCD_Buttons[id].label = button->label;
	LCD_Buttons[id].color = button->color;
	LCD_Buttons[id].font = button->font;
	LCD_Buttons[id].image = button->image;
	LCD_Buttons[id].led = button->led;

	return id;
}

void LCD_Button_DrawAll(void) {
	uint8_t id = 0;
	for (id = 0; id < LCD_BUTTON_MAX_BUTTONS; id++) {
		if ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_USED)) {
			//Button enabled, draw it to screen
			LCD_Button_Draw(id);
		}
	}
}

ErrorStatus LCD_Button_Draw(uint8_t id) {

	uint16_t x, y, i, j;

	if ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_USED) == 0) {
		//Button not enabled
		return ERROR;
	}
	//Draw label
	if (LCD_Buttons[id].flags & TM_BUTTON_FLAG_IMAGE) {
		//Draw picture
//		LCD_OpenBulkWrite(LCD_Buttons[id].x, LCD_Buttons[id].width, LCD_Buttons[id].y,LCD_Buttons[id].height);
//		LCD_BulkWrite(LCD_Buttons[id].image,LCD_Buttons[id].width*LCD_Buttons[id].height);
//		LCD_CloseBulkWrite();

	} else {
		//Background
		LCD_fillRect(					LCD_Buttons[id].x,
										LCD_Buttons[id].y,
										LCD_Buttons[id].width,
										LCD_Buttons[id].height,
										LCD_Buttons[id].background );

	}

	//Border
	if ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_NOBORDER) == 0) {
		//Border enabled
		LCD_DrawEmptyRect(				LCD_Buttons[id].x,
										LCD_Buttons[id].y,
										LCD_Buttons[id].height,
										LCD_Buttons[id].width,
										LCD_Buttons[id].borderColor );
	}

	//Display label
	if ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_NOLABEL) == 0) {

		uint16_t offset;

	    const uint16_t bbH = LCD_TextHeight(1);
	    const uint16_t txtW = LCD_TextWidth(LCD_Buttons[id].label,1);
	    const uint16_t bbOffset = txtW>LCD_Buttons[id].width?0:((LCD_Buttons[id].width - txtW)+1)/2;

	    if (LCD_Buttons[id].flags & TM_BUTTON_FLAG_LED)
	    	offset=5;
	    else
	    	offset=0;

		    LCD_PrintText((LCD_Buttons[id].x + bbOffset),LCD_Buttons[id].y + LCD_Buttons[id].height / 2-bbH/2 -offset,LCD_Buttons[id].label,LCD_Buttons[id].color,LCD_Buttons[id].background,1);


	}

	//Display label
	if ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_LED)) {

		int col;

		if ( LCD_Buttons[id].led )
			col=Green;
		else
			col=Black;

		//Draws LED area
		LCD_fillRect(		LCD_Buttons[id].x+10,
							LCD_Buttons[id].y+LCD_Buttons[id].height-10,
							LCD_Buttons[id].width-20,
							5,
							col );
	}


	return SUCCESS;
}

int8_t LCD_Button_Touch(void) {

	uint8_t id;
	for (id = 0; id < LCD_BUTTON_MAX_BUTTONS; id++) {
		//If button not enabled, ignore it
		if ((LCD_Buttons[id].flags & TM_BUTTON_FLAG_ENABLED) == 0) {
			continue;
		}
		//If touch data is inside button somewhere
		if (
			(ts_event.coords[0].x > LCD_Buttons[id].x && ts_event.coords[0].x < (LCD_Buttons[id].x + LCD_Buttons[id].width)) &&
			(ts_event.coords[0].y > LCD_Buttons[id].y && ts_event.coords[0].y < (LCD_Buttons[id].y + LCD_Buttons[id].height))
		) {
			//Return its id
			return id;
		}
	}

	//No one was pressed
	return -1;
}


void LCD_Button_Enable(uint8_t id) {
	//Add enabled flag
	LCD_Buttons[id].flags |= TM_BUTTON_FLAG_ENABLED;
}

void LCD_Button_Disable(uint8_t id) {
	//Remove enabled flag
	LCD_Buttons[id].flags &= ~TM_BUTTON_FLAG_ENABLED;
}

void LCD_Button_DeleteAll(void) {
	uint8_t i;
	for (i = 0; i < LCD_BUTTON_MAX_BUTTONS; i++) {
		LCD_Button_Delete(i);
	}
}

void LCD_Button_Delete(uint8_t id) {
	//Just remove USED flag from button
	LCD_Buttons[id].flags &= ~TM_BUTTON_FLAG_USED;
}

