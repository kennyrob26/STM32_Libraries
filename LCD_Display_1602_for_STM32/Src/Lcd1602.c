/*
 * Lcd1602.c
 *
 *  Created on: Mar 30, 2026
 *      Author: kenny
 */


#include "Lcd1602.h"

#include "stdarg.h"
#include "stdio.h"

#define I2C_WRITE 0x4E


TIM_HandleTypeDef *lcd_tim;


static inline void cursor_x_increment(LCD_TypeDef *lcd)
{
	if(lcd->cursor_x < (lcd->size.max_lines - 1))
		lcd->cursor_x++;
	else
		lcd->cursor_x = (lcd->size.max_lines - 1);
}

static inline void cursor_x_decrement(LCD_TypeDef *lcd)
{
	if(lcd->cursor_x > 0 && lcd->cursor_x < 6) //6 of buffer overflow (-1 = 255, -2 = 254...)
		lcd->cursor_x--;
	else
		lcd->cursor_x = 0;
}

static inline void cursor_y_increment(LCD_TypeDef *lcd)
{
	if(lcd->cursor_y < (lcd->size.max_columns -1))
		lcd->cursor_y++;
	else
		lcd->cursor_y = (lcd->size.max_columns - 1);
}

static inline void cursor_y_decrement(LCD_TypeDef *lcd)
{
	if(lcd->cursor_y > 0 && lcd->cursor_y < 30) //30 of buffer overflow (-1 = 255, -2 = 254...)
		lcd->cursor_y--;
	else
		lcd->cursor_y = 0;
}


static inline void LCD_Delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(lcd_tim, 0); // Reset TIM counter

	while(__HAL_TIM_GET_COUNTER(lcd_tim) < us)
	{/*Wait time*/};
}

static inline uint8_t LCD_Check_EscapeSequence(LCD_TypeDef *lcd, uint8_t data)
{
	switch (data) {
		case '\n':
			LCD_LineBreak(lcd);
			return 1;
		break;
		case '\b':
			LCD_Backspace(lcd);
			return 1;
		break;
		case '\t':
			LCD_Tab(lcd);
			return 1;
		break;
		case '\r':
			LCD_CarrigeReturn(lcd);
			return 1;
		default:
			return 0;
			break;
	}
}

LCD_ERROR LCD_GPIO_SetDB0(LCD_TypeDef *lcd, GPIO_TypeDef *db0_port, uint16_t db0_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db0_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db0.port = db0_port;
	lcd->pin.db0.pin  = db0_pin;
	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB1(LCD_TypeDef *lcd, GPIO_TypeDef *db1_port, uint16_t db1_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db1_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db1.port = db1_port;
	lcd->pin.db1.pin  = db1_pin;

	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB2(LCD_TypeDef *lcd, GPIO_TypeDef *db2_port, uint16_t db2_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db2_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db2.port = db2_port;
	lcd->pin.db2.pin  = db2_pin;

	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB3(LCD_TypeDef *lcd, GPIO_TypeDef *db3_port, uint16_t db3_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db3_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db3.port = db3_port;
	lcd->pin.db3.pin  = db3_pin;
	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB4(LCD_TypeDef *lcd, GPIO_TypeDef *db4_port, uint16_t db4_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db4_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db4.port = db4_port;
	lcd->pin.db4.pin  = db4_pin;
	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB5(LCD_TypeDef *lcd, GPIO_TypeDef *db5_port, uint16_t db5_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db5_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db5.port = db5_port;
	lcd->pin.db5.pin  = db5_pin;

	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB6(LCD_TypeDef *lcd, GPIO_TypeDef *db6_port, uint16_t db6_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db6_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db6.port = db6_port;
	lcd->pin.db6.pin  = db6_pin;
	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetDB7(LCD_TypeDef *lcd, GPIO_TypeDef *db7_port, uint16_t db7_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db7_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db7.port = db7_port;
	lcd->pin.db7.pin  = db7_pin;
	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetRS(LCD_TypeDef *lcd, GPIO_TypeDef *rs_port, uint16_t rs_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(rs_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.RS.port = rs_port;
	lcd->pin.RS.pin  = rs_pin;

	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetEnable(LCD_TypeDef *lcd, GPIO_TypeDef *enable_port, uint16_t enable_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(enable_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.E.port = enable_port;
	lcd->pin.E.pin  = enable_pin;
	return LCD_OK;
}
LCD_ERROR LCD_GPIO_SetRW(LCD_TypeDef *lcd, GPIO_TypeDef *rw_port, uint16_t rw_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(rw_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.RW.port = rw_port;
	lcd->pin.RW.pin  = rw_pin;
	return LCD_OK;
}

LCD_ERROR LCD_I2C_SetI2CHandle(LCD_TypeDef *lcd, I2C_HandleTypeDef *i2c)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(i2c == NULL)
		return LCD_ERROR_;

	lcd->i2c = i2c;

	return LCD_OK;
}

/*
static LCD_ERROR LCD_Send_Byte(LCD_TypeDef *lcd, uint8_t byte, LCD_RS rs)
{

	uint8_t cmd[3];
	cmd[0] = I2C_WRITE;
	cmd[1] = (rs << 6);
	cmd[2] = byte;
	HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, cmd, sizeof(cmd), 100);

	return LCD_OK;
}
*/
static LCD_ERROR LCD_Send_Nibble(LCD_TypeDef *lcd, uint8_t nibble, LCD_Nibble_Type nibble_type, LCD_RS rs)
{


	if(lcd->interface == LCD_INTERFACE_4BIT)
	{
		uint8_t d7_value = (nibble >> 3) & 0x01;
		uint8_t d6_value = (nibble >> 2) & 0x01;
		uint8_t d5_value = (nibble >> 1) & 0x01;
		uint8_t d4_value = nibble & 0x01;

		HAL_GPIO_WritePin(lcd->pin.RW.port, lcd->pin.RW.pin, 0);
		HAL_GPIO_WritePin(lcd->pin.RS.port, lcd->pin.RS.pin, rs);
		LCD_Delay_us(1);

		HAL_GPIO_WritePin(lcd->pin.db7.port, lcd->pin.db7.pin, d7_value);
		HAL_GPIO_WritePin(lcd->pin.db6.port, lcd->pin.db6.pin, d6_value);
		HAL_GPIO_WritePin(lcd->pin.db5.port, lcd->pin.db5.pin, d5_value);
		HAL_GPIO_WritePin(lcd->pin.db4.port, lcd->pin.db4.pin, d4_value);

		LCD_Delay_us(1);

		HAL_GPIO_WritePin(lcd->pin.E.port, lcd->pin.E.pin, 1);

		LCD_Delay_us(1);

		HAL_GPIO_WritePin(lcd->pin.E.port, lcd->pin.E.pin, 0);
	}
	else if(lcd->interface == LCD_INTERFACE_I2C)
	{

		uint8_t cmd_nibble = (nibble << 4);
		cmd_nibble |= (rs << 0);
		cmd_nibble |= (1 << 3);

		//uint8_t cmd[2];
		//cmd[0] = I2C_WRITE;
		//cmd[1] = cmd_nibble;
		cmd_nibble &= ~(1<<2);
		HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, &cmd_nibble, 1, 100);

		LCD_Delay_us(50);


		cmd_nibble |= (1 << 2);

		HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, &cmd_nibble, 1, 100);

		LCD_Delay_us(50);

		cmd_nibble &= ~(1 << 2);  //reset e bit, equals a xor

		HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, &cmd_nibble, 1, 100);

		LCD_Delay_us(50);

	}



	return LCD_OK;

}

LCD_ERROR LCD_Send_CMD(LCD_TypeDef *lcd, uint8_t cmd)
{
	//if(lcd->interface == LCD_INTERFACE_4BIT)

	uint8_t high_nibble = (cmd >> 4) & 0x0F;
	uint8_t low_nibble  = cmd & 0x0F;
	LCD_Send_Nibble(lcd, high_nibble, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
	LCD_Send_Nibble(lcd, low_nibble, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);


	return LCD_OK;
}

LCD_ERROR LCD_CMD_FunctionSet(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface)
{
	uint8_t command = 0x28;

	/*
	if(lcd_interface == LCD_INTERFACE_4BIT || lcd_interface == LCD_INTERFACE_I2C)
	{
		command = 0x28;
	}
	else
	{
		command = 0x38;
	}*/

	LCD_Send_CMD(lcd, command);

	LCD_Delay_us(100);
	return LCD_OK;
}

static inline LCD_ERROR LCD_UpdateOnOff(LCD_TypeDef *lcd)
{
	uint8_t command = 0x08;

	command |= ((lcd->display_status & 0x01) << 2);
	command |= ((lcd->cursor_status & 0x01) << 1);
	command |= (lcd->cursor_blink_status & 0x01);

	LCD_Send_CMD(lcd, command);

	return LCD_OK;
}

LCD_ERROR LCD_CMD_OnOff(LCD_TypeDef *lcd, uint8_t display_on_off, uint8_t cursor, uint8_t blink_cursor)
{
	lcd->display_status = display_on_off;
	lcd->cursor_status = cursor;
	lcd->cursor_blink_status = blink_cursor;

	LCD_UpdateOnOff(lcd);

	return LCD_OK;
}

LCD_ERROR LCD_Display_SetSize(LCD_TypeDef *lcd, uint8_t columns, uint8_t lines)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(columns > 20 || lines > 4)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->size.max_columns = columns;
	lcd->size.max_lines   = lines;

	return LCD_OK;
}

LCD_ERROR LCD_Display_SetFormat(LCD_TypeDef *lcd, LCD_DisplayFormat format)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	switch (format)
	{
		case LCD_FORMAT_16_02:
			return LCD_Display_SetSize(lcd, 16, 2);
		case LCD_FORMAT_16_04:
			return LCD_Display_SetSize(lcd, 16, 4);
		case LCD_FORMAT_20_02:
			return LCD_Display_SetSize(lcd, 20, 2);
		case LCD_FORMAT_20_04:
			return LCD_Display_SetSize(lcd, 20, 4);
		default:
			return LCD_ERROR_INCORRECT_PARAM;
	}
}
LCD_ERROR LCD_Display_Enable(LCD_TypeDef *lcd)
{
	lcd->display_status = LCD_DISPLAY_ON;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Display_Disable(LCD_TypeDef *lcd)
{
	lcd->display_status = LCD_DISPLAY_OFF;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Cursor_SetPos(LCD_TypeDef *lcd, uint8_t x, uint8_t y)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t cursor_adress;

	if(x >= lcd->size.max_lines || y >= lcd->size.max_columns)
		return LCD_ERROR_;

	if(x == 0)
		cursor_adress = LCD_LINE1_ADDRESS + y;
	else if(x == 1)
		cursor_adress = LCD_LINE2_ADDRESS + y;

	cursor_adress |= 0x80; //1xxxxxxx

	LCD_Send_CMD(lcd, cursor_adress);
	LCD_Delay_us(50);

	lcd->cursor_x = x;
	lcd->cursor_y = y;

	return LCD_OK;
}

LCD_ERROR LCD_Cursor_Enable(LCD_TypeDef *lcd)
{
	lcd->cursor_status = LCD_CURSOR_ENABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Cursor_Disable(LCD_TypeDef *lcd)
{
	lcd->cursor_status = LCD_CURSOR_DISABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Cursor_BlinkEnable(LCD_TypeDef *lcd)
{
	lcd->cursor_blink_status = LCD_BLINK_CURSOR_ENABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}
LCD_ERROR LCD_Cursor_BlinkDisable(LCD_TypeDef *lcd)
{
	lcd->cursor_blink_status = LCD_BLINK_CURSOR_DISABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Cursor_MovetoLeft(LCD_TypeDef *lcd)
{
	cursor_y_decrement(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor_x, lcd->cursor_y);
}

LCD_ERROR LCD_Cursor_MovetoRight(LCD_TypeDef *lcd)
{
	cursor_y_increment(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor_x, lcd->cursor_y);
}

LCD_ERROR LCD_Cursor_MovetoUp(LCD_TypeDef *lcd)
{
	cursor_x_decrement(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor_x, lcd->cursor_y);
}

LCD_ERROR LCD_Cursor_MovetoDown(LCD_TypeDef *lcd)
{
	cursor_x_increment(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor_x, lcd->cursor_y);
}




LCD_ERROR LCD_CMD_DisplayClear(LCD_TypeDef *lcd)
{
	uint8_t cmd = 0x01;

	LCD_Send_CMD(lcd, cmd);

	HAL_Delay(3);  //recommended 1.53ms

	return LCD_OK;

}


LCD_ERROR LCD_SetAutoLineBreak(LCD_TypeDef *lcd, LCD_AutoLineBreak auto_line_break)
{
	lcd->auto_line_break = auto_line_break;
	return LCD_OK;
}


LCD_ERROR LCD_Init(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface, TIM_HandleTypeDef *tim)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	if(lcd->size.max_columns == 0)
		LCD_Display_SetFormat(lcd, LCD_FORMAT_16_02);

	lcd->interface = lcd_interface;

	LCD_SetAutoLineBreak(lcd, LCD_AUTO_LINE_BREAK_ENABLE);
	lcd_tim = tim;

	HAL_Delay(40);

	LCD_Send_Nibble(lcd, 0x03, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
	HAL_Delay(5);

	LCD_CMD_FunctionSet(lcd, lcd_interface);
	LCD_CMD_FunctionSet(lcd, lcd_interface);
	LCD_CMD_OnOff(lcd, LCD_DISPLAY_ON, LCD_CURSOR_DISABLE, LCD_BLINK_CURSOR_DISABLE);
	HAL_Delay(1);
	LCD_CMD_DisplayClear(lcd);
	LCD_Send_CMD(lcd, 0x06);  //Display Entrey default;

	return LCD_OK;
}

static inline LCD_ERROR LCD_CheckEndLine(LCD_TypeDef *lcd)
{
	if(lcd->cursor_y >= lcd->size.max_columns)
	{
		if(lcd->cursor_x < (lcd->size.max_lines - 1))
		{
			if(lcd->auto_line_break == LCD_AUTO_LINE_BREAK_ENABLE)
				LCD_LineBreak(lcd);
		}
		else
		{
			lcd->cursor_y = lcd->size.max_columns;
			return LCD_ERROR_;
		}

	}

	return LCD_OK;
}

LCD_ERROR LCD_PutChar(LCD_TypeDef *lcd, uint8_t data)
{
	if(LCD_CheckEndLine(lcd) != LCD_OK)
		return LCD_ERROR_;

	if(LCD_Check_EscapeSequence(lcd, data))
		return LCD_OK;

	if(lcd->interface == LCD_INTERFACE_4BIT || LCD_INTERFACE_I2C)
	{
		uint8_t high_nibble = (data >> 4) & 0x0F;
		uint8_t low_nibble  = data & 0x0F;
		LCD_Send_Nibble(lcd, high_nibble, LCD_HIGH_NIBBLE, LCD_RS_DATA);
		LCD_Send_Nibble(lcd, low_nibble, LCD_LOW_NIBBLE, LCD_RS_DATA);
	}

	LCD_Delay_us(50);

	lcd->cursor_y++;
	LCD_CheckEndLine(lcd);


	return LCD_OK;
}

LCD_ERROR LCD_Print(LCD_TypeDef *lcd, const char *string)
{
	uint8_t i = 0;
	while(string[i] != '\0')
	{
		if(LCD_PutChar(lcd, string[i]) != LCD_OK)
			break;
		i++;
	}

	return LCD_OK;
}

LCD_ERROR LCD_PrintF(LCD_TypeDef *lcd, const char *stringf, ...)
{
	va_list args; //arg list (...)
	va_start(args, stringf);

	char send_string[32];
	//sprintf(send_string, stringf, args);
	vsnprintf(send_string, sizeof(send_string), stringf, args);
	va_end(args);

	return LCD_Print(lcd, send_string);
}

LCD_ERROR LCD_Backspace(LCD_TypeDef *lcd)
{
	uint8_t previous_x = 0;
	uint8_t previous_y = 0;

	if(lcd->cursor_y > 0)
	{
		previous_x = lcd->cursor_x;
		previous_y = lcd->cursor_y - 1;

	}
	else if(lcd->cursor_x > 0)
	{
		previous_x = (lcd->cursor_x - 1);
		previous_y = (lcd->size.max_columns - 1);
	}
	else
		return LCD_ERROR_;


	LCD_Cursor_SetPos(lcd, previous_x, previous_y);
	LCD_PutChar(lcd, ' ');
	LCD_Cursor_SetPos(lcd, previous_x, previous_y);

	return LCD_OK;
}

LCD_ERROR LCD_LineBreak(LCD_TypeDef *lcd)
{
	if(lcd->cursor_x < (lcd->size.max_lines - 1))
		LCD_Cursor_SetPos(lcd, (lcd->cursor_x + 1), 0);
	else
		return LCD_ERROR_;

	return LCD_OK;
}

LCD_ERROR LCD_Tab(LCD_TypeDef *lcd)
{
	uint8_t pos_y = ((uint8_t)(lcd->cursor_y / 3) + 1) * 3;

	LCD_Cursor_SetPos(lcd, lcd->cursor_x, pos_y);
	//LCD_Send_String(lcd, "   ");

	return LCD_OK;
}

LCD_ERROR LCD_CarrigeReturn(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	LCD_Cursor_SetPos(lcd, lcd->cursor_x, 0);
}

LCD_ERROR LCD_Clear_Char(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	return LCD_PutChar(lcd, ' ');
}

LCD_ERROR LCD_Clear_Line(LCD_TypeDef *lcd, uint8_t line)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	if(line >= lcd->size.max_lines)
		return LCD_ERROR_;

	LCD_Cursor_SetPos(lcd, line, 0);
	for(uint8_t i=0; i<lcd->size.max_columns; i++)
	{
		LCD_Cursor_SetPos(lcd, line, i);
		LCD_Clear_Char(lcd);
	}

	LCD_Cursor_SetPos(lcd, line, 0);
	return LCD_OK;
}

LCD_ERROR LCD_Clear_Display(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	for(uint8_t i=0; i<lcd->size.max_lines; i++)
	{
		LCD_Clear_Line(lcd, i);
	}

	LCD_Cursor_SetPos(lcd, 0, 0);

	return LCD_OK;
}


LCD_ERROR LCD_Area_CreateNew(LCD_TypeDef *lcd, LCD_Area *area)
{
	if(area == NULL)
		return LCD_ERROR_;

	area->lcd = lcd;

	uint8_t line_size  = (area->y2 - area->y1) + 1;
	uint8_t line_count = (area->x2 - area->x1) + 1;

	area->lines = line_count;
	area->line_length = line_size;
	area->lentgth = line_count * line_size;

	return LCD_OK;
}

LCD_ERROR LCD_Clear_Area(LCD_Area *area)
{
	if(area == NULL)
		return LCD_ERROR_;
	if(area->lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	for(uint8_t i=area->x1; i <= area->x2; i++)
	{
		for(uint8_t j=area->y1; j <= area->y2; j++)
		{
			LCD_Cursor_SetPos(area->lcd, i, j);
			LCD_Clear_Char(area->lcd);
		}
	}

	return LCD_OK;
}

LCD_ERROR LCD_Area_Update(LCD_Area *area, uint8_t string[])
{
	if(area == NULL)
		return LCD_ERROR_;
	if(area->lcd == NULL)
			return LCD_ERROR_HADLE_NOT_DEFINED;

	LCD_Clear_Area(area);

	uint8_t i=0;
	uint8_t x = area->x1;
	uint8_t y = area->y1;

	while(string[i] != 0)
	{
		if(y > area->y2)
		{
			if(x < area->x2)
			{
				x++;
				y = area->y1;
			}
			else
				break;

		}


		LCD_Cursor_SetPos(area->lcd, x, y);
		LCD_PutChar(area->lcd, string[i]);

		if(string[i] != '\n' && string[i] != '\t' && string[i] != '\b')
			y++;
		i++;
	}

	return LCD_OK;
}



















