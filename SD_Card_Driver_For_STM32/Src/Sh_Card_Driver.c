/*
 * Sh_Card_Driver.c
 *
 *  Created on: Mar 9, 2026
 *      Author: kenny
 */


#include "Sd_Card_Driver.h"

SD_ERROR SD_SPI_SetSPI(SD_HandleTypeDef *sd, SPI_HandleTypeDef *spi)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;
	if(spi == NULL)
		return SD_ERROR_NO_SPI_HANDLER_DEFINED;

	sd->hspi = spi;

	return SD_ERROR_OK;
}

SD_ERROR SD_SPI_SetCsPin(SD_HandleTypeDef *sd, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;
	if(cs_port == NULL)
		return SD_ERROR_NO_SPI_CS_DEFINED;

	sd->cs_pin.port = cs_port;
	sd->cs_pin.pin  = cs_pin;

	return SD_ERROR_OK;
}

SD_ERROR SD_SPI_EnterSpiMode(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;
	if(sd->cs_pin.port == NULL)
		return SD_ERROR_NO_SPI_CS_DEFINED;

	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 1);
	uint8_t initSd_spiMode[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   //80 pulsos de clock com MOSI em HIGH;
	HAL_SPI_Transmit(sd->hspi, initSd_spiMode, sizeof(initSd_spiMode), 10);


	HAL_SPI_Transmit(sd->hspi, initSd_spiMode, sizeof(initSd_spiMode), 10);

	return SD_ERROR_OK;

}

SD_R1_Response SD_SPI_WaitingResponse(SD_HandleTypeDef *sd, uint16_t response_timeout)
{
	if(sd == NULL)
		return SD_R1_ERROR;

	int16_t rx = 0xFF;
	uint8_t dummy_byte = 0xFF;

	uint32_t initial_time  = HAL_GetTick();

	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 0);
	while(rx == SD_R1_NO_RESPONSE)
	{
		if((HAL_GetTick() - initial_time) > response_timeout)
		{
			rx = SD_R1_TIMEOUT;
			break;
		}

		HAL_SPI_TransmitReceive(sd->hspi, &dummy_byte, (uint8_t*)&rx, 1, 100);
	}
	HAL_SPI_Transmit(sd->hspi, &dummy_byte, 1, 100);    //8 clocks  after the answer
	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 1);

	return rx;

}

SD_R1_Response SD_SPI_TransmitCMD(SD_HandleTypeDef *sd, uint8_t command_id, uint32_t argument, uint16_t timeout_response)
{
	if(sd == NULL || command_id > 63)
		return SD_R1_ERROR;

	uint8_t cmd[6] = {0x00};

	if(command_id > 63)
		return -1;
	uint8_t crc = 0x01;
	if(command_id == 0)
		crc = 0x95;
	else if(command_id == 8)
		crc = 0x87;

	cmd[0] = (command_id | 0x40);
	cmd[1] = argument >> 24;           //0x11000000
	cmd[2] = (argument >> 16) & 0xFF;  //0x00110000
	cmd[3] = (argument >> 8)  & 0xFF;  //0x00001100
	cmd[4] = argument & 0xFF;
	cmd[5] = crc;


	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 0);
	HAL_SPI_Transmit(sd->hspi, cmd, sizeof(cmd), 10);

	return SD_SPI_WaitingResponse(sd, timeout_response);
}

SD_R1_Response SD_CMD_ResetSdCard(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_R1_ERROR;

	const uint8_t  cmd_resetSD   = 0x00;
	const uint8_t argument      = 0x00;
	const uint8_t  timeout       = 10;

	return SD_SPI_TransmitCMD(sd, cmd_resetSD, (uint32_t)argument, (uint16_t)timeout);
}

SD_R1_Response SD_CMD_SetAppCommand(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_R1_ERROR;

	const uint8_t  cmd_App    =   55;
	const uint8_t argument   = 0x00;
	const uint8_t  timeout    =   10;

	return SD_SPI_TransmitCMD(sd, cmd_App, (uint32_t)argument, (uint16_t)timeout);

}

SD_R1_Response SD_CMD_AppInitSD(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_R1_ERROR;

	const uint8_t  cmd_initSd =   41;
	const uint8_t  argument   = 0x00;
	const uint8_t  timeout    =   10;

	return SD_SPI_TransmitCMD(sd, cmd_initSd, (uint32_t)argument, (uint16_t)timeout);
}

SD_ERROR SD_Init(SD_HandleTypeDef *sd)
{
	  //SD_SPI_SetSPI(&sd_card, &hspi1);
	  //SD_SPI_SetCsPin(&sd_card, GPIOB, sd_cs_Pin);

	SD_Init_Status stateSd = SD_INIT_ENTER_SPI_MODE;
	SD_R1_Response r1_response;

	while(stateSd != SD_INIT_OK)
	{
		switch (stateSd)
		{
			case SD_INIT_ENTER_SPI_MODE:
				if(SD_SPI_EnterSpiMode(sd) == SD_ERROR_OK)
					stateSd = SD_INIT_RESET_SD_CARD;
				else
					stateSd = SD_INIT_ERROR;
			break;
			case SD_INIT_RESET_SD_CARD:
				r1_response = SD_CMD_ResetSdCard(sd);
				switch(r1_response)
				{
					case SD_R1_IDLE:
						stateSd = SD_INIT_SET_APP_CMD;
					break;
					case SD_R1_NO_RESPONSE:
						stateSd = SD_INIT_ENTER_SPI_MODE;
					break;
					default:
						stateSd = SD_INIT_ERROR;
				}
			break;
			case SD_INIT_SET_APP_CMD:
				r1_response = SD_CMD_SetAppCommand(sd);

				switch(r1_response)
				{
					case SD_R1_IDLE:
						stateSd = SD_INIT_SEND_APP_INIT_CMD;
					break;
					case SD_R1_NO_RESPONSE:
						stateSd = SD_INIT_ENTER_SPI_MODE;
					break;
					default:
						stateSd = SD_INIT_ERROR;
				}
			break;
			case SD_INIT_SEND_APP_INIT_CMD:
				r1_response = SD_CMD_AppInitSD(sd);

				switch(r1_response)
				{
					case SD_R1_IDLE:
						stateSd = SD_INIT_SET_APP_CMD;
					break;
					case SD_R1_OK:
						stateSd = SD_INIT_OK;
					break;
					case SD_R1_NO_RESPONSE:
						stateSd = SD_INIT_ENTER_SPI_MODE;
					break;
					default:
						stateSd = SD_INIT_ERROR;
				}
			break;
			case SD_INIT_ERROR:
				return SD_ERROR_NO_INIT;
			default:
				break;
		}
	}

	return SD_ERROR_OK;

}
















