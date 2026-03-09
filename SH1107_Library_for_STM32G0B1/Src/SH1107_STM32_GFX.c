/*
 * SH1107_STM32_GFX.c
 *
 *  Created on: Mar 5, 2026
 *      Author: kenny
 */


#include "SH1107_STM32_GFX.h"

SH1107_ERROR SH1107_GFX_ClearnDisplay(SH1107_HandleTypeDef *sh1107)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;

	memset(sh1107->buffer.allPixels, 0x00, SH1107_ALLBYTES);
	for(uint8_t i=0; i<SH1107_PAGES; i++)
	{
		sh1107->page[i].max_x = (SH1107_WIDTH - 1);
		sh1107->page[i].min_x = 0;
		sh1107->page[i].write = SH1107_PAGE_WRITTEN;
	}


	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_DrawPixel(SH1107_HandleTypeDef *sh1107, uint8_t x, uint8_t y, SH1107_PIXEL_STATE color)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(x >= SH1107_WIDTH || y >= SH1107_HEIGHT)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	uint8_t page = convertLineToPage(y);
	uint8_t byte_column = convertLineToHex(y, color);

	if(color == SH1107_PIXEL_ON)
		sh1107->buffer.pages[page][x] |= byte_column;
	else if(color == SH1107_PIXEL_OFF)
		sh1107->buffer.pages[page][x] &= ~byte_column;

	sh1107->page[page].write = SH1107_PAGE_WRITTEN;

	if(sh1107->page[page].min_x > x)
		sh1107->page[page].min_x = x;
	if(sh1107->page[page].max_x < x)
		sh1107->page[page].max_x = x;

	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_UpdatePage(SH1107_HandleTypeDef *sh1107, uint8_t page, uint8_t *data)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(data == NULL)
		return SH1107_ERROR_INCORRECT_PARAMETER;


	sh1107->page[page].write = SH1107_PAGE_WRITTEN;

	//Preciso otimizar aqui...................
	sh1107->page[page].max_x = 127;
	sh1107->page[page].min_x = 0;

	memcpy(sh1107->buffer.pages[page], data, SH1107_WIDTH);

	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_DrawFillRetangle(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1107_PIXEL_STATE color)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;

	if(x1 >= SH1107_WIDTH  || x2 >= SH1107_WIDTH || y1 >= SH1107_HEIGHT || y2 >= SH1107_HEIGHT)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	normalize_range(&x1, &x2);
	normalize_range(&y1, &y2);

	for(uint8_t line=y1; line<=y2; line++)
	{
		for(uint8_t column=x1; column<=x2; column++)
			SH1107_GFX_DrawPixel(sh1107, column, line, color);
	}

	return SH1107_OK;
}


SH1107_ERROR SH1107_GFX_DrawRectangle(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t border_weigth, SH1107_PIXEL_STATE color)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(x1 >= SH1107_WIDTH  || x2 >= SH1107_WIDTH || y1 >= SH1107_HEIGHT || y2 >= SH1107_HEIGHT)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	normalize_range(&x1, &x2);
	normalize_range(&y1, &y2);

	uint8_t w = 1;
	while(w <= border_weigth)
	{
		for(uint8_t i=x1; i<=x2; i++)
		{
			SH1107_GFX_DrawPixel(sh1107, i, y1, color);
			SH1107_GFX_DrawPixel(sh1107, i, y2, color);
		}
		for(uint8_t i=(y1+1); i<=(y2-1); i++)
		{
			SH1107_GFX_DrawPixel(sh1107, x1, i, color);
			SH1107_GFX_DrawPixel(sh1107, x2, i, color);
		}

		if((x2-x1) > 2 && (y2-y1) > 2)
		{
			x1++; y1++;
			x2--; y2--;
		}
		else
			break;
		w++;
	}

	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_DrawLine(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1107_PIXEL_STATE color)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(x1 >= SH1107_WIDTH  || x2 >= SH1107_WIDTH || y1 >= SH1107_HEIGHT || y2 >= SH1107_HEIGHT)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	int dx = abs((int)x2 - (int)x1);
	int dy = -abs((int)y2 - (int)y1);
	int error = dx + dy;
	int error2 = 0;

	int sx = x1<x2 ? 1: -1;
	int sy = y1<y2 ? 1: -1;

	while(1)
	{
		SH1107_GFX_DrawPixel(sh1107, x1, y1, color);
		if(x1==x2 && y1==y2)
			break;

		error2 = (error << 1); // error*2

		if(error2 >= dy)
		{
			error += dy;
			x1    += sx;
		}
		if(error2 <= dx)
		{
			error += dx;
			y1    += sy;
		}

	}

	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_PrintPage(SH1107_HandleTypeDef *sh1107, uint8_t page)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(page >= SH1107_PAGES)
		return SH1107_ERROR_INCORRECT_PARAMETER;


	if(sh1107->page[page].write == SH1107_PAGE_WRITTEN)
	{
		uint8_t initial_column = sh1107->page[page].min_x;
		uint8_t max_column     = sh1107->page[page].max_x - sh1107->page[page].min_x;

		uint8_t *data = &sh1107->buffer.pages[page][initial_column];

		SH1107_CMD_SetCursor(sh1107, initial_column, page);
		SH1107_CMD_WriteDisplayData(sh1107, data, (max_column + 1));

		sh1107->page[page].max_x = 0;
		sh1107->page[page].min_x = 127;

	}

	sh1107->page[page].write = SH1107_PAGE_NO_WRITTEN;

	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_PrintDisplay(SH1107_HandleTypeDef *sh1107)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;

	uint8_t page = 0;
	while(page < SH1107_PAGES)
	{

		SH1107_GFX_PrintPage(sh1107, page);
		page++;
	}
	return SH1107_OK;
}

SH1107_ERROR SH1107_GFX_DrawFrame(SH1107_HandleTypeDef *sh1107, uint8_t *dataFrame, uint16_t size)
{
	if(sh1107 == NULL)
			return SH1107_ERROR_SH1107_NOT_DEFINED;

	memcpy(sh1107->buffer.allPixels, dataFrame, size);

	return SH1107_OK;

}

SH1107_ERROR SH1107_GFX_PrintFrame(SH1107_HandleTypeDef *sh1107, uint8_t *datframe, uint16_t size)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;

	SH1107_GFX_DrawFrame(sh1107, datframe, size);
	SH1107_CMD_ForceDisplayUpdate(sh1107);

	return SH1107_OK;
}
SH1107_ERROR SH1107_GFX_PrintGif(SH1107_HandleTypeDef *sh1107, uint8_t *frames, uint8_t size, uint16_t time_frames)
{
	if(sh1107 == NULL)
			return SH1107_ERROR_SH1107_NOT_DEFINED;

	for(uint16_t i=0; i<size; i++)
	{
		SH1107_GFX_PrintFrame(sh1107, (frames + (i * SH1107_ALLBYTES)), SH1107_ALLBYTES);
		HAL_Delay(time_frames);
	}

	return SH1107_OK;
}










