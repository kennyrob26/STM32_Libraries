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

typedef enum
{
	LCD_LINE1_ADDRESS = 0x00,
	LCD_LINE2_ADDRESS = 0x40,
	LCD_LINE3_ADDRESS = 0x10,
	LCD_LINE4_ADDRESS = 0x50
}LCD_Line_Adress;

typedef enum
{
	LCD_LINE1 = 0,
	LCD_LINE2 = 1,
	LCD_LINE3 = 2,
	LCD_LINE4 = 3
}LCD_Line;


typedef struct{
	uint8_t max_lines;
	uint8_t max_columns;
}LCD_DisplaySize;

typedef enum{
	LCD_FORMAT_16_02 = 0,
	LCD_FORMAT_20_02 = 1,
	LCD_FORMAT_16_04 = 2,
	LCD_FORMAT_20_04 = 3
}LCD_DisplayFormat;

typedef enum{
	LCD_AUTO_LINE_BREAK_DISABLE = 0,
	LCD_AUTO_LINE_BREAK_ENABLE  = 1
}LCD_AutoLineBreak;

typedef enum
{
	LCD_DISPLAY_OFF = 0,
	LCD_DISPLAY_ON  = 1
}LCD_DisplayStatus;

typedef enum
{
	LCD_CURSOR_DISABLE   = 0,
	LCD_CURSOR_ENABLE    = 1
}LCD_CursorStatus;

typedef enum
{
	LCD_BLINK_CURSOR_DISABLE   = 0,
	LCD_BLINK_CURSOR_ENABLE    = 1
}LCD_CursorBlinkStatus;

typedef struct
{
	LCD_DisplayStatus display_status;
	LCD_CursorStatus cursor_status;
	LCD_CursorBlinkStatus cursor_blink_status;
}LCD_CmdOnOff;

typedef struct
{
	uint8_t x;
	uint8_t y;
}LCD_CursorPos;

typedef struct
{
	LCD_Pins pin;
	LCD_INTERFACE interface;
	I2C_HandleTypeDef *i2c;
	LCD_CmdOnOff cmd_onOff;
	LCD_DisplaySize size;
	TIM_HandleTypeDef *tim;
	LCD_CursorPos cursor;
	//uint8_t cursor_x;
	//uint8_t cursor_y;
	LCD_AutoLineBreak auto_line_break;
}LCD_TypeDef;

typedef enum{
	LCD_OK                       	 = 0,
	LCD_ERROR_HADLE_NOT_DEFINED  	 = 1,
	LCD_ERROR_I2C_HANDLE_NOT_DEFINED = 2,
	LCD_ERROR_INCORRECT_PARAM        = 3,
	LCD_ERROR_TIMEOUT            	 = 4,
	LCD_ERROR_					 	 = 5
}LCD_ERROR;

typedef enum{
	LCD_HIGH_NIBBLE = 0,
	LCD_LOW_NIBBLE  = 1
}LCD_Nibble_Type;

typedef enum{
	LCD_RS_CONTROL = 0,
	LCD_RS_DATA    = 1
}LCD_RS;

typedef struct
{
	LCD_TypeDef *lcd;
	uint8_t x1;
	uint8_t y1;
	uint8_t x2;
	uint8_t y2;
	uint8_t lines;
	uint8_t line_length;
	uint8_t lentgth;
}LCD_Area;

LCD_ERROR LCD_GPIO_SetDB0(LCD_TypeDef *lcd, GPIO_TypeDef *db0_port, uint16_t db0_pin);
LCD_ERROR LCD_GPIO_SetDB1(LCD_TypeDef *lcd, GPIO_TypeDef *db1_port, uint16_t db1_pin);
LCD_ERROR LCD_GPIO_SetDB2(LCD_TypeDef *lcd, GPIO_TypeDef *db2_port, uint16_t db2_pin);
LCD_ERROR LCD_GPIO_SetDB3(LCD_TypeDef *lcd, GPIO_TypeDef *db3_port, uint16_t db3_pin);
LCD_ERROR LCD_GPIO_SetDB4(LCD_TypeDef *lcd, GPIO_TypeDef *db4_port, uint16_t db4_pin);
LCD_ERROR LCD_GPIO_SetDB5(LCD_TypeDef *lcd, GPIO_TypeDef *db5_port, uint16_t db5_pin);
LCD_ERROR LCD_GPIO_SetDB6(LCD_TypeDef *lcd, GPIO_TypeDef *db6_port, uint16_t db6_pin);
LCD_ERROR LCD_GPIO_SetDB7(LCD_TypeDef *lcd, GPIO_TypeDef *db7_port, uint16_t db7_pin);
LCD_ERROR LCD_GPIO_SetRS(LCD_TypeDef *lcd, GPIO_TypeDef *rs_port, uint16_t rs_pin);
LCD_ERROR LCD_GPIO_SetEnable(LCD_TypeDef *lcd, GPIO_TypeDef *enable_port, uint16_t enable_pin);
LCD_ERROR LCD_GPIO_SetRW(LCD_TypeDef *lcd, GPIO_TypeDef *rw_port, uint16_t rw_pin);
LCD_ERROR LCD_I2C_SetI2CHandle(LCD_TypeDef *lcd, I2C_HandleTypeDef *i2c);

LCD_ERROR LCD_Send_CMD(LCD_TypeDef *lcd, uint8_t cmd);
LCD_ERROR LCD_CMD_FunctionSet(LCD_TypeDef *lcd);

LCD_ERROR LCD_CMD_DisplayOnOff(LCD_TypeDef *lcd, LCD_DisplayStatus display_status, LCD_CursorStatus cursor_status, LCD_CursorBlinkStatus blink_status);
LCD_ERROR LCD_Display_SetSize(LCD_TypeDef *lcd, uint8_t columns, uint8_t lines);
LCD_ERROR LCD_Display_SetFormat(LCD_TypeDef *lcd, LCD_DisplayFormat format);
LCD_ERROR LCD_Display_Enable(LCD_TypeDef *lcd);
LCD_ERROR LCD_Display_Disable(LCD_TypeDef *lcd);

LCD_ERROR LCD_Cursor_SetPos(LCD_TypeDef *lcd, uint8_t x, uint8_t y);
LCD_ERROR LCD_Cursor_Enable(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_Disable(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_BlinkEnable(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_BlinkDisable(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_MovetoLeft(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_MovetoRight(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_MovetoUp(LCD_TypeDef *lcd);
LCD_ERROR LCD_Cursor_MovetoDown(LCD_TypeDef *lcd);

LCD_ERROR LCD_Init(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface, TIM_HandleTypeDef *tim);
LCD_ERROR LCD_PutChar(LCD_TypeDef *lcd, uint8_t data);
LCD_ERROR LCD_Print(LCD_TypeDef *lcd, const char *string);
LCD_ERROR LCD_PrintF(LCD_TypeDef *lcd, const char *stringf, ...);
LCD_ERROR LCD_Backspace(LCD_TypeDef *lcd);
LCD_ERROR LCD_LineBreak(LCD_TypeDef *lcd);
LCD_ERROR LCD_Tab(LCD_TypeDef *lcd);
LCD_ERROR LCD_CarrigeReturn(LCD_TypeDef *lcd);

LCD_ERROR LCD_Clear_Char(LCD_TypeDef *lcd);
LCD_ERROR LCD_Clear_Line(LCD_TypeDef *lcd, uint8_t line);
LCD_ERROR LCD_Clear_Display(LCD_TypeDef *lcd);

LCD_ERROR LCD_Area_CreateNew(LCD_TypeDef *lcd, LCD_Area *area);
LCD_ERROR LCD_Clear_Area(LCD_Area *area);
LCD_ERROR LCD_Area_Update(LCD_Area *area, uint8_t string[]);


#endif /* INC_LCD1602_H_ */
