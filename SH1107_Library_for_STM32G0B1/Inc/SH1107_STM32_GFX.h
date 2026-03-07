/*
 * SH1107_STM32_GFX.h
 *
 *  Created on: Mar 5, 2026
 *      Author: kenny
 */

#ifndef SH1107_LIBRARY_FOR_STM32G0B1_INC_SH1107_STM32_GFX_H_
#define SH1107_LIBRARY_FOR_STM32G0B1_INC_SH1107_STM32_GFX_H_

#include "SH1107_Library_STM32.h"


#endif /* SH1107_LIBRARY_FOR_STM32G0B1_INC_SH1107_STM32_GFX_H_ */

SH1107_ERROR SH1107_GFX_ClearnDisplay(SH1107_HandleTypeDef *sh1107);
SH1107_ERROR SH1107_GFX_DrawPixel(SH1107_HandleTypeDef *sh1107, uint8_t x, uint8_t y, SH1107_PIXEL_STATE color);
SH1107_ERROR SH1107_GFX_UpdatePage(SH1107_HandleTypeDef *sh1107, uint8_t page, uint8_t *data);
SH1107_ERROR SH1107_GFX_DrawFillRetangle(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1107_PIXEL_STATE color);
SH1107_ERROR SH1107_GFX_DrawRectangle(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t border_weigth, SH1107_PIXEL_STATE color);
SH1107_ERROR SH1107_GFX_DrawLine(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1107_PIXEL_STATE color);
SH1107_ERROR SH1107_GFX_DrawFrame(SH1107_HandleTypeDef *sh1107, uint8_t *dataFrame, uint16_t size);
SH1107_ERROR SH1107_GFX_PrintFrame(SH1107_HandleTypeDef *sh1107, uint8_t *datframe, uint16_t size);
SH1107_ERROR SH1107_GFX_PrintGif(SH1107_HandleTypeDef *sh1107, uint8_t *frames, uint8_t size, uint8_t time_frames);
