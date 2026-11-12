/*
 * modbus.h
 *
 *  Created on: Nov 2, 2026
 *      Author: kenny
 */

#ifndef MODBUS_MODBUS_H_
#define MODBUS_MODBUS_H_

#include "stm32f4xx_hal.h"

#define SWAP16(x) (((x) >> 8) | ((x) << 8))


typedef enum
{
	MODBUS_HANDLER_CHECK_ID        		= 0,
	MODBUS_HANDLER_FILL_PACKET_INFO 	= 1,
	MODBUS_HANDLER_CHECK_CRC        	= 2,
	MODBUS_HANDLER_EXECUTE_FC       	= 3,
	MODBUS_HANDLER_RESPONSE_TO_MASTER 	= 4,

	MODBUS_HANDLER_FINISHED             = 200,
	MODBUS_HANDLER_ERROR                = 201
}ModBus_ReceivePacket_t;

typedef enum
{
	MODBUS_OK 							= 0,
	MODBUS_ERROR 						= 1,
	MODBUS_ERROR_HANDLER_NOT_DEFINED 	= 2,
	MODBUS_ERROR_PACKET_MOUNT           = 3,
	MODBUS_ERROR_CRC_INVALID            = 4,

}ModBus_Error_t;

typedef struct
{
	uint8_t  coils[50];               //FC01
	uint8_t  discrete_inputs[50];     //FC02
	uint16_t holding_registers[25];	  //FC03
	uint16_t input_registers[25];     //FC04
}MB_Table_t;

typedef enum
{
	FC01 = 0x01,			//Read Coils
	FC02 = 0x02,			//Read Discrete inputs
	FC03 = 0x03,			//Read Holding Registers
	FC04 = 0x04,			//Read Input Registers

	FC05 = 0x05,			//Write Single Coil
	FC06 = 0x06,			//Write Singe Register
	FC15 = 0x0F,			//Write Multiple Coils
	FC16 = 0x10,			//Write Multiple Registers
}MB_Function_Code_t;

typedef struct
{
	uint8_t slave_id;
	MB_Function_Code_t function_code;
	uint8_t start_addres[2];
}MB_head_t;

typedef struct
{
	uint8_t packet_size;
	uint8_t slave_id;
	MB_Function_Code_t function_code;
	uint16_t start_address;
	uint16_t quantity;
	uint8_t byte_count;
	//MB_data_t data;
	uint16_t crc;
}MB_packet_t;


typedef struct
{
	uint8_t id;

	UART_HandleTypeDef* huart;
	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;

	MB_packet_t packet_info;
	MB_Table_t table;

	uint8_t buffer_rx[50];
	uint8_t buffer_tx[70];

	uint8_t response_size;

	uint16_t packet_size;
}MB_handler_t;


uint16_t CRC16_calc(uint8_t *buffer, uint8_t buffer_size);
void ModBus_Transmit(MB_handler_t *handler, uint8_t *buffer, uint8_t size);

ModBus_Error_t ModBus_FillPacketInfo(MB_handler_t *handler);

ModBus_Error_t ModBus_ValidadeCRC(MB_handler_t *handler);

void ModBus_MountReceivedPacket(MB_handler_t *handler/*, MB_packet_t *packet_info*//*,  uint8_t size, MB_Buffer_t *buffer_rx*/);

#endif /* MODBUS_MODBUS_H_ */
