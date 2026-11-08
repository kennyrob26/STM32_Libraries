/*
 * modbus.c
 *
 *  Created on: Nov 7, 2026
 *      Author: kenny
 */

#include "modbus.h"


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

//    crc_response[0] = (uint8_t)(buffer_crc & 0x00FF);			//O byte menos significativos primeiro
//    crc_response[1] = (uint8_t)((buffer_crc >> 8) & 0x00FF);

    //printf("resultado: %x\n", buffer_crc);
}


void ModBus_Transmit(MB_handler_t *handler, uint8_t *buffer, uint8_t size)
{
	HAL_GPIO_WritePin(handler->gpio_port, handler->gpio_pin, 1);
	HAL_UART_Transmit(handler->huart, buffer, size, 100);
	//HAL_Delay(20);
	HAL_GPIO_WritePin(handler->gpio_port, handler->gpio_pin, 0);
}

static inline uint16_t calc_quantity(uint8_t *data_payload)
{
	uint16_t quantity = 0;

	quantity  = (data_payload[0] << 8);
	quantity |= data_payload[1];

	return quantity;
}

void ModBus_MountReceivedPacket(MB_handler_t *handler, MB_packet_raw_t *packet_raw, MB_packet_t *packet_info, uint8_t size, MB_Buffer_t *buffer_rx)
{
	packet_info->slave_id = packet_raw->head.slave_id;
	packet_info->function_code = packet_raw->head.function_code;
	packet_info->start_address = (uint16_t)(packet_raw->head.start_addres[0] << 8);
	packet_info->start_address |= packet_raw->head.start_addres[1];
	//packet_info->crc[0] = (buffer_rx->raw[size-2]);
	//packet_info->crc[1] = (buffer_rx->raw[size-1]);
	packet_info->crc = buffer_rx->raw[size-2];
	packet_info->crc |= ((uint16_t)(buffer_rx->raw[size-1] << 8) & 0xFF00);


	uint16_t crc16 = CRC16_calc(buffer_rx->raw, (size-2));

	crc = crc16;

	if(packet_info->crc == crc16)
	{

	}
//    packet_info->crc[0] = (uint8_t)(crc16 & 0x00FF);			//O byte menos significativos primeiro
//    packet_info->crc[1] = (uint8_t)((crc16 >> 8) & 0x00FF);


	switch (packet_raw->head.function_code)
	{
		case FC01:
		case FC02:
		case FC03:
		case FC04:
			packet_info->quantity = calc_quantity(packet_raw->payload);
		break;

		case FC05:
			packet_info->quantity = 0;
			packet_info->byte_count = 0;
			//packet_info->data.coil[0]  = packet_raw->payload[0];
			if(packet_info->start_address < sizeof(handler->table.coils))
				handler->table.coils[packet_info->start_address] = packet_raw->payload[0];

			ModBus_Transmit(handler, buffer_rx->raw, size);

		break;
		case FC06:
			packet_info->quantity = 0;
			packet_info->byte_count = 0;
			packet_info->data.reg[0] = (packet_raw->payload[0] << 8);
			packet_info->data.reg[0] |= packet_raw->payload[1];

			handler->table.registers[packet_info->start_address] = (packet_raw->payload[0] << 8);
			handler->table.registers[packet_info->start_address] |= packet_raw->payload[1];

		break;
		case FC15:
			packet_info->quantity = calc_quantity(packet_raw->payload);
			packet_info->byte_count = packet_raw->payload[2];

			uint8_t mask;

			for(uint8_t j=0; j<packet_info->byte_count; j++)
			{
				for(uint8_t i=0; i<8; i++)
				{
					mask = (1<<i);
					packet_info->data.coil[i +(j*8)]  = ((packet_raw->payload[j+3] & mask) > 0);
				}
			}
			break;

		case FC16:
			packet_info->quantity = calc_quantity(packet_raw->payload);
			packet_info->byte_count = packet_raw->payload[2];

			for(uint8_t i=0; i<packet_info->quantity; i++)
			{
				uint8_t high_byte_index = (i * 2) + 3;
				uint8_t low_byte_index  = high_byte_index + 1;
				packet_info->data.reg[i]  = ((packet_raw->payload[high_byte_index]) << 8);
				packet_info->data.reg[i]  |= (packet_raw->payload[low_byte_index]);
			}
		break;
		default:
			break;
	}
}
