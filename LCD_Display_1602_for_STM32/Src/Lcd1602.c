/*
 * Lcd1602.c
 *
 *  Created on: Mar 30, 2026
 *      Author: kenny
 */


#include "Lcd1602.h"

#define LINE1 0x00
#define LINE2 0x40

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


LCD_ERROR LCD_SetPin_DB0(LCD_TypeDef *lcd, GPIO_TypeDef *db0_port, uint16_t db0_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db0_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db0.port = db0_port;
	lcd->pin.db0.pin  = db0_pin;
	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB1(LCD_TypeDef *lcd, GPIO_TypeDef *db1_port, uint16_t db1_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db1_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db1.port = db1_port;
	lcd->pin.db1.pin  = db1_pin;

	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB2(LCD_TypeDef *lcd, GPIO_TypeDef *db2_port, uint16_t db2_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db2_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db2.port = db2_port;
	lcd->pin.db2.pin  = db2_pin;

	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB3(LCD_TypeDef *lcd, GPIO_TypeDef *db3_port, uint16_t db3_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db3_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db3.port = db3_port;
	lcd->pin.db3.pin  = db3_pin;
	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB4(LCD_TypeDef *lcd, GPIO_TypeDef *db4_port, uint16_t db4_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db4_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db4.port = db4_port;
	lcd->pin.db4.pin  = db4_pin;
	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB5(LCD_TypeDef *lcd, GPIO_TypeDef *db5_port, uint16_t db5_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db5_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db5.port = db5_port;
	lcd->pin.db5.pin  = db5_pin;

	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB6(LCD_TypeDef *lcd, GPIO_TypeDef *db6_port, uint16_t db6_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db6_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db6.port = db6_port;
	lcd->pin.db6.pin  = db6_pin;
	return LCD_OK;
}
LCD_ERROR LCD_SetPin_DB7(LCD_TypeDef *lcd, GPIO_TypeDef *db7_port, uint16_t db7_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(db7_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.db7.port = db7_port;
	lcd->pin.db7.pin  = db7_pin;
	return LCD_OK;
}
LCD_ERROR LCD_SetPin_RS(LCD_TypeDef *lcd, GPIO_TypeDef *rs_port, uint16_t rs_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(rs_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.RS.port = rs_port;
	lcd->pin.RS.pin  = rs_pin;

	return LCD_OK;
}
LCD_ERROR LCD_SetPin_Enable(LCD_TypeDef *lcd, GPIO_TypeDef *enable_port, uint16_t enable_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(enable_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.E.port = enable_port;
	lcd->pin.E.pin  = enable_pin;
	return LCD_OK;
}
LCD_ERROR LCD_SetPin_RW(LCD_TypeDef *lcd, GPIO_TypeDef *rw_port, uint16_t rw_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(rw_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->pin.RW.port = rw_port;
	lcd->pin.RW.pin  = rw_pin;
	return LCD_OK;
}

LCD_ERROR LCD_SetDisplaySize(LCD_TypeDef *lcd, uint8_t columns, uint8_t lines)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(columns > 20 || lines > 4)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->size.max_columns = columns;
	lcd->size.max_lines   = lines;

	return LCD_OK;
}

LCD_ERROR LCD_SetDisplayFormat(LCD_TypeDef *lcd, LCD_DisplayFormat format)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	switch (format)
	{
		case LCD_FORMAT_16_02:
			return LCD_SetDisplaySize(lcd, 16, 2);
		case LCD_FORMAT_16_04:
			return LCD_SetDisplaySize(lcd, 16, 4);
		case LCD_FORMAT_20_02:
			return LCD_SetDisplaySize(lcd, 20, 2);
		case LCD_FORMAT_20_04:
			return LCD_SetDisplaySize(lcd, 20, 4);
		default:
			return LCD_ERROR_INCORRECT_PARAM;
	}
}

static LCD_ERROR LCD_Send_Nibble(LCD_TypeDef *lcd, uint8_t nibble, LCD_Nibble_Type nibble_type, LCD_RS rs)
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

	return LCD_OK;

}

LCD_ERROR LCD_Send_CMD(LCD_TypeDef *lcd, uint8_t cmd)
{
	if(lcd->interface == LCD_INTERFACE_4BIT)
	{
		uint8_t high_nibble = (cmd >> 4) & 0x0F;
		uint8_t low_nibble  = cmd & 0x0F;
		LCD_Send_Nibble(lcd, high_nibble, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
		LCD_Send_Nibble(lcd, low_nibble, LCD_LOW_NIBBLE, LCD_RS_CONTROL);
	}

	return LCD_OK;
}

LCD_ERROR LCD_CMD_FunctionSet(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface)
{
	if(lcd_interface == LCD_INTERFACE_4BIT)
	{
		uint8_t command = 0x28;
		LCD_Send_CMD(lcd, command);
	}

	LCD_Delay_us(100);
	return LCD_OK;
}

LCD_ERROR LCD_Enable_Display(LCD_TypeDef *lcd)
{
	lcd->display_status = LCD_DISPLAY_ON;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Disable_Display(LCD_TypeDef *lcd)
{
	lcd->display_status = LCD_DISPLAY_OFF;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Enable_Cursor(LCD_TypeDef *lcd)
{
	lcd->cursor_status = LCD_CURSOR_ENABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Disable_Cursor(LCD_TypeDef *lcd)
{
	lcd->cursor_status = LCD_CURSOR_DISABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

LCD_ERROR LCD_Enable_BlinkCursor(LCD_TypeDef *lcd)
{
	lcd->cursor_blink_status = LCD_BLINK_CURSOR_ENABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}
LCD_ERROR LCD_Disable_BlinkCursor(LCD_TypeDef *lcd)
{
	lcd->cursor_blink_status = LCD_BLINK_CURSOR_DISABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}


LCD_ERROR LCD_UpdateOnOff(LCD_TypeDef *lcd)
{
	uint8_t command = 0x08;

	command |= ((lcd->display_status & 0x01) << 2);
	command |= ((lcd->cursor_status & 0x01) << 1);
	command |= (lcd->cursor_blink_status & 0x01);

	LCD_Send_CMD(lcd, command);
}

LCD_ERROR LCD_CMD_OnOff(LCD_TypeDef *lcd, uint8_t display_on_off, uint8_t cursor, uint8_t blink_cursor)
{
	lcd->display_status = display_on_off;
	lcd->cursor_status = cursor;
	lcd->cursor_blink_status = blink_cursor;

	LCD_UpdateOnOff(lcd);

	return LCD_OK;
}

LCD_ERROR LCD_CMD_DisplayClear(LCD_TypeDef *lcd)
{
	uint8_t cmd = 0x01;

	LCD_Send_CMD(lcd, cmd);

	HAL_Delay(3);  //recommended 1.53ms

	return LCD_OK;

}

LCD_ERROR LCD_CMD_SetCursor(LCD_TypeDef *lcd, uint8_t x, uint8_t y)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t cursor_adress;

	if(x >= lcd->size.max_lines || y >= lcd->size.max_columns)
		return LCD_ERROR_;

	if(x == 0)
		cursor_adress = LINE1 + y;
	else if(x == 1)
		cursor_adress = LINE2 + y;

	cursor_adress |= 0x80; //1xxxxxxx

	LCD_Send_CMD(lcd, cursor_adress);
	LCD_Delay_us(50);

	lcd->cursor_x = x;
	lcd->cursor_y = y;

	return LCD_OK;
}

LCD_ERROR LCD_MoveCursor_toLeft(LCD_TypeDef *lcd)
{
	cursor_y_decrement(lcd);

	return LCD_CMD_SetCursor(lcd, lcd->cursor_x, lcd->cursor_y);
}

LCD_ERROR LCD_MoveCursor_toRight(LCD_TypeDef *lcd)
{
	cursor_y_increment(lcd);

	return LCD_CMD_SetCursor(lcd, lcd->cursor_x, lcd->cursor_y);
}

LCD_ERROR LCD_MoveCursor_toUp(LCD_TypeDef *lcd)
{
	cursor_x_decrement(lcd);

	return LCD_CMD_SetCursor(lcd, lcd->cursor_x, lcd->cursor_y);
}

LCD_ERROR LCD_MoveCursor_toDown(LCD_TypeDef *lcd)
{
	cursor_x_increment(lcd);

	return LCD_CMD_SetCursor(lcd, lcd->cursor_x, lcd->cursor_y);
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
		LCD_SetDisplayFormat(lcd, LCD_FORMAT_16_02);

	LCD_SetAutoLineBreak(lcd, LCD_AUTO_LINE_BREAK_ENABLE);
	lcd_tim = tim;

	HAL_Delay(40);

	LCD_Send_Nibble(lcd, 0x03, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
	HAL_Delay(1);
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

LCD_ERROR LCD_Send_Data(LCD_TypeDef *lcd, uint8_t data)
{
	if(LCD_CheckEndLine(lcd) != LCD_OK)
		return LCD_ERROR_;

	if(lcd->interface == LCD_INTERFACE_4BIT)
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

LCD_ERROR LCD_Send_String(LCD_TypeDef *lcd, uint8_t string[])
{
	uint8_t i = 0;
	while(string[i] != '\0')
	{
		if(LCD_Send_Data(lcd, string[i]) != LCD_OK)
			break;
		i++;
	}

	return LCD_OK;
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


	LCD_CMD_SetCursor(lcd, previous_x, previous_y);
	LCD_Send_Data(lcd, ' ');
	LCD_CMD_SetCursor(lcd, previous_x, previous_y);

	return LCD_OK;
}

LCD_ERROR LCD_LineBreak(LCD_TypeDef *lcd)
{
	if(lcd->cursor_x < (lcd->size.max_lines - 1))
		LCD_CMD_SetCursor(lcd, (lcd->cursor_x + 1), 0);
	else
		return LCD_ERROR_;

	return LCD_OK;
}












