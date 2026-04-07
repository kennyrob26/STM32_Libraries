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
	if(lcd->cursor.x < (lcd->size.max_lines - 1))
		lcd->cursor.x++;
	else
		lcd->cursor.x = (lcd->size.max_lines - 1);
}

static inline void cursor_x_decrement(LCD_TypeDef *lcd)
{
	if(lcd->cursor.x > 0 && lcd->cursor.x < 6) //6 of buffer overflow (-1 = 255, -2 = 254...)
		lcd->cursor.x--;
	else
		lcd->cursor.x = 0;
}

static inline void cursor_y_increment(LCD_TypeDef *lcd)
{
	if(lcd->cursor.y < (lcd->size.max_columns -1))
		lcd->cursor.y++;
	else
		lcd->cursor.y = (lcd->size.max_columns - 1);
}

static inline void cursor_y_decrement(LCD_TypeDef *lcd)
{
	if(lcd->cursor.y > 0 && lcd->cursor.y < 30) //30 of buffer overflow (-1 = 255, -2 = 254...)
		lcd->cursor.y--;
	else
		lcd->cursor.y = 0;
}

static inline LCD_ERROR LCD_CheckGPIOConfig(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	switch (lcd->interface)
	{
		case LCD_INTERFACE_4BIT:
			if(
				   (lcd->pin.E.port   == NULL) ||
				   (lcd->pin.RS.port  == NULL) ||
				   (lcd->pin.db7.port == NULL) ||
				   (lcd->pin.db6.port == NULL) ||
				   (lcd->pin.db5.port == NULL) ||
				   (lcd->pin.db4.port == NULL)
			) return LCD_ERROR_GPIO_NOT_DEFINED;
			break;

		case LCD_INTERFACE_8BIT:
			if(
				   (lcd->pin.E.port   == NULL) ||
				   (lcd->pin.RS.port  == NULL) ||
				   (lcd->pin.db7.port == NULL) ||
				   (lcd->pin.db6.port == NULL) ||
				   (lcd->pin.db5.port == NULL) ||
				   (lcd->pin.db4.port == NULL) ||
				   (lcd->pin.db3.port == NULL) ||
				   (lcd->pin.db2.port == NULL) ||
				   (lcd->pin.db1.port == NULL) ||
				   (lcd->pin.db0.port == NULL)

			) return LCD_ERROR_GPIO_NOT_DEFINED;
			break;

		case LCD_INTERFACE_I2C:
			if(lcd->i2c == NULL)
				return LCD_ERROR_I2C_HANDLE_NOT_DEFINED;

			if(HAL_I2C_IsDeviceReady(lcd->i2c, I2C_WRITE, 10, 100) != HAL_OK)
				return LCD_ERROR_I2C_HANDLE_NOT_DEFINED;

			break;
		default:
			return LCD_OK;
			break;
	}
}

/**
 * @brief Delay in micro seconds
 *
 * This function is used for microsecond delays and requires a TIM of 1 MHz.
 *
 * @param us is a time delay in us
 */
static inline void LCD_Delay_us(uint16_t us)
{
	if(lcd_tim != NULL)
	{
		__HAL_TIM_SET_COUNTER(lcd_tim, 0); // Reset TIM counter

		while(__HAL_TIM_GET_COUNTER(lcd_tim) < us)
		{/*Wait time*/};
	}
}

/**
 * @brief Check if char Escape Sequence
 *
 * This function check is char data is a escape sequence (\n, \b, \r, \t)
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param data this param is the target char
 *
 * @return 1 if is char is escape sequence
 * @return 0 if is a default char
 */
static inline uint8_t LCD_Check_EscapeSequence(LCD_TypeDef *lcd, uint8_t data)
{
	if(lcd != NULL)
	{
		switch (data)
		{
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
}

/**
 * @brief Set a GPIO param in GPIO of struct
 *
 * @note This functions is the basis for all others functions gpio setter of struct lcd
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param **lcd_gpio_port is a pointer for pointer lcd->pin.gpio.port, save a port in lcd struct
 * @param *lcd_gpio_pin is a pointer for lcd->pin.gpio.pin, save a pin in lcd struct
 * @param *gpio_port it's a gpio port we want to set in *lcd_gpio_port of lcd struct
 * @param  gpio_pin it's a gpio pin we want to set in *lcd_gpio_pin of lcd_struct
 *
 * @retval LCD_ERROR
 *
 */

static inline LCD_ERROR LCD_GPIO_Set(LCD_TypeDef *lcd,GPIO_TypeDef **lcd_gpio_port, uint16_t *lcd_gpio_pin, GPIO_TypeDef *gpio_port, uint16_t gpio_pin)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(gpio_port == NULL || lcd_gpio_pin == NULL || gpio_port == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	*lcd_gpio_port = gpio_port;
	*lcd_gpio_pin  = gpio_pin;
	return LCD_OK;
}

/**
 * @brief Set a DB0 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db0_port it's the DB0 GPIO port
 * @param db0_pin it's the DB0 GPIO pin
 *
 *  @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB0(LCD_TypeDef *lcd, GPIO_TypeDef *db0_port, uint16_t db0_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db0.port, &lcd->pin.db0.pin, db0_port, db0_pin);
}

/**
 * @brief Set a DB1 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db1_port it's the DB1 GPIO port
 * @param db1_pin it's the DB1 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB1(LCD_TypeDef *lcd, GPIO_TypeDef *db1_port, uint16_t db1_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db1.port, &lcd->pin.db1.pin, db1_port, db1_pin);
}

/**
 * @brief Set a DB2 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db2_port it's the DB2 GPIO port
 * @param db2_pin it's the DB2 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB2(LCD_TypeDef *lcd, GPIO_TypeDef *db2_port, uint16_t db2_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db2.port, &lcd->pin.db2.pin, db2_port, db2_pin);
}

/**
 * @brief Set a DB3 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db3_port it's the DB3 GPIO port
 * @param db3_pin it's the DB3 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB3(LCD_TypeDef *lcd, GPIO_TypeDef *db3_port, uint16_t db3_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db3.port, &lcd->pin.db3.pin, db3_port, db3_pin);
}

/**
 * @brief Set a DB4 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db4_port it's the DB4 GPIO port
 * @param db4_pin it's the DB4 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB4(LCD_TypeDef *lcd, GPIO_TypeDef *db4_port, uint16_t db4_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db4.port, &lcd->pin.db4.pin, db4_port, db4_pin);
}

/**
 * @brief Set a DB5 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db5_port it's the DB5 GPIO port
 * @param db5_pin it's the DB5 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB5(LCD_TypeDef *lcd, GPIO_TypeDef *db5_port, uint16_t db5_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db5.port, &lcd->pin.db5.pin, db5_port, db5_pin);
}

/**
 * @brief Set a DB6 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db6_port it's the DB6 GPIO port
 * @param db6_pin it's the DB6 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB6(LCD_TypeDef *lcd, GPIO_TypeDef *db6_port, uint16_t db6_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db6.port, &lcd->pin.db6.pin, db6_port, db6_pin);
}

/**
 * @brief Set a DB7 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *db7_port it's the DB7 GPIO port
 * @param db7_pin it's the DB7 GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetDB7(LCD_TypeDef *lcd, GPIO_TypeDef *db7_port, uint16_t db7_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.db7.port, &lcd->pin.db7.pin, db7_port, db7_pin);
}

/**
 * @brief Set a rs display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *rs_port it's the RS GPIO port
 * @param rs_pin it's the RS GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetRS(LCD_TypeDef *lcd, GPIO_TypeDef *rs_port, uint16_t rs_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.RS.port, &lcd->pin.RS.pin, rs_port, rs_pin);
}

/**
 * @brief Set a DB0 display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *enable_port it's the EN GPIO port
 * @param enable_pin it's the EN GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetEnable(LCD_TypeDef *lcd, GPIO_TypeDef *enable_port, uint16_t enable_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.E.port, &lcd->pin.E.pin, enable_port, enable_pin);
}

/**
 * @brief Set a RW display pin
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *rw_port it's the RW GPIO port
 * @param rw_pin it's the RW GPIO pin
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_GPIO_SetRW(LCD_TypeDef *lcd, GPIO_TypeDef *rw_port, uint16_t rw_pin)
{
	return LCD_GPIO_Set(lcd, &lcd->pin.RW.port, &lcd->pin.RW.pin, rw_port, rw_pin);
}

/**
 * @brief Set a I2C Handle for LCD
 *
 * Setter a pointer for i2C Handle in current lcd
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *i2c is a pointer for i2c Handle
 *
 * @retval LCD_ERROR
 *
 */
LCD_ERROR LCD_I2C_SetI2CHandle(LCD_TypeDef *lcd, I2C_HandleTypeDef *i2c)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(i2c == NULL)
		return LCD_ERROR_I2C_HANDLE_NOT_DEFINED;

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


static inline LCD_ERROR LCD_PinEnablePulse(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	switch (lcd->interface) {
		case LCD_INTERFACE_4BIT:
		case LCD_INTERFACE_8BIT:
			LCD_Delay_us(1);
			HAL_GPIO_WritePin(lcd->pin.E.port, lcd->pin.E.pin, 1);

			LCD_Delay_us(1);
			HAL_GPIO_WritePin(lcd->pin.E.port, lcd->pin.E.pin, 0);

			break;
		case LCD_INTERFACE_I2C:

			break;
		default:
			break;
	}

	return LCD_OK;
}


/*
 * @brief Send a Nibble for LCD
 *
 * This is an important function that involves sending a nibble to the display.
 * The display's behavior depends on the communication interface used.
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param nibble is a 1/2 byte (4 bits) for data
 * @param nibble_type define is is a high or low nibble
 * @param rs define a rs pin state
 *
 * @retval LCD_ERROR
 */
static LCD_ERROR LCD_Send_Nibble(LCD_TypeDef *lcd, uint8_t nibble, LCD_Nibble_Type nibble_type, LCD_RS rs)
{

	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	switch (lcd->interface)
	{
		case LCD_INTERFACE_4BIT:
			/**
			if(
				   (lcd->pin.E.port   == NULL) ||
				   (lcd->pin.RS.port  == NULL) ||
				   (lcd->pin.db7.port == NULL) ||
				   (lcd->pin.db6.port == NULL) ||
				   (lcd->pin.db5.port == NULL) ||
				   (lcd->pin.db4.port == NULL)
			)
			{
				return LCD_ERROR_GPIO_NOT_DEFINED;
			}
			*/
			uint8_t d7_value = (nibble >> 3) & 0x01;
			uint8_t d6_value = (nibble >> 2) & 0x01;
			uint8_t d5_value = (nibble >> 1) & 0x01;
			uint8_t d4_value = nibble & 0x01;

			if(lcd->pin.RW.port != NULL)
				HAL_GPIO_WritePin(lcd->pin.RW.port, lcd->pin.RW.pin, 0);
			HAL_GPIO_WritePin(lcd->pin.RS.port, lcd->pin.RS.pin, rs);
			LCD_Delay_us(1);

			HAL_GPIO_WritePin(lcd->pin.db7.port, lcd->pin.db7.pin, d7_value);
			HAL_GPIO_WritePin(lcd->pin.db6.port, lcd->pin.db6.pin, d6_value);
			HAL_GPIO_WritePin(lcd->pin.db5.port, lcd->pin.db5.pin, d5_value);
			HAL_GPIO_WritePin(lcd->pin.db4.port, lcd->pin.db4.pin, d4_value);

			LCD_PinEnablePulse(lcd);
		break;

		case LCD_INTERFACE_I2C:
			/*
			if(lcd->i2c == NULL)
				return LCD_ERROR_I2C_HANDLE_NOT_DEFINED;
			*/

			uint8_t cmd_nibble = (nibble << 4);
			cmd_nibble |= (rs << 0);
			cmd_nibble |= (1 << 3);

			cmd_nibble &= ~(1<<2);
			HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, &cmd_nibble, 1, 100);

			LCD_Delay_us(50);

			cmd_nibble |= (1 << 2);

			HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, &cmd_nibble, 1, 100);

			LCD_Delay_us(50);

			cmd_nibble &= ~(1 << 2);  //reset e bit, equals a xor

			HAL_I2C_Master_Transmit(lcd->i2c, I2C_WRITE, &cmd_nibble, 1, 100);

			LCD_Delay_us(50);
		break;
		default:
			return LCD_ERROR_;
			break;
	}

	return LCD_OK;

}

static LCD_ERROR LCD_Send_Byte(LCD_TypeDef *lcd, uint8_t byte, LCD_RS rs)
{
	switch(lcd->interface)
	{
		case LCD_INTERFACE_4BIT:
		case LCD_INTERFACE_I2C:
			uint8_t high_nibble = (byte >> 4) & 0x0F;
			uint8_t low_nibble  = byte & 0x0F;

			LCD_Send_Nibble(lcd, high_nibble, LCD_HIGH_NIBBLE, rs);
			LCD_Send_Nibble(lcd, low_nibble,  LCD_LOW_NIBBLE,  rs);
		break;
		case LCD_INTERFACE_8BIT:
			/*
			if(
				   (lcd->pin.E.port   == NULL) ||
				   (lcd->pin.RS.port  == NULL) ||
				   (lcd->pin.db7.port == NULL) ||
				   (lcd->pin.db6.port == NULL) ||
				   (lcd->pin.db5.port == NULL) ||
				   (lcd->pin.db4.port == NULL) ||
				   (lcd->pin.db3.port == NULL) ||
				   (lcd->pin.db2.port == NULL) ||
				   (lcd->pin.db1.port == NULL) ||
				   (lcd->pin.db0.port == NULL)
		    )
			{
				return LCD_ERROR_GPIO_NOT_DEFINED;
			}
		*/
			if(lcd->pin.RW.port != NULL)
				HAL_GPIO_WritePin(lcd->pin.RW.port, lcd->pin.RW.pin, 0);
			HAL_GPIO_WritePin(lcd->pin.RS.port, lcd->pin.RS.pin, rs);
			LCD_Delay_us(1);

			HAL_GPIO_WritePin(lcd->pin.db7.port, lcd->pin.db7.pin, ((byte >> 7) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db6.port, lcd->pin.db6.pin, ((byte >> 6) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db5.port, lcd->pin.db5.pin, ((byte >> 5) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db4.port, lcd->pin.db4.pin, ((byte >> 4) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db3.port, lcd->pin.db3.pin, ((byte >> 3) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db2.port, lcd->pin.db2.pin, ((byte >> 2) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db1.port, lcd->pin.db1.pin, ((byte >> 1) & 0x01));
			HAL_GPIO_WritePin(lcd->pin.db0.port, lcd->pin.db0.pin, (byte & 0x01));

			LCD_PinEnablePulse(lcd);
		break;
		default:
			break;
	}


    return LCD_OK;
}


/**
 * @brief Send a CMD LCD
 *
 * @note This function sending a command for display, commands is listen in datasheet
 *       for sending command defined RS in LOW
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param cmd is a us command
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Send_CMD(LCD_TypeDef *lcd, uint8_t cmd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	LCD_Send_Byte(lcd, cmd, LCD_RS_CONTROL);

	return LCD_OK;
}

/**
 * @brief Command Function Set
 *
 * @note This is a standard LCD command, important for display initialization.
 * 		 It defines whether the communication (MCU -> LCD) is 4-bit or 8-bit
 * @note Set 0x28 to use 4 bits (nibble)
 * @note set 0x38 to use 8 bits (byte)
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_CMD_FunctionSet(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t command;

	switch (lcd->interface)
	{
		case LCD_INTERFACE_4BIT:
		case LCD_INTERFACE_I2C:
			command = 0x28;
		break;
		case LCD_INTERFACE_8BIT:
			command = 0x38;
		break;
		default:
			return LCD_ERROR_INCORRECT_PARAM;
			break;
	}

	LCD_ERROR lcd_error = LCD_Send_CMD(lcd, command);

	LCD_Delay_us(100);

	return lcd_error;
}

/*
 * @brief Update a Command Display ON/OFF
 *
 * @note The command Display ON/OFF performs several functions. For this reason, we need an update function.
 *       All functions that use the ON/OFF command are required to perform this update to maintain synchronization
 *       between the display and the MCU.
 *
 * @note this function  uses struct params for update:
 * 	     - lcd->cmd_onOff.display_status -  ON/OFF display
 * 	     - lcd->cmd_onOff.cursor_status  -  ON/OFF cursor
 * 	     - lcd->cmd_onOff.cursor_status  -  ON/OFF blink cursor
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */

static inline LCD_ERROR LCD_UpdateOnOff(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t command = 0x08;

	command |= ((lcd->cmd_onOff.display_status & 0x01) << 2);
	command |= ((lcd->cmd_onOff.cursor_status & 0x01) << 1);
	command |= (lcd->cmd_onOff.cursor_blink_status & 0x01);

	LCD_Send_CMD(lcd, command);

	return LCD_OK;
}

/**
 * @brief Command Display ON/OFF
 *
 * @note This is a standard LCD command that sets the state of the display and cursor
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param display_status defines whether the display ON/OFF
 * @param cursor_status  defines whether the cursor is enabled or disabled.
 * @param blink_status   defines whether the blinking cursor is enabled or disabled.
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_CMD_DisplayOnOff(LCD_TypeDef *lcd, LCD_DisplayStatus display_status, LCD_CursorStatus cursor_status, LCD_CursorBlinkStatus blink_status)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.display_status = display_status;
	lcd->cmd_onOff.cursor_status = cursor_status;
	lcd->cmd_onOff.cursor_blink_status = blink_status;

	LCD_UpdateOnOff(lcd);

	return LCD_OK;
}

/**
 * @brief Define Display Size
 *
 * @note This function set the display size (columns x lines)
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param columns defines how many columns the display has
 * @param lines  defines how many lines the display has
 *
 * @retval LCD_ERROR
 */
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

/**
 * @brief Define Display format
 *
 * @note This function set the display format (16x2, 16x4...)
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param format is the LCD_DisplayFormat type
 *
 * @retval LCD_ERROR
 */
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

/**
 * @brief On Display
 *
 * @note define display ON (read ram data)
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Display_Enable(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.display_status = LCD_DISPLAY_ON;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

/**
 * @brief Off Display
 *
 * @note define display OFF (not read ram data)
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Display_Disable(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.display_status = LCD_DISPLAY_OFF;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

/**
 * @brief Set a cursor position
 *
 * @note Defines the x and y position of the cursor, with the initial position at 0
 *       and the final position at a max of -1.
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param  x is the x-coordinate of the cursor
 * @param  y is the y-coordinate of the cursor
 *
 * @retval LCD_ERROR
 */
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

	lcd->cursor.x = x;
	lcd->cursor.y = y;

	return LCD_OK;
}

/**
 * @brief Enable Cursor (visible)
 *
 * @note Makes the cursor visible at the current position
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_Enable(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.cursor_status = LCD_CURSOR_ENABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

/**
 * @brief Disable Cursor (not visible)
 *
 * @note Make the cursor invisible
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_Disable(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.cursor_status = LCD_CURSOR_DISABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

/**
 * @brief Cursor blink Enable
 *
 * @note The cursor is enabled and blinks
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_BlinkEnable(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.cursor_blink_status = LCD_BLINK_CURSOR_ENABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

/**
 * @brief Cursor blink Disable
 *
 * @note The blinks cursor is disable
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_BlinkDisable(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->cmd_onOff.cursor_blink_status = LCD_BLINK_CURSOR_DISABLE;
	LCD_UpdateOnOff(lcd);
	return LCD_OK;
}

/**
 * @brief Move the cursor to the left
 *
 * @note Move the cursor to the left, use this to navigate the display.
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_MovetoLeft(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	cursor_y_decrement(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor.x, lcd->cursor.y);
}

/**
 * @brief Move the cursor to the Right
 *
 * @note Move the cursor to the right, use this to navigate the display.
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_MovetoRight(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	cursor_y_increment(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor.x, lcd->cursor.y);
}

/**
 * @brief Move the cursor to the Up
 *
 * @note Move the cursor to the up, use this to navigate the display.
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_MovetoUp(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	cursor_x_decrement(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor.x, lcd->cursor.y);
}

/**
 * @brief Move the cursor to the down
 *
 * @note Move the cursor to the down, use this to navigate the display.
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Cursor_MovetoDown(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	cursor_x_increment(lcd);

	return LCD_Cursor_SetPos(lcd, lcd->cursor.x, lcd->cursor.y);
}


/**
 * @brief Command Clear Display, NOT RECOMMENDED
 *
 * @note this fucntion clear display, but prefer LCD_Clear_Display() function
 * @note Uses in initialization of display
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */

LCD_ERROR LCD_CMD_DisplayClear(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t cmd = 0x01;

	LCD_Send_CMD(lcd, cmd);

	HAL_Delay(3);  //recommended >= 1.53ms (datasheet)

	LCD_Cursor_SetPos(lcd, 0, 0); //Just to be sure

	return LCD_OK;
}

/**
 * @brief Set Auto line break
 *
 * @note If enabled (default), whenever we reach the end of the line, the line break will be automatic
 * @note If disabled, each line is viewed individually
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param auto_line_break is the LCD_AutoLineBreak type
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_SetAutoLineBreak(LCD_TypeDef *lcd, LCD_AutoLineBreak auto_line_break)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	lcd->auto_line_break = auto_line_break;
	return LCD_OK;
}

/**
 * @brief The display is initialized
 *
 * @note The display is initialized; the initialization defines the communication interface
 *       and follows the flowchart presented in the datasheet.
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param lcd_interface defines the communication interface (4-bit, 8-bit or I2C), is the LCD_INTERFACE type
 * @param *tim receives a tim base of 1MHz for micro seconds delay
 *
 * @retval LCD_ERROR
 */

LCD_ERROR LCD_Init(LCD_TypeDef *lcd, LCD_INTERFACE lcd_interface, TIM_HandleTypeDef *tim)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(tim == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	lcd->interface = lcd_interface;

	LCD_ERROR gpioStatus = LCD_CheckGPIOConfig(lcd);
	if(gpioStatus != LCD_OK)
		return gpioStatus;

	if(lcd->size.max_columns == 0)
		LCD_Display_SetFormat(lcd, LCD_FORMAT_16_02);

	LCD_SetAutoLineBreak(lcd, LCD_AUTO_LINE_BREAK_ENABLE);
	lcd_tim = tim;

	HAL_Delay(40);

	if(lcd->interface != LCD_INTERFACE_8BIT)
	{
		LCD_Send_Nibble(lcd, 0x03, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
		HAL_Delay(5);
		LCD_Send_Nibble(lcd, 0x03, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
		HAL_Delay(1);
		LCD_Send_Nibble(lcd, 0x03, LCD_HIGH_NIBBLE, LCD_RS_CONTROL);
		HAL_Delay(1);

		LCD_Send_Nibble(lcd, 0x02, LCD_HIGH_NIBBLE, LCD_RS_CONTROL); //force 4 bits
		HAL_Delay(1);
	}

	LCD_CMD_FunctionSet(lcd);
	LCD_CMD_FunctionSet(lcd);
	LCD_CMD_DisplayOnOff(lcd, LCD_DISPLAY_ON, LCD_CURSOR_DISABLE, LCD_BLINK_CURSOR_DISABLE);
	HAL_Delay(1);
	LCD_CMD_DisplayClear(lcd);
	LCD_Send_CMD(lcd, 0x06);  //Display Entrey default;

	lcd->status = LCD_INIT_INITIALIZED;

	return LCD_OK;
}

/**
 * @brief Check if we've reached the end of the line.
 *
 * @note If we reach the end of the line and automatic line breaks are enabled, we will break the line.
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @return LCD_ERROR
 */
static inline LCD_ERROR LCD_CheckEndLine(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	if(lcd->cursor.y >= lcd->size.max_columns)
	{
		if(lcd->cursor.x < (lcd->size.max_lines - 1))
		{
			if(lcd->auto_line_break == LCD_AUTO_LINE_BREAK_ENABLE)
				LCD_LineBreak(lcd);
		}
		else
		{
			lcd->cursor.y = lcd->size.max_columns;
			return LCD_ERROR_;
		}

	}

	return LCD_OK;
}

/**
 * @brief Put Char in display
 *
 * @note Sends a character for display in current cursor position, allows the use of some escape characters (\r, \n, \b and \t)
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param data is a unsigned char (0 - 256)
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_PutChar(LCD_TypeDef *lcd, uint8_t data)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	if(LCD_CheckEndLine(lcd) != LCD_OK)
		return LCD_ERROR_;

	if(LCD_Check_EscapeSequence(lcd, data))
		return LCD_OK;

	LCD_Send_Byte(lcd, data, LCD_RS_DATA);
	LCD_Delay_us(50);

	lcd->cursor.y++;
	LCD_CheckEndLine(lcd);


	return LCD_OK;
}


/**
 * @brief Print String in display
 *
 * @note Receive and print the string in current cursor position
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *string  is the text we want to print on the screen.
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Print(LCD_TypeDef *lcd, const char *string)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
	if(string == NULL)
		return LCD_ERROR_INCORRECT_PARAM;

	uint8_t i = 0;
	while(string[i] != '\0')
	{
		if(LCD_PutChar(lcd, string[i]) != LCD_OK)
			break;
		i++;
	}

	return LCD_OK;
}

/**
 * @brief Print the formated string
 *
 * @note Based on the Printf function, this function is similar, taking a string and a list of parameters.
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param *stringf is a formated string
 * @param ... is a va_list of arguments
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_PrintF(LCD_TypeDef *lcd, const char *stringf, ...)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	va_list args; //arg list (...)
	va_start(args, stringf);

	char send_string[32];

	vsnprintf(send_string, sizeof(send_string), stringf, args);
	va_end(args);

	return LCD_Print(lcd, send_string);
}

/**
 * @brief Backspace \b
 *
 * @note Go back to the previous character and clear
 * @note used as escape character \b
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Backspace(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t previous_x = 0;
	uint8_t previous_y = 0;

	if(lcd->cursor.y > 0)
	{
		previous_x = lcd->cursor.x;
		previous_y = lcd->cursor.y - 1;

	}
	else if(lcd->cursor.x > 0)
	{
		previous_x = (lcd->cursor.x - 1);
		previous_y = (lcd->size.max_columns - 1);
	}
	else
		return LCD_ERROR_;


	LCD_Cursor_SetPos(lcd, previous_x, previous_y);
	LCD_PutChar(lcd, ' ');
	LCD_Cursor_SetPos(lcd, previous_x, previous_y);

	return LCD_OK;
}

/**
 * @brief Line Break \n
 *
 * @note If there are more lines, a line break will be applied; otherwise, it is simply ignored.
 * @note used as escape character \n
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_LineBreak(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	if(lcd->cursor.x < (lcd->size.max_lines - 1))
		LCD_Cursor_SetPos(lcd, (lcd->cursor.x + 1), 0);
	else
		return LCD_ERROR_;

	return LCD_OK;
}

/**
 * @brief Tab  \t
 *
 * @note Divide the line into sections of 3 characters, and with each "tab" organize the data into
 * 	     the next section. This ensures alignment between lines (tables).
 * @note used as escape character \t
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Tab(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	uint8_t pos_y = ((uint8_t)(lcd->cursor.y / 3) + 1) * 3;

	LCD_Cursor_SetPos(lcd, lcd->cursor.x, pos_y);
	//LCD_Send_String(lcd, "   ");

	return LCD_OK;
}

/**
 * @brief Carrige return \r
 *
 * @note Return to the beginning of the current line. (x=x_current, y = 0)
 * @note used as escape character \r
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_CarrigeReturn(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	return LCD_Cursor_SetPos(lcd, lcd->cursor.x, 0);
}

/**
 * @brief Clear current char
 *
 * @note clears the char at the current cursor position
 *
 * @param *lcd is a pointer for Lcd Handle
 *
 * @retval LCD_ERROR
 */
LCD_ERROR LCD_Clear_Char(LCD_TypeDef *lcd)
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	return LCD_PutChar(lcd, ' ');
}

/**
 * @brief Clear current line
 *
 * @note clear the indicated line
 * @note After clearing, the cursor is positioned at the beginning of the line.
 *
 * @param *lcd is a pointer for Lcd Handle
 * @param line is the target line
 *
 * @retval LCD_ERROR
 */
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

/**
 * @brief Clear All display
 *
 * @note Clear all display and the cursor is positioned in line and column 0 (0,0)
 */
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
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;
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

LCD_ERROR LCD_CreateChar(LCD_TypeDef *lcd, uint8_t pos, uint8_t charactere[8])
{
	if(lcd == NULL)
		return LCD_ERROR_HADLE_NOT_DEFINED;

	if(pos > 7)
		return LCD_ERROR_INCORRECT_PARAM;

	uint8_t cgram_pos_0 = 0x40;
	uint8_t cgram_pos   = cgram_pos_0 | (pos << 3); //pos << 3 == pos * 8

	LCD_Send_CMD(lcd, cgram_pos);

	LCD_Delay_us(50);

	for(uint8_t line = 0; line < 8; line++)
	{
		LCD_Send_Byte(lcd, charactere[line], LCD_RS_DATA);
	}

	LCD_Delay_us(50);
	LCD_Cursor_SetPos(lcd, lcd->cursor.x, lcd->cursor.y);
	return LCD_OK;
}

















