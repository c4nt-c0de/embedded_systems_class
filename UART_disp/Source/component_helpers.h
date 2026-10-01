#pragma once

#include  "stm32f10x.h"
#include "pins.h"
#include "utils.h"

// shift register helpers
void sr_reset(void);
void sr_send_MSB(uint8_t pattern);
void sr_send_LSB(uint8_t pattern);

typedef struct {
    uint8_t ext1;
    uint8_t ext2;
    uint8_t ext3;
    uint8_t enc_btn;
		uint8_t onboard_btn;
} button_state_t;


// LCD stuff
typedef struct {
	pin_t *rs;
	pin_t *e;

	void (*send_command)(uint8_t cmd);
	void (*send_data)(uint8_t data);
	void (*clear)(void);
	void (*write_char)(char c);
	void (*write_string)(char* str);
	void (*place_cursor)(uint8_t row, uint8_t col);
	void (*init)(void);
} lcd_t;

extern lcd_t LCD;

// helper functions
int encoder_read(void);
button_state_t read_buttons(void);
void lcd_send_command(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_clear(void);
void lcd_write_char(char c);
void lcd_write_string(char* str);
void lcd_place_cursor(uint8_t row, uint8_t col);
void lcd_init(void);

// UART helpers
int  uart_char_available(void);      // returns non-zero if a char is ready to read
char uart_read_char(void);           // read one char from UART
void uart_send_char(char c);         // send one char
void uart_send_str(const char *s);   // send null-terminated string
void uart_send_int(int val);         // send integer as string
