#include "lcd1602_i2c.h"
#include <string.h>

static I2C_HandleTypeDef *lcd_i2c;
static uint8_t lcd_backlight = 1; // The backlight is always on

// Sending four bits to the display
static void lcd_Send4Bits(uint8_t data, uint8_t rs) {
    uint8_t data_t[4];
    
    // Forming the byte for transmission:
    // High 4 bits = data, RS = rs, EN = first 1, then 0
    // RW always 0 (write), BL = backlight state
    uint8_t byte = (data & 0xF0) | (rs ? LCD_RS : 0) | (lcd_backlight ? LCD_BL : 0);
    
    // Send with EN=1
    data_t[0] = byte | LCD_EN;
    // Send with EN=0
    data_t[1] = byte & ~LCD_EN;
    
    HAL_I2C_Master_Transmit(lcd_i2c, (LCD_ADDRESS << 1), data_t, 2, 100);
}

// Sending a command to the display
void lcd_SendCommand(char cmd) {
    uint8_t upper = cmd & 0xF0;      // high 4 bits
    uint8_t lower = (cmd << 4) & 0xF0; // // low 4 bits
    
    lcd_Send4Bits(upper, 0); // RS = 0 for command
    lcd_Send4Bits(lower, 0);
    HAL_Delay(2);
}

// Sending data (symbol) to the display
void lcd_SendData(char data) {
    uint8_t upper = data & 0xF0;
    uint8_t lower = (data << 4) & 0xF0;
    
    lcd_Send4Bits(upper, 1); /// RS = 1 for data
    lcd_Send4Bits(lower, 1);
    HAL_Delay(1);
}

// Sending a string
void lcd_SendString(char *str) {
    while(*str) {
        lcd_SendData(*str++);
    }
}

// Cursor setting (row: 0-1, col: 0-15)
void lcd_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address;
    
    if(row == 0) {
        address = 0x00 + col;      // DDRAM area start at 0x00 for row 0
    } else {
        address = 0x40 + col;      // DDRAM area start at 0x40 for row 1
    }
    
    lcd_SendCommand(0x80 | address); // Set DDRAM address command
}

// Clearing the display
void lcd_Clear(void) {
    lcd_SendCommand(0x01); // Clear entire display
    HAL_Delay(2);
}

// Backlight control (1 - on, 0 - off)
void lcd_Backlight(uint8_t state) {
    lcd_backlight = state;
    // Send single byte for backlight control
    uint8_t byte = (lcd_backlight ? LCD_BL : 0);
    HAL_I2C_Master_Transmit(lcd_i2c, (LCD_ADDRESS << 1), &byte, 1, 100);
}

// Initializing the display
void lcd_Init(I2C_HandleTypeDef *i2c_handle) {
    lcd_i2c = i2c_handle;
    
	// Wait after power-on (>40ms)
    HAL_Delay(50);
    
	// Initialization sequence to 4-bit mode
    lcd_Send4Bits(0x30, 0);
    HAL_Delay(5);
    lcd_Send4Bits(0x30, 0);
    HAL_Delay(1);
    lcd_Send4Bits(0x30, 0);
    HAL_Delay(10);
    
	// Switch to 4-bit mode
    lcd_Send4Bits(0x20, 0);
    HAL_Delay(10);
    
	// Function set: 4-bit, 2 lines, 5x8 font
    lcd_SendCommand(0x28); 
    lcd_SendCommand(0x08); // Display OFF
    lcd_SendCommand(0x01); // // Clear display
    HAL_Delay(2);
    lcd_SendCommand(0x06); // Entry mode: increment cursor, no shift
    lcd_SendCommand(0x0C); // Display ON, cursor OFF, blink OFF
    
		// Turn backlight on
    lcd_Backlight(1);
}