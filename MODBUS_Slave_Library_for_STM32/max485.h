/*
 * max485.h
 *
 *  Created on: Nov 24, 2026
 *      Author: kenny
 */

#ifndef MODBUS_SLAVE_LIBRARY_FOR_STM32_MAX485_H_
#define MODBUS_SLAVE_LIBRARY_FOR_STM32_MAX485_H_

#include "modbus.h"


typedef struct
{
	UART_HandleTypeDef* huart;
	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;

	MB_handler_t *handler_mb;
	uint8_t buffer_rx[50];
	uint8_t rx_size;

	uint8_t flag;
}MAX_handler_t;

void Max_HandlerConfig(	UART_HandleTypeDef* huart, GPIO_TypeDef* gpio_port, uint16_t gpio_pin, MB_handler_t *handler_mb);

uint8_t Max485_GetFlag();
void Max485_ResetFlag();

#endif /* MODBUS_SLAVE_LIBRARY_FOR_STM32_MAX485_H_ */
