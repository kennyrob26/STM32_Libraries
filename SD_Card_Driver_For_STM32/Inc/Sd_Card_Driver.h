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

typedef enum
{
	SD_ERROR_OK                     = 0,
	SD_ERROR_NO_HANDLER_DEFINED     = 1,
	SD_ERROR_NO_SPI_HANDLER_DEFINED = 2,
	SD_ERROR_NO_SPI_CS_DEFINED      = 3,
	SD_ERROR_TIMEOUT                = 4
}SD_ERROR;

typedef enum
{
	SD_R1_OK 		  =  0x0,
	SD_R1_BUSY        =  0x01,
	SD_R1_NO_RESPONSE =  0xFF,
	SD_R1_ERROR       =  -1,
	SD_R1_TIMEOUT     =  -2

}SD_R1_Response;


SD_ERROR SD_SPI_SetSPI(SD_HandleTypeDef *sd, SPI_HandleTypeDef *spi);
SD_ERROR SD_SPI_SetCsPin(SD_HandleTypeDef *sd, GPIO_TypeDef *cs_port, uint16_t cs_pin);
SD_ERROR SD_SPI_EnterSpiMode(SD_HandleTypeDef *sd);
SD_R1_Response SD_SPI_WaitingResponse(SD_HandleTypeDef *sd, uint16_t response_timeout);
SD_R1_Response SD_SPI_TransmitCMD(SD_HandleTypeDef *sd, uint8_t command_id, uint32_t argument, uint16_t timeout_response);
SD_R1_Response SD_CMD_ResetSdCard(SD_HandleTypeDef *sd);
SD_R1_Response SD_CMD_SetAppCommand(SD_HandleTypeDef *sd);
SD_R1_Response SD_CMD_AppInitSD(SD_HandleTypeDef *sd);

#endif /* SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_ */
