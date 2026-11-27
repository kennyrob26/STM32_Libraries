/*
 * modbus_interface_config.h
 *
 *  Created on: Nov 25, 2026
 *      Author: kenny
 */

#ifndef MODBUS_SLAVE_LIBRARY_FOR_STM32_INC_MODBUS_INTERFACE_CONFIG_H_
#define MODBUS_SLAVE_LIBRARY_FOR_STM32_INC_MODBUS_INTERFACE_CONFIG_H_

#include "modbus.h"

//static ModBus_Error_t  port_ModBusTransmit(uint8_t *buffer, uint8_t size);

void modbus_port_init(MB_handler_t *handler);


#endif /* MODBUS_SLAVE_LIBRARY_FOR_STM32_INC_MODBUS_INTERFACE_CONFIG_H_ */

