/*
 * modbus_interface_config.c
 *
 *  Created on: Nov 26, 2026
 *      Author: kenny
 */

#include "modbus_interface_config.h"
#include "max485.h"

static ModBus_Error_t  port_ModBusTransmit(uint8_t *buffer, uint8_t size)
{
	return ModBus_Transmit(buffer, size);
}

void modbus_port_init(MB_handler_t *handler)
{

	ModBus_Functions_t functions =
	{
			.ModBus_TransmitPacket = port_ModBusTransmit,
	};

	ModBus_Init(handler, &functions);

}
