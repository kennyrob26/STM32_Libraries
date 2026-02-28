/*
 * SH1107_Library_STM32.c
 *
 *  Created on: Feb 25, 2026
 *      Author: kenny
 */


#include "SH1107_Library_STM32.h"

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
	if(sh1107->hspi == NULL)
		return SH1107_ERROR_SPI_HANDLE_NOT_DEFINED;

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
	if(sh1107->hspi == NULL)
		return SH1107_ERROR_SPI_HANDLE_NOT_DEFINED;

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
	if(sh1107->hspi == NULL)
		return SH1107_ERROR_SPI_HANDLE_NOT_DEFINED;

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
 * @param *buffer Is a byte sequence for commands or datas
 * @param size    This is the size of buffer,
 */
SH1107_ERROR SH1107_Transmit(SH1107_HandleTypeDef *sh1107, SH1107_DC dc, uint8_t *buffer, uint16_t size)
{
	if(sh1107->cs_pin.port != NULL && sh1107->dc_pin.port != NULL)
	{
		HAL_GPIO_WritePin(sh1107->cs_pin.port, sh1107->cs_pin.pin, 0);		//seleciona o chip
		HAL_GPIO_WritePin(sh1107->dc_pin.port, sh1107->dc_pin.pin, dc);		//Enviar um comando
		HAL_Delay(1);
		HAL_SPI_Transmit(sh1107->hspi, buffer, size, 10); //seta multiplex
		HAL_Delay(1);
		HAL_GPIO_WritePin(sh1107->cs_pin.port, sh1107->cs_pin.pin, 1);

		return SH1107_OK;
	}
	return SH1107_ERROR_PIN_NOT_DEFINED;
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
	HAL_Delay(100);
	HAL_GPIO_WritePin(sh1107->reset_pin.port, sh1107->reset_pin.pin, 1);
	HAL_GPIO_WritePin(sh1107->cs_pin.port, sh1107->cs_pin.pin, 1);		//Não seleciona o chip
	HAL_Delay(50);

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

	return SH1107_Transmit(sh1107, 0, &cmd, 1);
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

	return SH1107_Transmit(sh1107, SH1107_DC_COMMAND, &mode, 1);
}

SH1107_ERROR SH1107_CMD_SetContrast(SH1107_HandleTypeDef *sh1107, uint8_t contrast)
{
	uint8_t cmd_contrast[2] = {0x81, contrast};

	return SH1107_Transmit(sh1107, SH1107_DC_COMMAND, cmd_contrast, sizeof(cmd_contrast));
}






