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




typedef enum
{
	ADRESSING_MODE_HORIZONTAL = 0x00,
	ADRESSING_MODE_VERTICAL   = 0x01,
	ADRESSING_MODE_PAGE       = 0x02,
	ADRESSING_MODE_INVALID    = 0x03
}SSD1306_ADRESSING_MODE;

typedef enum
{
	PAGE_0 = 0xB0,
	PAGE_1 = 0xB1,
	PAGE_2 = 0xB2,
	PAGE_3 = 0xB3,
	PAGE_4 = 0xB4,
	PAGE_5 = 0xB5,
	PAGE_6 = 0xB6,
	PAGE_7 = 0xB7
}SSD1306_PAGE;


void SSD1306_Init();
void SSD1306_setAdressingMode(SSD1306_ADRESSING_MODE mode);
void SSD1306_SetPage(SSD1306_PAGE startAddress, SSD1306_PAGE endAddress);
void SSD1306_SetColumn(uint8_t startAddress, uint8_t endAddress);
void SSD1306_ClearDisplay();
void SSD1306_PAGE_setColumn(uint8_t column);
void SSD1306_PAGE_setPage(SSD1306_PAGE page);
void SSD1306_PAGE_setPage(SSD1306_PAGE page);
void SSD1306_DrawLine(uint8_t line, uint8_t start_column, uint8_t end_column);
void updateDisplay();



#endif /* INC_SSD_LIBRARY_STM32_H_ */
