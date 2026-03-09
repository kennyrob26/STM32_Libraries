/*
 * Sh_Card_Driver.c
 *
 *  Created on: Mar 9, 2026
 *      Author: kenny
 */


#include "Sd_Card_Driver.h"

void SD_SetSPI(SD_HandleTypeDef *sd, SPI_HandleTypeDef *spi)
{
	sd->hspi = spi;
}

void SD_SetPin_CS(SD_HandleTypeDef *sd, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
	sd->cs_pin.port = cs_port;
	sd->cs_pin.pin  = cs_pin;
}

void SD_EnterSpiMode(SD_HandleTypeDef *sd)
{
	HAL_Delay(100);

	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 1);
	uint8_t initSd_spiMode[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   //80 pulsos de clock com MOSI em HIGH;
	HAL_SPI_Transmit(sd->hspi, initSd_spiMode, sizeof(initSd_spiMode), 10);


	//HAL_GPIO_WritePin(GPIOB, sd_cs_Pin, 1);
	HAL_SPI_Transmit(sd->hspi, initSd_spiMode, sizeof(initSd_spiMode), 10);

}

int16_t spi_waiting_response(SD_HandleTypeDef *sd, uint16_t response_timeout)
{
	int16_t rx = 0xFF;
	uint8_t dummy_byte = 0xFF;

	uint32_t initial_time  = HAL_GetTick();

	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 0);
	while(rx == 0xFF)
	{
		if((HAL_GetTick() - initial_time) > response_timeout)
		{
			rx = -1;
			break;
		}

		HAL_SPI_TransmitReceive(sd->hspi, &dummy_byte, (uint8_t*)&rx, 1, 100);
	}
	HAL_SPI_Transmit(sd->hspi, &dummy_byte, 1, 100);    //8 clocks  after the answer
	HAL_GPIO_WritePin(sd->cs_pin.port, sd->cs_pin.pin, 1);

	return rx;

}

int16_t SD_TransmitCMD(SD_HandleTypeDef *sd, uint8_t command_id, uint32_t argument, uint16_t timeout_response)
{
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

	return spi_waiting_response(sd, 10);
}
