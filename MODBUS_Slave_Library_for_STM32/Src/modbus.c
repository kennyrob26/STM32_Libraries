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
		case FC01:
		case FC03:
			ModBus_Transmit(handler, handler->buffer_tx, handler->response_size);
		break;

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

void ModBus_FillPacketInfo(MB_handler_t *handler)
{
	handler->packet_info.slave_id = handler->buffer_rx[0];
	handler->packet_info.function_code = handler->buffer_rx[1];
	handler->packet_info.start_address = (uint16_t)(handler->buffer_rx[2] << 8);
	handler->packet_info.start_address |= handler->buffer_rx[3];

	handler->packet_info.quantity = 0;
	handler->packet_info.byte_count = 0;

	handler->packet_info.crc = handler->buffer_rx[handler->packet_size-2];
	handler->packet_info.crc |= ((uint16_t)(handler->buffer_rx[handler->packet_size-1] << 8) & 0xFF00);
}

uint8_t ModBus_ValidadeCRC(MB_handler_t *handler)
{
	uint16_t crc16 = CRC16_calc(handler->buffer_rx, (handler->packet_size-2));

	if(handler->packet_info.crc == crc16)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void ModBus_ReadTable(MB_handler_t *handler)
{
	handler->packet_info.quantity = calc_quantity(handler->buffer_rx);

	const uint8_t head_bytes = 3; // ID+FC+Count
	uint8_t byte_count = 0; //(uint8_t)((handler->packet_info.quantity + 7) / 8);
	uint8_t response_size = 0;//head_bytes + byte_count;

	uint8_t current_byte;
	uint16_t crc16;

	switch (handler->packet_info.function_code)
	{
		case FC01:

			byte_count = (uint8_t)((handler->packet_info.quantity + 7) / 8);
			response_size = head_bytes + byte_count;

			handler->buffer_tx[0] = handler->packet_info.slave_id;
			handler->buffer_tx[1] = handler->packet_info.function_code;
			handler->buffer_tx[2] = byte_count;

			current_byte    = 0;
			uint8_t current_address = 0;

			for(uint8_t i=0; i<byte_count; i++)
			{
				current_byte = i+3;

				handler->buffer_tx[current_byte] = 0;

				for(uint8_t j=0; j<8; j++)
				{
					current_address = handler->table.coils[j + (i*8) + handler->packet_info.start_address];
					handler->buffer_tx[current_byte] |= (current_address << j);
				}
			}

			crc16 = CRC16_calc(handler->buffer_tx, response_size);

			handler->buffer_tx[response_size] = (uint8_t)(crc16 & 0x00FF);
			handler->buffer_tx[response_size + 1] = (uint8_t)((crc16 >> 8) & 0x00FF);

			handler->response_size = response_size + 2;

		break;
		case FC02:

		break;
		case FC03:
			byte_count = (handler->packet_info.quantity * 2);
			response_size = head_bytes + byte_count;

			handler->buffer_tx[0] = handler->packet_info.slave_id;
			handler->buffer_tx[1] = handler->packet_info.function_code;
			handler->buffer_tx[2] = byte_count;

			current_byte = 0;


			for(uint8_t i=0; i<handler->packet_info.quantity; i++)
			{
				current_byte = (i*2)+3;
				handler->buffer_tx[current_byte]     = (uint8_t)((handler->table.registers[i] >> 8) & 0x00FF);
				handler->buffer_tx[current_byte + 1] = (uint8_t)(handler->table.registers[i] & 0x00FF);
			}

			crc16 = CRC16_calc(handler->buffer_tx, response_size);

			handler->buffer_tx[response_size] = (uint8_t)(crc16 & 0x00FF);
			handler->buffer_tx[response_size + 1] = (uint8_t)((crc16 >> 8) & 0x00FF);

			handler->response_size = response_size + 2;


		break;
		case FC04:

		break;
		default:
			break;
	}
}

void ModBus_MountReceivedPacket(MB_handler_t *handler/*, MB_packet_t *packet_info*/)
{

	ModBus_FillPacketInfo(handler);

	if(ModBus_ValidadeCRC(handler) == 1)
	{
		switch (handler->packet_info.function_code)
		{
			case FC01:
			case FC02:
			case FC03:
			case FC04:
				//handler->packet_info.quantity = calc_quantity(handler->buffer_rx);
				ModBus_ReadTable(handler);
			break;

			case FC05:
				if(handler->packet_info.start_address < sizeof(handler->table.coils))
					handler->table.coils[handler->packet_info.start_address] = handler->buffer_rx[4];

			break;
			case FC06:
				if(handler->packet_info.start_address < sizeof(handler->table.registers))
				{
					handler->table.registers[handler->packet_info.start_address] = (handler->buffer_rx[4] << 8);
					handler->table.registers[handler->packet_info.start_address] |= handler->buffer_rx[5];
				}

			break;
			case FC15:
				handler->packet_info.quantity = calc_quantity(handler->buffer_rx);
				handler->packet_info.byte_count = handler->buffer_rx[6];

				uint8_t mask;

				for(uint16_t j=0; j<handler->packet_info.byte_count; j++)
				{
					uint16_t index = handler->packet_info.start_address + (j * 8);

					for(uint16_t i=0; i<8; i++)
					{
						mask = (1<<i);

						if((index+i) < handler->packet_info.quantity)
							handler->table.coils[index + i]  = ((handler->buffer_rx[j+7] & mask) > 0);
						else
							break;
					}
				}

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

			break;
			default:
				break;
		}

		ModBus_ResponseToMaster(handler, 1);

	}
}








