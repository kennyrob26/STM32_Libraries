/*
 * Sd_Card_Driver.h
 *
 *  Created on: Mar 9, 2026
 *      Author: kenny
 */

#ifndef SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_
#define SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_


#include "stm32g0xx_hal.h"

typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
}SD_GPIO_t;

typedef struct
{
	SPI_HandleTypeDef *hspi;
	SD_GPIO_t cs_pin;

}SD_HandleTypeDef;


void SD_SetSPI(SD_HandleTypeDef *sd, SPI_HandleTypeDef *spi);
void SD_SetPin_CS(SD_HandleTypeDef *sd, GPIO_TypeDef *cs_port, uint16_t cs_pin);
void SD_EnterSpiMode(SD_HandleTypeDef *sd);
int16_t spi_waiting_response(SD_HandleTypeDef *sd, uint16_t response_timeout);
int16_t SD_TransmitCMD(SD_HandleTypeDef *sd, uint8_t command_id, uint32_t argument, uint16_t timeout_response);

#endif /* SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_ */
