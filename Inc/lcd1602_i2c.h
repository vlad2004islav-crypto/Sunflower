#ifndef LCD1602_I2C_H
#define LCD1602_I2C_H

#include "main.h"
#include <stdint.h>

// adress: 0x27
#define LCD_ADDRESS 0x27


#define LCD_RS 0x01  // Register Select (P0) - 0: command, 1: data
#define LCD_RW 0x02  // Read/Write (P1) - always 0 for write mode
#define LCD_EN 0x04  // Enable (P2) - strobe pin
#define LCD_BL  0x08 // Backlight (P3) - 1: backlight ON, 0: OFF

void lcd_Init(I2C_HandleTypeDef *i2c_handle);
void lcd_SendCommand(char cmd);
void lcd_SendData(char data);
void lcd_SendString(char *str);
void lcd_SetCursor(uint8_t row, uint8_t col);
void lcd_Clear(void);
void lcd_Backlight(uint8_t state);

#endif