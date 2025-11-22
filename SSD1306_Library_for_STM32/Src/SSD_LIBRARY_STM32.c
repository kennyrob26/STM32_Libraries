/*
 * AHT_LIBRARY_STM32.C
 *
 *  Created on: Nov 14, 2025
 *      Author: kenny
 */

#include <SSD_LIBRARY_STM32.h>
#include "string.h"

I2C_HandleTypeDef *handleI2C;
//uint8_t display[8][128];
uint8_t display[8][128];
uint8_t display_buffer[8][128];

static uint8_t pagesAreEquals(uint8_t page_A[128], uint8_t page_B[128])
{
	if(memcmp(page_A, page_B, 128) != 0)
		return 0;
	else
		return 1;
}

static uint8_t convertLinetoHex(uint8_t line)
{
    uint8_t mod = (line+1) % 8;

    if(mod == 1)
        return(0x01);
    else if(mod == 2)
        return(0x02);
    else if(mod == 3)
        return(0x04);
    else if(mod == 4)
        return(0x08);
    else if(mod == 5)
        return(0x10);
    else if(mod == 6)
        return(0x20);
    else if(mod == 7)
        return(0x40);
    else if(mod == 0)
        return(0x80);
}

static SSD1306_PAGE convertLineToPage(uint8_t line)
{
    //line -= 1;
    if(line >= 0 && line <= 63)
    {
        if(line <= 7)
            return PAGE_0;
        else if (line <= 15)
            return PAGE_1;
        else if (line <= 23)
            return PAGE_2;
        else if (line <= 31)
            return PAGE_3;
        else if (line <= 39)
            return PAGE_4;
        else if (line <= 47)
            return PAGE_5;
        else if (line <= 55)
            return PAGE_6;
        else if (line <= 63)
            return PAGE_7;
    }
}

void SSD1306_Init(I2C_HandleTypeDef *handle_i2c)
{
	  handleI2C = handle_i2c;

	  // Display OFF (0xAE)
	  uint8_t display_off[2] = {0x00, 0xAE};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display_off, 2, 100);

	  // Resume to RAM Content (0xA4)
	  uint8_t entire_display[2] = {0x00, 0xA4};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, entire_display, 2, 100);

	  // Set Display Start Line (0x40)
	  uint8_t display_start_line[2] = {0x00, 0x40};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display_start_line, 2, 100);

	  /*
	  // Set Segment Re-map (0xA0)
	  uint8_t display_remap[2] = {0x00, 0xA0};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display_remap, 2, 100);
*/
	  // Set Multiplex Ratio (0xA8, 0x3F é o padrão, mas 0xBF também funciona)
	  // Set Multiplex Ratio (0xA8, 0x3F para 64 linhas)
	  uint8_t set_MUX[3] = {0x00, 0xA8, 0x3F}; // Mantenha 0x3F (63d => 64 linhas)
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, set_MUX, 3, 100);

	  // Set COM Output Scan Direction (0xC8 - Invertido, melhor para 64 linhas)
	  uint8_t setCOM[2] = {0x00, 0xC0}; // Mantenha 0xC8
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, setCOM, 2, 100);

	  uint8_t setCOM_PIN[3] = {0x00, 0xDA, 0xDA};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, setCOM_PIN, 3, 100);
	  // Set Display Offset (0xD3, 0x00)
	  uint8_t set_Display_offset[3] = {0x00, 0xD3, 0x00};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, set_Display_offset, 3, 100);

	  // ESSENCIAL: Set Charge Pump (0x8D, 0x14)
	  uint8_t cmd_pump[3] = {0x00, 0x8D, 0x14};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, cmd_pump, 3, 100);

	  // Set Display Clock Divisor (0xD5, 0x80)
	  uint8_t setDisplayClock[3] = {0x00, 0xD5, 0x80};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, setDisplayClock, 3, 100);

	  // Set Pre-charge Period (0xD9, 0xF1)
	  uint8_t setPrecharged[3] = {0x00, 0xD9, 0xF1};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, setPrecharged, 3, 100);

	  // Set VCOM Deselect Level (0xDB, 0x40)
	  uint8_t deselectLevel[3] = {0x00, 0xDB, 0x40}; // CORRIGIDO: Array [3], Transmit 3
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, deselectLevel, 3, 100);

	  // Display ON (0xAF)
	  HAL_Delay(1000);
	  uint8_t display_on[2] = {0x00, 0xAF};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display_on, 2, 100);


	  //Mapeamento
	  // 1. Mapeamento de Segmentos (0xA1 - Invertido)
	  uint8_t cmd_remap_seg[2] = {0x00, 0xA1};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, cmd_remap_seg, 2, 100);

/*	  // 2. Direção de Varredura COM (0xC8 - Invertido)
	  uint8_t cmd_scan_dir[2] = {0x00, 0xC0};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, cmd_scan_dir, 2, 100);
*/
}

void SSD1306_setAdressingMode(SSD1306_ADRESSING_MODE mode)
{
	  uint8_t adressing_mode[3] = {0x00, 0x20, mode};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, adressing_mode, 3, 100);
}

void SSD1306_SetColumn(uint8_t startAddress, uint8_t endAddress)
{
	uint8_t cmd_area_col[4] = {0x00, 0x21, startAddress, endAddress}; // C0 a C127
	HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, cmd_area_col, 4, 100);
}

void SSD1306_SetPage(SSD1306_PAGE startAddress, SSD1306_PAGE endAddress)
{
	  uint8_t cmd_area_page[4] = {0x00, 0x22, startAddress, endAddress}; // P0 a P7
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, cmd_area_page, 4, 100);
}

void SSD1306_ClearDisplay(SSD1306_COLOR color)
{
	uint8_t pixels;
	if(color == COLOR_WHITE)
		pixels = 0xFF;
	else if(color == COLOR_BLACK)
		pixels = 0x00;

	for(uint8_t i=0; i<8; i++)
	{
		for(uint8_t j=0; j<128; j++)
		{
			display_buffer[i][j] = pixels;
		}
	}

	SSD1306_UpdateDisplay();
}

void SSD1306_PAGE_setColumn(uint8_t column)
{
    uint8_t lowerNibble  = 0,
            higherNibble = 0;

    lowerNibble = (column & 0x0F);

    higherNibble = 0x10 | (column >> 4);

    uint8_t column_set[3] = {0x00, lowerNibble, higherNibble};
    HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, column_set, 3, 100);

}


void SSD1306_PAGE_setPage(SSD1306_PAGE page)
{
	  uint8_t display_page[2] = {0x00, page};
	  HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display_page, 2, 100);
}

/*
void SSD1306_DrawLine(uint8_t line, uint8_t start_column, uint8_t end_column)
{
	SSD1306_PAGE page = convertLineToPage(line);
	uint8_t line_hex  = convertLinetoHex(line);


	uint8_t qtd_pixels = (end_column - start_column) + 1;
	uint8_t pixels[qtd_pixels];
	pixels[0] = 0x40;

	SSD1306_SetColumn(start_column, end_column);
	SSD1306_SetPage(page, page);

    for(uint8_t i = 1; i <= qtd_pixels; i++)
    	pixels[i] = line_hex;

    HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, pixels, qtd_pixels, 500);
}
*/
void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
	uint8_t page     = convertLineToPage(y) & 0x0F;
	uint8_t line_hex = convertLinetoHex(y);

	if(color == COLOR_WHITE)
		display_buffer[page][x] |= line_hex;
	else if(color == COLOR_BLACK)
		display_buffer[page][x] &= (~line_hex);
}
void SSD1306_DrawLineHorizontal(uint8_t x1, uint8_t x2, uint8_t y, SSD1306_COLOR color)
{
	for(uint8_t i = x1; i<=x2; i++)
	{
		SSD1306_DrawPixel(i, y, color);
	}
}

SSD1306_DrawLineVertical(uint8_t x, uint8_t y1,uint8_t y2, SSD1306_COLOR color)
{
	for(uint8_t i=y1; i<=y2; i++)
	{
		SSD1306_DrawPixel(x, i, color);
	}
}
void SSD1306_DrawLine(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, SSD1306_COLOR color)
{
	if(x1 == x2)
		SSD1306_DrawLineVertical(x1, y1, y2, color);
	else if(y1 == y2)
		SSD1306_DrawLineHorizontal(x1, x2, y1, color);
}
/*
void SSD1306_DrawLine(uint8_t line, uint8_t start_column, uint8_t end_column)
{
	uint8_t page     = convertLineToPage(line) & 0x0F;
	uint8_t line_hex = convertLinetoHex(line);

	//display[page][]

	uint8_t qtd_pixels = (end_column - start_column);

    for(uint8_t i = start_column; i <= qtd_pixels; i++)
    	display[page][i] |= line_hex;

}*/



void SSD1306_UpdatePage(SSD1306_PAGE page, uint8_t page_pixels[128])
{
	SSD1306_setAdressingMode(ADRESSING_MODE_HORIZONTAL);

	SSD1306_SetColumn(0x00, 0x7F);
	SSD1306_SetPage(page, page);

	uint8_t command_size = 129;
	uint8_t i2c_command[command_size];

	memcpy(&i2c_command[1], page_pixels, command_size - 1);
	i2c_command[0] = 0x40;

	HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, i2c_command, command_size, 500);

}

void SSD1306_UpdateDisplay()
{
	for(int page=0; page<=7; page++)
	{
		if(!pagesAreEquals(display[page], display_buffer[page]))
		{
			memcpy(&display[page], display_buffer[page], 128);
			SSD1306_UpdatePage(page, display[page]);
		}
	}
}
/*
void updateDisplay()
{
	SSD1306_setAdressingMode(ADRESSING_MODE_HORIZONTAL);

	SSD1306_SetColumn(0x00, 0x7F);
	SSD1306_SetPage(PAGE_0, PAGE_7);

	//clear display
	uint8_t display_update[129];
	display_update[0] = 0x40;

	for(int i=0; i<=7; i++)
	{

		for (int j = 1; j <=(128); j++)
		{
			display_update[j] = display[i][j-1];
		}
		HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display_update, 129, 500);

		//display[i][0] = 0x40;
		//HAL_I2C_Master_Transmit(handleI2C, DISPLAY_WRITE_CODE, display[i], 129, 500);
	}
}
*/


