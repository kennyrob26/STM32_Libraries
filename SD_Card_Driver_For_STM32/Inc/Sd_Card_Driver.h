/*
 * Sd_Card_Driver.h
 *
 *  Created on: Mar 9, 2026
 *      Author: kenny
 */

#ifndef SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_
#define SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_


#include "stm32g0xx_hal.h"

#define SD_BLOCK_SIZE 512

#define CMD_0_CRC 0x95
#define CMD_8_CRC 0x87



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
	SD_ERROR_TIMEOUT                = 4,
	SD_ERROR_NO_INIT                = 5,
	SD_ERROR_ 						= 6
}SD_ERROR;

typedef enum
{
	SD_R1_OK 		  =  0x0,
	SD_R1_IDLE        =  0x01,
	SD_R1_NO_RESPONSE =  0xFF,
	SD_R1_ERROR       =  -1,
	SD_R1_TIMEOUT     =  -2

}SD_R1_Response;

typedef enum
{
	SD_INIT_OK                = 0,
	SD_INIT_ENTER_SPI_MODE    = 1,
	SD_INIT_RESET_SD_CARD     = 2,
	SD_INIT_SET_APP_CMD       = 3,
	SD_INIT_SEND_APP_INIT_CMD = 4,
	SD_INIT_ERROR_TIMEOUT     = 5,
	SD_INIT_ERROR             = 6
}SD_Init_Status;

typedef enum
{
	SD_TOKEN_READ_START  = 0xFE,
	SD_TOKEN_WRITE_START = 0xFC,
	SD_TOKEN_WRITE_STOP  = 0xFD,
	SD_RESPONSE_ACCEPTED    = 0xE5, //010
	SD_RESPONSE_CRC_ERROR   = 0xEB, //101
	SD_RESPONSE_WRITE_ERROR = 0xED  //110
}SD_Token;


SD_ERROR SD_SPI_SetSPI(SD_HandleTypeDef *sd, SPI_HandleTypeDef *spi);
SD_ERROR SD_SPI_SetCsPin(SD_HandleTypeDef *sd, GPIO_TypeDef *cs_port, uint16_t cs_pin);
SD_ERROR SD_SPI_setCsHigh(SD_HandleTypeDef *sd);
SD_ERROR SD_SPI_SetCsLow(SD_HandleTypeDef *sd);
SD_ERROR SD_SPI_EnterSpiMode(SD_HandleTypeDef *sd);
SD_R1_Response SD_SPI_WaitingResponse(SD_HandleTypeDef *sd, uint16_t response_timeout);
SD_ERROR SD_SPI_WhileBusyWait(SD_HandleTypeDef *sd, uint16_t timeout);
SD_R1_Response SD_SPI_TransmitCMD(SD_HandleTypeDef *sd, uint8_t command_id, uint32_t argument, uint16_t timeout_response);
SD_R1_Response SD_CMD_ResetSdCard(SD_HandleTypeDef *sd);
SD_R1_Response SD_CMD_SetAppCommand(SD_HandleTypeDef *sd);
SD_R1_Response SD_CMD_AppInitSD(SD_HandleTypeDef *sd);
SD_ERROR SD_Init(SD_HandleTypeDef *sd, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
SD_ERROR SD_CMD_ReadSingleBlock(SD_HandleTypeDef *sd, uint32_t block, uint8_t *read_buffer, uint32_t size);
SD_ERROR SD_CMD_ReadMultipleBlock(SD_HandleTypeDef *sd, uint32_t init_block, uint8_t read_buffer[][512], uint32_t size);
SD_ERROR SD_CMD_WriteSingleBlock(SD_HandleTypeDef *sd, uint32_t block, uint8_t *write_block);
SD_ERROR SD_CMD_WriteMultipleBlocks(SD_HandleTypeDef *sd, uint32_t init_block, uint8_t write_buffer[][512], uint32_t size);
#endif /* SD_CARD_DRIVER_FOR_STM32_INC_SD_CARD_DRIVER_H_ */
