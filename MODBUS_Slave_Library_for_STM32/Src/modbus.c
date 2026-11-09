/*
 * modbus.c
 *
 *  Created on: Nov 7, 2026
 *      Author: kenny
 */

#include "modbus.h"
#include "string.h"


#define CRC16_POLYNOMINAL 0xA001

uint16_t crc;


uint16_t  CRC16_calc(uint8_t *buffer, uint8_t buffer_size)
{
    uint16_t buffer_crc = 0xFFFF;
    uint8_t last_bit = 0;

    for (uint8_t i=0; i<buffer_size; i++)
    {
        buffer_crc = buffer_crc ^ buffer[i];

        uint8_t current_bit = 0;
        while(current_bit < 8)
        {
            last_bit = (uint8_t)(buffer_crc & 0x0001);

            buffer_crc = buffer_crc >> 1;

            if(last_bit == 1)
                buffer_crc = buffer_crc ^ CRC16_POLYNOMINAL;

            current_bit++;
        }
    }

    return buffer_crc;
}


void ModBus_Transmit(MB_handler_t *handler, uint8_t *buffer, uint8_t size)
{
	HAL_GPIO_WritePin(handler->gpio_port, handler->gpio_pin, 1);
	HAL_UART_Transmit(handler->huart, buffer, size, 100);
	//HAL_Delay(20);
	HAL_GPIO_WritePin(handler->gpio_port, handler->gpio_pin, 0);
}

void ModBus_ResponseToMaster(MB_handler_t *handler, uint8_t response_ok)
{
	switch (handler->packet_info.function_code)
	{
		case FC05:												//Response ID+FC+Address+Data+CRC
		case FC06:
			ModBus_Transmit(handler, handler->buffer_rx, handler->packet_size);
		break;

		case FC15:												//Response 8 bytes: ID+FC+Address+Quantity+CRC
		case FC16:
			uint8_t response[8];
			memcpy(response, handler->buffer_rx, 6);

			uint16_t crc16 = CRC16_calc(response, 6);

			response[6] = (uint8_t)(crc16 & 0x00FF);
			response[7] = (uint8_t)((crc16 >> 8) & 0x00FF);

			ModBus_Transmit(handler, response, 8);
		break;

		default:
			break;

	}
}

static inline uint16_t calc_quantity(uint8_t *data_payload)
{
	uint16_t quantity = 0;

	quantity  = (data_payload[4] << 8);
	quantity |= data_payload[5];

	return quantity;
}

void ModBus_MountReceivedPacket(MB_handler_t *handler/*, MB_packet_t *packet_info*/)
{
	handler->packet_info.slave_id = handler->buffer_rx[0];
	handler->packet_info.function_code = handler->buffer_rx[1];
	handler->packet_info.start_address = (uint16_t)(handler->buffer_rx[2] << 8);
	handler->packet_info.start_address |= handler->buffer_rx[3];


	handler->packet_info.crc = handler->buffer_rx[handler->packet_size-2];
	handler->packet_info.crc |= ((uint16_t)(handler->buffer_rx[handler->packet_size-1] << 8) & 0xFF00);

	uint16_t crc16 = CRC16_calc(handler->buffer_rx, (handler->packet_size-2));

	crc = crc16;

	if(handler->packet_info.crc == crc16)
	{

	}

	switch (handler->packet_info.function_code)
	{
		case FC01:
		case FC02:
		case FC03:
		case FC04:
			handler->packet_info.quantity = calc_quantity(handler->buffer_rx);
		break;

		case FC05:
			handler->packet_info.quantity = 0;
			handler->packet_info.byte_count = 0;

			if(handler->packet_info.start_address < sizeof(handler->table.coils))
				handler->table.coils[handler->packet_info.start_address] = handler->buffer_rx[4];

			//ModBus_Transmit(handler, handler->buffer_rx, handler->packet_size);

		break;
		case FC06:
			handler->packet_info.quantity = 0;
			handler->packet_info.byte_count = 0;

			handler->table.registers[handler->packet_info.start_address] = (handler->buffer_rx[4] << 8);
			handler->table.registers[handler->packet_info.start_address] |= handler->buffer_rx[5];

			//ModBus_Transmit(handler, handler->buffer_rx, handler->packet_size);

		break;
		case FC15:
			handler->packet_info.quantity = calc_quantity(handler->buffer_rx);
			handler->packet_info.byte_count = handler->buffer_rx[6];

			uint8_t mask;

			for(uint8_t j=0; j<handler->packet_info.byte_count; j++)
			{
				for(uint8_t i=0; i<8; i++)
				{
					mask = (1<<i);

					handler->table.coils[i +(j*8)]  = ((handler->buffer_rx[j+7] & mask) > 0);
				}
			}

			//ModBus_Transmit(handler, handler->buffer_rx, handler->packet_size);

			break;

		case FC16:
			handler->packet_info.quantity = calc_quantity(handler->buffer_rx);
			handler->packet_info.byte_count = handler->buffer_rx[6];

			for(uint8_t i=0; i<handler->packet_info.quantity; i++)
			{
				uint8_t high_byte_index = (i * 2) + 3;
				uint8_t low_byte_index  = high_byte_index + 1;

				handler->table.registers[handler->packet_info.start_address + i]  = ((handler->buffer_rx[high_byte_index + 4]) << 8);
				handler->table.registers[handler->packet_info.start_address + i]  |= (handler->buffer_rx[low_byte_index + 4]);
			}
			uint8_t buffer_tx[8];
			memcpy(buffer_tx, handler->buffer_rx, 6);

			crc16 = CRC16_calc(buffer_tx, 6);

			buffer_tx[6] = (uint8_t)(crc16 & 0x00FF);
			buffer_tx[7] = (uint8_t)((crc16 >> 8) & 0x00FF);

			//ModBus_Transmit(handler, buffer_tx, 8);
		break;
		default:
			break;
	}

	ModBus_ResponseToMaster(handler, 1);
}








