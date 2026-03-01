/*
 * SH1107_Library_STM32.h
 *
 *  Created on: Feb 25, 2026
 *      Author: kenny
 */

#ifndef INC_SH1107_LIBRARY_STM32_H_
#define INC_SH1107_LIBRARY_STM32_H_

#include "stm32g0xx_hal.h"

#define SH1107_MAX_MULTIPLEX 	 0x7FU
#define SH1107_128x128_MULTIPLEX 0x7FU

#define SH1107_WIDTH  128
#define SH1107_HEIGHT 128
#define SH1107_PAGES   16



typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
}SH1107_GPIO_t;

typedef enum{
	SH1107_OK                    		= 0,
	SH1107_ERROR_SH1107_NOT_DEFINED     = 1,
	SH1107_ERROR_SPI_HANDLE_NOT_DEFINED = 2,
	SH1107_ERROR_SPI_CS_NOT_DEFINED     = 3,
	SH1107_ERROR_SPI_DC_NOT_DEFINED     = 4,
	SH1107_ERROR_PIN_NOT_DEFINED 		= 5,
	SH1107_ERROR_SPI_HAL                = 6,
	SH1107_ERROR_INCORRECT_PARAMETER    = 7
}SH1107_ERROR;

typedef enum{
	SH1107_SPI_DISCONNECTED = 0,
	SH1107_SPI_CONNECTED    = 1
}SH1107_SPI_Connection;

typedef enum{
	SH1107_DC_COMMAND = 0,
	SH1107_DC_DATA    = 1
}SH1107_DC;

typedef enum{
	SH1107_STATUS_OFF = 0,
	SH1107_STATUS_ON  = 1
}SH1107_Status;

typedef enum{
	SH1107_ADRESSING_MODE_PAGE     = 0,
	SH1107_ADRESSING_MODE_VERTICAL = 1
}SH1107_ADRESSING_MODE;

typedef enum{
	SH1107_CONTRAST_LOW     = 0x00U, //0
	SH1107_CONTRAST_DEFAULT = 0x80U, //128 - max recomended contrast
	SH1107_CONTRAST_HIGH    = 0xFFU
}SH1107_CONTRAST;

typedef enum{
	SH1107_MODE_NORMAL  = 0,
	SH1107_MODE_INVERSE = 1
}SH1107_DisplayMode;

typedef struct
{
	SPI_HandleTypeDef *hspi;

	SH1107_GPIO_t cs_pin;
	SH1107_GPIO_t dc_pin;
	SH1107_GPIO_t reset_pin;

	uint8_t buffer[SH1107_PAGES][SH1107_WIDTH];

	SH1107_Status display_status;
	SH1107_ADRESSING_MODE adressing_mode;
	SH1107_CONTRAST contrast;
	SH1107_DisplayMode display_mode;
}SH1107_HandleTypeDef;

SH1107_ERROR SH1107_SetSPI(SH1107_HandleTypeDef *sh1107, SPI_HandleTypeDef *hspi);
SH1107_ERROR SH1107_SetPin_Cs(SH1107_HandleTypeDef *sh1107,GPIO_TypeDef *port, uint16_t pin);
SH1107_ERROR SH1107_SetPin_Dc(SH1107_HandleTypeDef *sh1107, GPIO_TypeDef *port, uint16_t pin);
SH1107_ERROR SH1107_SetPin_Reset(SH1107_HandleTypeDef *sh1107, GPIO_TypeDef *port, uint16_t pin);
SH1107_ERROR SH1107_Transmit(SH1107_HandleTypeDef *sh1107, SH1107_DC dc, uint8_t *buffer, uint16_t size);
SH1107_ERROR SH1107_CMD_Reset(SH1107_HandleTypeDef *sh1107);
SH1107_ERROR SH1107_CMD_Display_ON(SH1107_HandleTypeDef *sh1107, SH1107_Status display_on_off);
SH1107_ERROR SH1107_CMD_SetMultiplex(SH1107_HandleTypeDef *sh1107, uint8_t multiplex);
SH1107_ERROR SH1107_CMD_SetAdressingMode(SH1107_HandleTypeDef *sh1107, SH1107_ADRESSING_MODE adress_mode);
SH1107_ERROR SH1107_CMD_SetContrast(SH1107_HandleTypeDef *sh1107, uint8_t contrast);
SH1107_ERROR SH1107_CMD_SetDisplayMode(SH1107_HandleTypeDef *sh1107, SH1107_DisplayMode mode);
SH1107_ERROR SH1107_CMD_SetColumn(SH1107_HandleTypeDef *sh1107, uint8_t column);
SH1107_ERROR SH1107_CMD_SetPage(SH1107_HandleTypeDef *sh1107, uint8_t page_adress);
SH1107_ERROR SH1107_CMD_SetCursor(SH1107_HandleTypeDef *sh1107, uint8_t column, uint8_t page_adress);
SH1107_ERROR SH1107_CMD_WriteDisplayData(SH1107_HandleTypeDef *sh1107, uint8_t *data, uint16_t size);
SH1107_ERROR SH1107_DRAW_Page(SH1107_HandleTypeDef *sh1107, uint8_t page, uint8_t *data);
SH1107_ERROR SH1107_CMD_ClearDisplay(SH1107_HandleTypeDef *sh1107);
SH1107_ERROR SH1107_Draw_Pixel(SH1107_HandleTypeDef *sh1107, uint8_t x, uint8_t y);
SH1107_ERROR SH1107_Update_Page(SH1107_HandleTypeDef *sh1107, uint8_t page);
SH1107_ERROR SH1107_Update_Display(SH1107_HandleTypeDef *sh1107);
SH1107_ERROR SH1107_Draw_FillRetangle(SH1107_HandleTypeDef *sh1107, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#endif /* INC_SH1107_LIBRARY_STM32_H_ */
