/*
 * max485.c
 *
 *  Created on: Nov 24, 2026
 *      Author: kenny
 */

#include "max485.h"

MAX_handler_t max_handler;

void Max_HandlerConfig(	UART_HandleTypeDef* huart, GPIO_TypeDef* gpio_port, uint16_t gpio_pin)
{
	max_handler.huart = huart;
	max_handler.gpio_port = gpio_port;
	max_handler.gpio_pin = gpio_pin;
}

ModBus_Error_t ModBus_Transmit(uint8_t *buffer, uint8_t size)
{

	HAL_GPIO_WritePin(max_handler.gpio_port, max_handler.gpio_pin, 1);
	HAL_UART_Transmit(max_handler.huart, buffer, size, 100);
	while (__HAL_UART_GET_FLAG(max_handler.huart, UART_FLAG_TC) == RESET);
	HAL_GPIO_WritePin(max_handler.gpio_port, max_handler.gpio_pin, 0);

	return MODBUS_OK;

}
