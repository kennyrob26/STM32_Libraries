/*
 * max485.c
 *
 *  Created on: Nov 24, 2026
 *      Author: kenny
 */

#include "max485.h"

MAX_handler_t max485_handler;



void Max_HandlerConfig(	UART_HandleTypeDef* huart, GPIO_TypeDef* gpio_port, uint16_t gpio_pin, MB_handler_t *handler_mb)
{
	max485_handler.huart = huart;
	max485_handler.gpio_port = gpio_port;
	max485_handler.gpio_pin = gpio_pin;
	max485_handler.handler_mb = handler_mb;
}

/*
void Max_ReceivePacketDMA(uint16_t size)
{
	max485_handler.handler_mb->packet_flag = 1;

	max485_handler.flag = 1;
	max485_handler.rx_size = size;

	HAL_UARTEx_ReceiveToIdle_DMA(max485_handler.huart, max485_handler.handler_mb->buffer_rx, sizeof(max485_handler.handler_mb->buffer_rx));
}
*/


void ModBus_Receive(uint16_t size)
{
	max485_handler.handler_mb->packet_flag = 1;
	max485_handler.handler_mb->packet_size = size;

	HAL_UARTEx_ReceiveToIdle_DMA(max485_handler.huart, max485_handler.handler_mb->buffer_rx, sizeof(max485_handler.handler_mb->buffer_rx));

}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == max485_handler.huart)
	{
		ModBus_Receive(Size);
	}
}


uint8_t Max485_GetFlag()
{
	return max485_handler.flag;
}

void Max485_ResetFlag()
{
	max485_handler.flag = 0;
}

ModBus_Error_t ModBus_Transmit(uint8_t *buffer, uint8_t size)
{

	HAL_GPIO_WritePin(max485_handler.gpio_port, max485_handler.gpio_pin, 1);
	HAL_UART_Transmit(max485_handler.huart, buffer, size, 100);
	while (__HAL_UART_GET_FLAG(max485_handler.huart, UART_FLAG_TC) == RESET);
	HAL_GPIO_WritePin(max485_handler.gpio_port, max485_handler.gpio_pin, 0);

	return MODBUS_OK;

}



