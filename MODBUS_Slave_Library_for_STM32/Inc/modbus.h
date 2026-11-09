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

typedef struct
{
	uint8_t  coils[50];
	uint16_t registers[25];
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

/*
typedef struct __attribute__((packed))
{
	MB_head_t head;
	uint8_t payload[100];
}MB_packet_raw_t;
*/
/*
typedef union
{
	uint8_t coil[20];
	uint16_t reg[10];
}MB_data_t;
*/
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

/*
typedef union
{
	MB_packet_raw_t packet;
	uint8_t raw[sizeof(MB_packet_raw_t)];
}MB_Buffer_t;
*/

typedef struct
{
	UART_HandleTypeDef* huart;
	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;

	MB_packet_t packet_info;
	MB_Table_t table;
	//MB_Buffer_t buffer_rx;
	uint8_t buffer_rx[50];

	uint16_t packet_size;
}MB_handler_t;


uint16_t CRC16_calc(uint8_t *buffer, uint8_t buffer_size);
void ModBus_Transmit(MB_handler_t *handler, uint8_t *buffer, uint8_t size);

void ModBus_MountReceivedPacket(MB_handler_t *handler/*, MB_packet_t *packet_info*//*,  uint8_t size, MB_Buffer_t *buffer_rx*/);

#endif /* MODBUS_MODBUS_H_ */
