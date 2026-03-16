/*
 * Sh_Card_Driver.c
 *
 *  Created on: Mar 9, 2026
 *      Author: kenny
 */


#include "Sd_Card_Driver.h"
#include "string.h"
#include "math.h"

static const uint8_t sd_dummy_bytes[SD_BLOCK_SIZE] = {[0 ... SD_BLOCK_SIZE-1] = 0xFF};

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

SD_ERROR SD_SPI_setCsHigh(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;
	if(sd->cs_pin.port == NULL)
		return SD_ERROR_NO_SPI_CS_DEFINED;

	SD_SPI_WhileBusyWait(sd, 1000);

	uint8_t dummy_byte = 0xFF;
	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 1);
	HAL_SPI_Transmit(sd->hspi, &dummy_byte, 1, 100);

	return SD_ERROR_OK;
}

SD_ERROR SD_SPI_SetCsLow(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;
	if(sd->cs_pin.port == NULL)
		return SD_ERROR_NO_SPI_CS_DEFINED;

	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 0);

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

	return SD_ERROR_OK;

}


SD_R1_Response SD_SPI_WaitingResponse(SD_HandleTypeDef *sd, uint16_t response_timeout)
{
	if(sd == NULL)
		return SD_R1_ERROR;

	if(SD_SPI_SetCsLow(sd) != SD_ERROR_OK)
		return SD_ERROR_NO_SPI_CS_DEFINED;

	int16_t rx = 0xFF;
	uint8_t dummy_byte = 0xFF;

	uint32_t initial_time  = HAL_GetTick();

	while(rx == SD_R1_NO_RESPONSE)
	{
		if((HAL_GetTick() - initial_time) > response_timeout)
		{
			rx = SD_R1_TIMEOUT;
			break;
		}

		HAL_SPI_TransmitReceive(sd->hspi, &dummy_byte, (uint8_t*)&rx, 1, 100);
	}

	return rx;

}

SD_ERROR SD_SPI_WaitingExpectedResponse(SD_HandleTypeDef *sd, uint8_t expected_response, uint16_t timeout)
{
	uint8_t response = 0;
	uint32_t initial_time = HAL_GetTick();
	while(response != expected_response)
	{
		if((HAL_GetTick() - initial_time) > timeout)
			return SD_ERROR_TIMEOUT;
		uint8_t dummy = 0xFF;
		HAL_SPI_TransmitReceive(sd->hspi, &dummy, &response, 1, 10);
	}
	return SD_ERROR_OK;
}


SD_ERROR SD_SPI_WhileBusyWait(SD_HandleTypeDef *sd, uint16_t timeout)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	uint8_t response = 0;
	uint32_t initial_time = HAL_GetTick();
	while(response == 0)
	{
		if((HAL_GetTick() - initial_time) > timeout)
			return SD_ERROR_TIMEOUT;
		uint8_t dummy = 0xFF;
		HAL_SPI_TransmitReceive(sd->hspi, &dummy, &response, 1, 1000);
	}

	return SD_ERROR_OK;
}

SD_R1_Response SD_SPI_TransmitCMD(SD_HandleTypeDef *sd, uint8_t command_id, uint32_t argument, uint16_t timeout_response)
{
	if(sd == NULL || command_id > 63)
		return SD_R1_ERROR;

	uint8_t cmd[6] = {0x00};

	uint8_t crc = 0x01;
	if(command_id == 0)
		crc = CMD_0_CRC;
	else if(command_id == 8)
		crc = CMD_8_CRC;

	cmd[0] = (command_id | 0x40);
	cmd[1] = argument >> 24;           //0x11000000
	cmd[2] = (argument >> 16) & 0xFF;  //0x00110000
	cmd[3] = (argument >> 8)  & 0xFF;  //0x00001100
	cmd[4] = argument & 0xFF;
	cmd[5] = crc;

	SD_SPI_SetCsLow(sd);
	HAL_SPI_Transmit(sd->hspi, cmd, sizeof(cmd), 10);

	int16_t response = SD_SPI_WaitingResponse(sd, timeout_response);

	SD_SPI_setCsHigh(sd);
	return response;
}

SD_R1_Response SD_CMD_ResetSdCard(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_R1_ERROR;

	const uint8_t  cmd_resetSD   = 0x00;
	const uint8_t argument      = 0x00;
	const uint16_t  timeout     = 100;

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


	const uint32_t  argument   = 0x40000000;
	const uint8_t  timeout    =   10;

	return SD_SPI_TransmitCMD(sd, SD_ACMD_41, (uint32_t)argument, (uint16_t)timeout);
}

SD_R1_Response SD_CMD_CMD8(SD_HandleTypeDef *sd)
{
	if(sd == NULL || sd->cs_pin.port == NULL)
		return SD_R1_ERROR;

	const uint32_t cmd_argument   = 0x000001AA;
	const uint8_t  cmd_timeout    =   10;

	SD_R1_Response r1_resp = SD_SPI_TransmitCMD(sd, SD_CMD_8, cmd_argument, cmd_timeout);
	SD_SPI_SetCsLow(sd);
	HAL_SPI_Transmit(sd->hspi, (uint8_t []){0xFF}, 1, 100);
	SD_SPI_setCsHigh(sd);

	return r1_resp;
}

SD_ERROR SD_Init(SD_HandleTypeDef *sd, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
	  SD_SPI_SetSPI(sd, hspi);
	  SD_SPI_SetCsPin(sd, cs_port, cs_pin);

	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	SD_Init_Status stateSd = SD_INIT_ENTER_SPI_MODE;
	SD_R1_Response r1_response;
	uint32_t initial_time = HAL_GetTick();

	while(stateSd != SD_INIT_OK)
	{
		if((HAL_GetTick() - initial_time) > 3000)
			stateSd = SD_INIT_ERROR_TIMEOUT;

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
						stateSd = SD_INIT_SEND_CMD8;
					break;
					case SD_R1_NO_RESPONSE:
						stateSd = SD_INIT_ENTER_SPI_MODE;
					break;
					default:
						stateSd = SD_INIT_ERROR;
				}
			break;
			case SD_INIT_SEND_CMD8:
				r1_response = SD_CMD_CMD8(sd);
				switch (r1_response)
				{
					case SD_R1_IDLE:
						stateSd = SD_INIT_SET_APP_CMD;
					break;
					case SD_R1_NO_RESPONSE:
						stateSd = SD_INIT_ENTER_SPI_MODE;
					break;
					default:
						stateSd = SD_INIT_ERROR;
						break;
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
			case SD_INIT_ERROR_TIMEOUT:
				return SD_ERROR_TIMEOUT;
			case SD_INIT_ERROR:
				return SD_ERROR_NO_INIT;
			default:
				break;
		}
	}

	return SD_ERROR_OK;

}

static inline SD_ERROR SD_ReadBlock(SD_HandleTypeDef *sd, uint8_t *block, uint16_t block_size)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	SD_SPI_SetCsLow(sd);

	if(SD_SPI_WaitingExpectedResponse(sd, SD_TOKEN_READ_START, 10) != SD_ERROR_OK)
		return SD_ERROR_;

	HAL_SPI_TransmitReceive(sd->hspi, sd_dummy_bytes, block, block_size, 100);
	uint8_t crcBytes_discard[2] = {0xFF, 0xFF};
	HAL_SPI_Transmit(sd->hspi, crcBytes_discard, sizeof(crcBytes_discard), 10);

	return SD_ERROR_OK;
}

SD_ERROR SD_CMD_ReadSingleBlock(SD_HandleTypeDef *sd, uint32_t block, uint8_t *read_buffer, uint32_t size)
{

	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	uint32_t block_adress;
	if(sd->csd.version == SD_CSD_VERSION_1)
		block_adress = block * sd->csd.block_length;
	else
		block_adress = block;

	SD_SPI_TransmitCMD(sd, SD_CMD_17, block_adress, 100);

	SD_ReadBlock(sd, read_buffer, SD_BLOCK_SIZE);
	SD_SPI_setCsHigh(sd);

	return SD_ERROR_OK;
}

SD_ERROR SD_CMD_ReadMultipleBlock(SD_HandleTypeDef *sd, uint32_t init_block, uint8_t read_buffer[][512], uint32_t size)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	uint32_t init_block_adress;
	if(sd->csd.version == SD_CSD_VERSION_1)
		init_block_adress = init_block * sd->csd.block_length;
	else
		init_block_adress = init_block;

	SD_SPI_TransmitCMD(sd, SD_CMD_18, init_block_adress, 10);

	for(uint32_t i=0; i<size; i++)
		SD_ReadBlock(sd, read_buffer[i], SD_BLOCK_SIZE);

	SD_SPI_TransmitCMD(sd, SD_CMD_12, 0x00, 10);
	SD_SPI_setCsHigh(sd);

	return SD_ERROR_OK;
}

SD_ERROR SD_CMD_WriteSingleBlock(SD_HandleTypeDef *sd, uint32_t block, uint8_t *buffer_write)
{
	SD_Token token = 0;
	SD_ERROR error_status = SD_INIT_OK;

	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	uint32_t block_adress;
	if(sd->csd.version == SD_CSD_VERSION_1)
		block_adress = block * sd->csd.block_length;
	else
		block_adress = block;

	SD_SPI_TransmitCMD(sd, SD_CMD_24, block_adress, 10);

	SD_SPI_SetCsLow(sd);

	uint8_t start_block_fe = 0xFE;
	HAL_SPI_Transmit(sd->hspi, &start_block_fe, 1, 10);
	HAL_SPI_Transmit(sd->hspi, buffer_write, SD_BLOCK_SIZE, 10);

	uint8_t crc[2] = {0xFF, 0xFF};
	HAL_SPI_Transmit(sd->hspi, crc, 2, 10);

	token = SD_SPI_WaitingResponse(sd, 10);
	if(token != SD_RESPONSE_ACCEPTED)
	{
		error_status = SD_ERROR_;

	}

	SD_SPI_setCsHigh(sd);

	return error_status;

}

SD_ERROR SD_CMD_WriteMultipleBlocks(SD_HandleTypeDef *sd, uint32_t init_block, uint8_t write_buffer[][512], uint32_t size)
{
	if(sd == NULL)
			return SD_ERROR_NO_HANDLER_DEFINED;

	SD_Token token = 0;

	uint32_t init_block_adress;
	if(sd->csd.version == SD_CSD_VERSION_1)
		init_block_adress = init_block * sd->csd.block_length;
	else
		init_block_adress = init_block;

	SD_SPI_TransmitCMD(sd, SD_CMD_25, init_block_adress, 10);
	SD_SPI_SetCsLow(sd);


	for(uint8_t i=0; i<size; i++)
	{
		HAL_SPI_Transmit(sd->hspi, (uint8_t[]){SD_TOKEN_WRITE_START}, 1, 10);
		HAL_SPI_Transmit(sd->hspi, write_buffer[i], SD_BLOCK_SIZE, 10);
		HAL_SPI_Transmit(sd->hspi, (uint8_t[]){0xFF,0xFF}, 2, 10);

		token = 0;
		token = SD_SPI_WaitingResponse(sd, 10);
		if(token == SD_RESPONSE_ACCEPTED)
			SD_SPI_WhileBusyWait(sd, 100);
	}
	HAL_SPI_Transmit(sd->hspi, (uint8_t[]){SD_TOKEN_WRITE_STOP}, 1, 1);
	HAL_SPI_Transmit(sd->hspi, (uint8_t[]){SD_DUMMY_BYTE}      , 1, 1);

	SD_SPI_setCsHigh(sd);

	return SD_ERROR_OK;
}

//PAGE 226,  archive:  Part1_Physical_Layer_Simplified_Specification_Ver9.00.pdf
SD_ERROR SD_CMD_CsdRead(SD_HandleTypeDef *sd)
{
	if(sd == NULL)
		return SD_ERROR_NO_HANDLER_DEFINED;

	SD_SPI_TransmitCMD(sd, SD_CMD_9, 0, 100);

	if(SD_SPI_WaitingExpectedResponse(sd, SD_R1_OK, 10) != SD_ERROR_OK)
		return SD_ERROR_;

	uint8_t csd_response[16];
	//SD_ReadBlock(sd, csd_response, sizeof(csd_response));

	SD_SPI_SetCsLow(sd);

	int16_t response = SD_SPI_WaitingResponse(sd, 10);
	if(response < 0)
		return SD_ERROR_;
	else if(response == 0xFE)
		HAL_SPI_TransmitReceive(sd->hspi, sd_dummy_bytes, csd_response, sizeof(csd_response), 100);
	else
	{
		csd_response[0] = response;
		HAL_SPI_TransmitReceive(sd->hspi, sd_dummy_bytes, (csd_response + 1), sizeof(csd_response) -1, 100);
	}

	uint8_t crcBytes_discard[2] = {0xFF, 0xFF};
	HAL_SPI_Transmit(sd->hspi, crcBytes_discard, sizeof(crcBytes_discard), 10);

	sd->csd.version = (csd_response[0] >> 6);

	if(sd->csd.version == SD_CSD_VERSION_1)
	{
		uint8_t read_bl_len = (csd_response[5] & 0x0F);
		sd->csd.block_length = 1UL << read_bl_len;  // 2^(bl_len)


		uint8_t c_size_mult = (csd_response[9] & 0x03) << 1;
		c_size_mult |= (csd_response[10] >> 7);

		sd->csd.size_mult = 1UL << (c_size_mult + 2); //2^(mult+2)


		uint16_t c_size = (uint16_t)(csd_response[6] & 0x03) << 10;  //0000xx0000000000
		c_size |= ((uint16_t)csd_response[7] << 2);
		c_size |= (csd_response[8] >> 6) & 0x03;

		uint32_t blocknr = (c_size + 1) * sd->csd.size_mult;
		sd->csd.size = blocknr * sd->csd.block_length;

		sd->csd.block_count = sd->csd.size / 512;

	}
	else if(sd->csd.version == SD_CSD_VERSION_2)
	{
		sd->csd.block_length = 512;   //In version 2 block is fixed in 512

		uint32_t c_size =  (uint32_t)(csd_response[7] & 0x3F) << 16; //
		c_size |= ((uint32_t)csd_response[8] << 8);
		c_size |= (uint32_t)csd_response[9];

		sd->csd.block_count = (uint64_t)(c_size + 1) << 10; // 2¹⁰
		sd->csd.size = (uint64_t)(c_size + 1) << 19; // 512 is in KB, equals a 1024 blocks of 512 bytes
	}

	SD_SPI_setCsHigh(sd);

	return SD_ERROR_OK;
}













