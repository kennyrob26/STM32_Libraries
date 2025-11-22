/*
 * AHT_LIBRARY_STM32.h
 *
 *  Created on: Nov 14, 2025
 *      Author: kenny
 */

#ifndef INC_SSD_LIBRARY_STM32_H_
#define INC_SSD_LIBRARY_STM32_H_

#include "stm32g0xx_hal.h"

#define DISPLAY_READ_CODE  0x79
#define DISPLAY_WRITE_CODE 0x78

#define DISPLAY_LINES    64
#define DISPLAY_COLUMNS 128
#define DISPLAY_PAGES     8
#define DISPLAY_BLOCKS   16

typedef union
{
	uint8_t all_pixels[DISPLAY_LINES * DISPLAY_COLUMNS];
	uint8_t page[DISPLAY_PAGES][DISPLAY_COLUMNS];
	uint8_t block[DISPLAY_PAGES][DISPLAY_BLOCKS][8];
}SSD1306_DISPLAY_FRAMES;


typedef enum
{
	COLOR_BLACK = 0,
	COLOR_WHITE = 1
}SSD1306_COLOR;

typedef enum
{
	ADRESSING_MODE_HORIZONTAL = 0x00,
	ADRESSING_MODE_VERTICAL   = 0x01,
	ADRESSING_MODE_PAGE       = 0x02,
	ADRESSING_MODE_INVALID    = 0x03
}SSD1306_ADRESSING_MODE;

typedef enum
{
	PAGE_0 = 0,
	PAGE_1 = 1,
	PAGE_2 = 2,
	PAGE_3 = 3,
	PAGE_4 = 4,
	PAGE_5 = 5,
	PAGE_6 = 6,
	PAGE_7 = 7
}SSD1306_PAGE;


void SSD1306_Init();
void SSD1306_setAdressingMode(SSD1306_ADRESSING_MODE mode);
void SSD1306_SetPage(SSD1306_PAGE startAddress, SSD1306_PAGE endAddress);
void SSD1306_SetColumn(uint8_t startAddress, uint8_t endAddress);
void SSD1306_ClearDisplay(SSD1306_COLOR color);
void SSD1306_PAGE_setColumn(uint8_t column);
void SSD1306_PAGE_setPage(SSD1306_PAGE page);
void SSD1306_PAGE_setPage(SSD1306_PAGE page);

void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
void SSD1306_DrawLineHorizontal(uint8_t x1, uint8_t x2, uint8_t y, SSD1306_COLOR color);
void SSD1306_DrawLineVertical(uint8_t x, uint8_t y1,uint8_t y2, SSD1306_COLOR color);
void SSD1306_DrawLine(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, SSD1306_COLOR color);
void SSD1306_UpdatePage(SSD1306_PAGE page, uint8_t page_pixels[128]);
void SSD1306_UpdateDisplay();



#endif /* INC_SSD_LIBRARY_STM32_H_ */
