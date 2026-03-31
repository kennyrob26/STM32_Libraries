/*
 * Lcd1602.h
 *
 *  Created on: Mar 30, 2026
 *      Author: kenny
 */

#ifndef INC_LCD1602_H_
#define INC_LCD1602_H_

#include "stm32f4xx_hal.h"



typedef struct{
	GPIO_TypeDef *port;
	uint16_t pin;
}LCD_GPIO;

typedef struct{
	LCD_GPIO db0;
	LCD_GPIO db1;
	LCD_GPIO db2;
	LCD_GPIO db3;
	LCD_GPIO db4;
	LCD_GPIO db5;
	LCD_GPIO db6;
	LCD_GPIO db7;
	LCD_GPIO RS;
	LCD_GPIO E;
	LCD_GPIO RW;
}LCD_Pins;

typedef enum{
	LCD_INTERFACE_4BIT = 0,
	LCD_INTERFACE_8BIT = 1,
	LCD_INTERFACE_I2C  = 2
}LCD_INTERFACE;

typedef struct
{
	LCD_Pins pin;
	LCD_INTERFACE interface;
	TIM_HandleTypeDef *tim;
}LCD_TypeDef;

typedef enum{
	LCD_OK                       = 0,
	LCD_ERROR_HADLE_NOT_DEFINED  = 1,
	LCD_ERROR_INCORRECT_PARAM    = 2,
	LCD_ERROR_TIMEOUT            = 3,
}LCD_ERROR;

typedef enum{
	LCD_HIGH_NIBBLE = 0,
	LCD_LOW_NIBBLE  = 1
}LCD_Nibble_Type;

typedef enum{
	LCD_RS_CONTROL = 0,
	LCD_RS_DATA    = 1
}LCD_RS;

LCD_ERROR LCD_SetPin_DB0(LCD_TypeDef *lcd, GPIO_TypeDef *db0_port, uint16_t db0_pin);
LCD_ERROR LCD_SetPin_DB1(LCD_TypeDef *lcd, GPIO_TypeDef *db1_port, uint16_t db1_pin);
LCD_ERROR LCD_SetPin_DB2(LCD_TypeDef *lcd, GPIO_TypeDef *db2_port, uint16_t db2_pin);
LCD_ERROR LCD_SetPin_DB3(LCD_TypeDef *lcd, GPIO_TypeDef *db3_port, uint16_t db3_pin);
LCD_ERROR LCD_SetPin_DB4(LCD_TypeDef *lcd, GPIO_TypeDef *db4_port, uint16_t db4_pin);
LCD_ERROR LCD_SetPin_DB5(LCD_TypeDef *lcd, GPIO_TypeDef *db5_port, uint16_t db5_pin);
LCD_ERROR LCD_SetPin_DB6(LCD_TypeDef *lcd, GPIO_TypeDef *db6_port, uint16_t db6_pin);
LCD_ERROR LCD_SetPin_DB7(LCD_TypeDef *lcd, GPIO_TypeDef *db7_port, uint16_t db7_pin);
LCD_ERROR LCD_SetPin_RS(LCD_TypeDef *lcd, GPIO_TypeDef *rs_port, uint16_t rs_pin);
LCD_ERROR LCD_SetPin_Enable(LCD_TypeDef *lcd, GPIO_TypeDef *enable_port, uint16_t enable_pin);
LCD_ERROR LCD_SetPin_RW(LCD_TypeDef *lcd, GPIO_TypeDef *rw_port, uint16_t rw_pin);
LCD_ERROR LCD_Send_CMD(LCD_TypeDef *lcd, uint8_t cmd);
LCD_ERROR LCD_CMD_FunctionSet(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface);
LCD_ERROR LCD_CMD_OnOff(LCD_TypeDef *lcd, uint8_t display_on_off, uint8_t cursor, uint8_t blink_cursor);
LCD_ERROR LCD_Init(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface, TIM_HandleTypeDef *tim);
LCD_ERROR LCD_Send_Data(LCD_TypeDef *lcd, uint8_t data);

#endif /* INC_LCD1602_H_ */
