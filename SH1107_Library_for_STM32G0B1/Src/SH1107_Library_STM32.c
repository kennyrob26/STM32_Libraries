/*
 * SH1107_Library_STM32.c
 *
 *  Created on: Feb 25, 2026
 *      Author: kenny
 */


#include "SH1107_Library_STM32.h"
#include "string.h"


uint8_t convertLineToPage(uint8_t target_line)
{
	return target_line >> 3; //division for 8
}

uint8_t convertLineToHex(uint8_t target_line)
{
	uint8_t bit_line = target_line % 8;
	uint8_t bit_hex  = 0x01 << bit_line;
	return bit_hex;
}

/**
 * @brief Receive SPI Handle
 *
 * Receive a pointer for SPI_HancleTypeDef
 *
 * @param sh1107 is the current instance of the sh1107 display
 * @param hspi   receive a instance of the handle SPI for this display
 */
SH1107_ERROR SH1107_SetSPI(SH1107_HandleTypeDef *sh1107, SPI_HandleTypeDef *hspi)
{
	if(hspi == NULL)
		return SH1107_ERROR_SPI_HANDLE_NOT_DEFINED;

	sh1107->hspi = hspi;

	return SH1107_OK;
}

/**
 * @brief Receive a CS Pin for SH1107 Display
 *
 * Receive a Chip Select (CS) pin for SH1107, define CS pin is mandatory
 *
 * @param sh1107 is the current instance of the sh1107 display
 * @param *port  is a pointer for GPIO port
 * @param pin	 is a GPIO pin
 */
SH1107_ERROR SH1107_SetPin_Cs(SH1107_HandleTypeDef *sh1107, GPIO_TypeDef *port, uint16_t pin)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(port == NULL)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	sh1107->cs_pin.port = port;
	sh1107->cs_pin.pin  = pin;

	return SH1107_OK;
}

/**
 * @brief Receive a DC Pin for SH1107 Display
 *
 * Receive a Data/Control (D/C) pin for SH1107, define DC pin is mandatory
 *
 * @param sh1107 is the current instance of the sh1107 display
 * @param *port  is a pointer for GPIO port
 * @param pin	 is a GPIO pin
 */
SH1107_ERROR SH1107_SetPin_Dc(SH1107_HandleTypeDef *sh1107, GPIO_TypeDef *port, uint16_t pin)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(port == NULL)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	sh1107->dc_pin.port = port;
	sh1107->dc_pin.pin  = pin;

	return SH1107_OK;
}

/**
 * @brief Receive a Reset Pin for SH1107 Display
 *
 * Receive a Reset pin for SH1107,  define reset pin is optional
 *
 * @param sh1107 is the current instance of the sh1107 display
 * @param *port  is a pointer for GPIO port
 * @param pin	 is a GPIO pin
 */
SH1107_ERROR SH1107_SetPin_Reset(SH1107_HandleTypeDef *sh1107, GPIO_TypeDef *port, uint16_t pin)
{
	if(sh1107 == NULL)
		return SH1107_ERROR_SH1107_NOT_DEFINED;
	if(port == NULL)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	sh1107->reset_pin.port  = port;
	sh1107->reset_pin.pin   = pin;

	return SH1107_OK;
}

/**
 * @brief Transmit data/command for SH1107
 *
 * This function Transmit a data or command for SH1107 using HAL_SPI_Transmit
 *
 * @param sh1107  is the current instance of the sh1107 display
 * @param dc      Selects Data or Command (DC), receives an SH1107_DC type
 * @param *buffer Is a byte se

/**
 * @brief Receive SPI Handlequence for commands or datas
 * @param size    This is the size of buffer,
 */
SH1107_ERROR SH1107_Transmit(SH1107_HandleTypeDef *sh1107, SH1107_DC dc, uint8_t *buffer, uint16_t size)
{
	HAL_StatusTypeDef errorcode = HAL_OK;

	if(sh1107->hspi == NULL)
		return SH1107_ERROR_SPI_HANDLE_NOT_DEFINED;
	if(sh1107->cs_pin.port == NULL)
		return SH1107_ERROR_SPI_CS_NOT_DEFINED;
	if(sh1107->dc_pin.port == NULL)
		return SH1107_ERROR_SPI_DC_NOT_DEFINED;

	HAL_GPIO_WritePin(sh1107->cs_pin.port, sh1107->cs_pin.pin, 0);		//seleciona o chip
	HAL_GPIO_WritePin(sh1107->dc_pin.port, sh1107->dc_pin.pin, dc);		//Enviar um comando
	errorcode = HAL_SPI_Transmit(sh1107->hspi, buffer, size, 10);
	HAL_GPIO_WritePin(sh1107->cs_pin.port, sh1107->cs_pin.pin, 1);

	if(errorcode == HAL_OK)
		return SH1107_OK;

	return SH1107_ERROR_SPI_HAL;
}

/**
 * @brief Reset of SH1107 witch pin Reset
 *
 * This command forces a reset of the SH1107 using the reset pin
 *
 * @param sh1107  is the current instance of the sh1107 display
 */
SH1107_ERROR SH1107_CMD_Reset(SH1107_HandleTypeDef *sh1107)
{
	if(sh1107->hspi == NULL)
		return SH1107_ERROR_SPI_HANDLE_NOT_DEFINED;

	if(sh1107->reset_pin.port == NULL || sh1107->cs_pin.port == NULL)
		return SH1107_ERROR_PIN_NOT_DEFINED;

	HAL_GPIO_WritePin(sh1107->reset_pin.port, sh1107->reset_pin.pin, 0);
	HAL_Delay(20);
	HAL_GPIO_WritePin(sh1107->reset_pin.port, sh1107->reset_pin.pin, 1);
	HAL_GPIO_WritePin(sh1107->cs_pin.port, sh1107->cs_pin.pin, 1);		//Não seleciona o chip
	HAL_Delay(10);

	return SH1107_OK;

}

/**
 * @brief ON/OFF Display
 *
 * This commmand setter is display is ON or OFF.
 *
 * @param sh1107         is the current instance of the sh1107 display
 * @param display_on_off setter the display state (ON/OFF), receive a SH1107_Status type
 */
SH1107_ERROR SH1107_CMD_Display_ON(SH1107_HandleTypeDef *sh1107, SH1107_Status display_on_off)
{
	const uint8_t cmd_display_on  = 0xAF;
	const uint8_t cmd_display_off = 0xAE;

	uint8_t cmd = 0;

	if(display_on_off == 1)
		cmd = cmd_display_on;
	else if(display_on_off == 0)
		cmd = cmd_display_off;
	else
		return SH1107_ERROR_INCORRECT_PARAMETER;

	SH1107_ERROR error_code = SH1107_Transmit(sh1107, 0, &cmd, 1);

	if(error_code == SH1107_OK)
		sh1107->display_status = display_on_off;

	return error_code;
}

SH1107_ERROR SH1107_CMD_SetMultiplex(SH1107_HandleTypeDef *sh1107, uint8_t multiplex)
{
	if(multiplex > SH1107_MAX_MULTIPLEX)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	uint8_t cmd_multiplex[2] = {0xA8, multiplex};
	return SH1107_Transmit(sh1107, 0, cmd_multiplex, sizeof(cmd_multiplex));
}

SH1107_ERROR SH1107_CMD_SetAdressingMode(SH1107_HandleTypeDef *sh1107, SH1107_ADRESSING_MODE adress_mode)
{
	const uint8_t page_adressing_mode 	  = 0x20;
	const uint8_t vertical_adressing_mode = 0x21;

	uint8_t mode = 0;
	if(adress_mode == SH1107_ADRESSING_MODE_PAGE)
		mode = page_adressing_mode;
	else if(adress_mode == SH1107_ADRESSING_MODE_VERTICAL)
		mode = vertical_adressing_mode;
	else
		return SH1107_ERROR_INCORRECT_PARAMETER;

	SH1107_ERROR error_code = SH1107_Transmit(sh1107, SH1107_DC_COMMAND, &mode, 1);

	if(error_code == SH1107_OK)
		sh1107->adressing_mode = adress_mode;

	return error_code;
}

SH1107_ERROR SH1107_CMD_SetContrast(SH1107_HandleTypeDef *sh1107, uint8_t contrast)
{
	uint8_t cmd_contrast[2] = {0x81, contrast};

	SH1107_ERROR error_code = SH1107_Transmit(sh1107, SH1107_DC_COMMAND, cmd_contrast, sizeof(cmd_contrast));

	if(error_code == SH1107_OK)
		sh1107->contrast = contrast;

	return error_code;
}

SH1107_ERROR SH1107_CMD_SetDisplayMode(SH1107_HandleTypeDef *sh1107, SH1107_DisplayMode mode)
{
	uint8_t cmd_mode = 0;
	if(mode == SH1107_MODE_NORMAL)
		cmd_mode = 0xA6;
	else if(mode == SH1107_MODE_INVERSE)
		cmd_mode = 0xA7;
	else
		return SH1107_ERROR_INCORRECT_PARAMETER;

	SH1107_ERROR error_code = SH1107_Transmit(sh1107, SH1107_DC_COMMAND, &cmd_mode, 1);

	if(error_code == SH1107_OK)
		sh1107->display_mode = mode;

	return error_code;
}

SH1107_ERROR SH1107_CMD_SetColumn(SH1107_HandleTypeDef *sh1107, uint8_t column)
{
	if(column >= SH1107_WIDTH)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	uint8_t high_bits = 0x10 | ((column >> 4) & 0x07);
	uint8_t low_bits  = 0x00 | (column & 0x0F);

	uint8_t cmd_column[2] = {high_bits, low_bits};

	return SH1107_Transmit(sh1107, SH1107_DC_COMMAND, cmd_column, 2);
}

SH1107_ERROR SH1107_CMD_SetPage(SH1107_HandleTypeDef *sh1107, uint8_t page_adress)
{
	if(page_adress >= SH1107_PAGES)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	uint8_t cmd_page_adress = 0xB0 | (page_adress & 0x0F);

	return SH1107_Transmit(sh1107, SH1107_DC_COMMAND, &cmd_page_adress, 1);
}

SH1107_ERROR SH1107_CMD_SetCursor(SH1107_HandleTypeDef *sh1107, uint8_t column, uint8_t page_adress)
{
	if(column >= SH1107_WIDTH || page_adress >= SH1107_PAGES)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	SH1107_CMD_SetColumn(sh1107, column);
	SH1107_CMD_SetPage(sh1107, page_adress);

	return SH1107_OK;
}

SH1107_ERROR SH1107_CMD_WriteDisplayData(SH1107_HandleTypeDef *sh1107, uint8_t *data, uint16_t size)
{
	if(data == NULL)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	return SH1107_Transmit(sh1107, SH1107_DC_DATA, data, size);;
}

SH1107_ERROR SH1107_DRAW_Page(SH1107_HandleTypeDef *sh1107, uint8_t page, uint8_t *data)
{
	if(data == NULL)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	memcpy(sh1107->buffer[page], data, SH1107_WIDTH);

	return SH1107_OK;
}

SH1107_ERROR SH1107_CMD_ClearDisplay(SH1107_HandleTypeDef *sh1107)
{
	uint8_t clearn_page[SH1107_WIDTH] = {0x00};

	for(uint8_t i=0; i<SH1107_PAGES; i++)
		SH1107_DRAW_Page(sh1107, i, clearn_page);

	SH1107_Update_Display(sh1107);
	return SH1107_OK;
}

SH1107_ERROR SH1107_Draw_Pixel(SH1107_HandleTypeDef *sh1107, uint8_t x, uint8_t y)
{
	if(x >= SH1107_WIDTH || y >= SH1107_HEIGHT)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	uint8_t page = convertLineToPage(y);
	uint8_t byte_column = convertLineToHex(y);

	sh1107->buffer[page][x] |= byte_column;

	return SH1107_OK;
}

SH1107_ERROR SH1107_Update_Page(SH1107_HandleTypeDef *sh1107, uint8_t page)
{
	if(page >= SH1107_PAGES)
		return SH1107_ERROR_INCORRECT_PARAMETER;

	uint8_t *data = sh1107->buffer[page];

	SH1107_CMD_SetCursor(sh1107, 0, page);
	SH1107_CMD_WriteDisplayData(sh1107, data, SH1107_WIDTH);


	return SH1107_OK;
}

SH1107_ERROR SH1107_Update_Display(SH1107_HandleTypeDef *sh1107)
{
	uint8_t page = 0;
	while(page < SH1107_PAGES)
	{
		SH1107_Update_Page(sh1107, page);
		page++;
	}
	return SH1107_OK;
}














