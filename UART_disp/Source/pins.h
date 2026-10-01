#pragma once

#include  "stm32f10x.h"

#define ONBOARD_BTN "PA0"
#define ONBOARD_LED "PC8"

#define ENCODER_BTN "PB7"
#define ENCODER_DT "PB6"
#define ENCODER_CLK "PB5"

#define EXT1_BTN "PC12"
#define EXT2_BTN "PC11"
#define EXT3_BTN "PC10"

#define UART_TX "PA9"
#define UART_RX "PA10"

#define SR_DATA "PC7"
#define SR_CLK "PB8"
#define SR_RST "PB9"

#define LCD_RS "PA1"
#define LCD_E "PA2"

// Shift register Q1-Q8 are wired to D0-D7 in order
// not gonna worry about that for now

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} pin_t;

extern pin_t onboard_btn;
extern pin_t onboard_led;

extern pin_t encoder_btn;	// confirm button
extern pin_t encoder_dt;
extern pin_t encoder_clk;

extern pin_t ext_btn_1;	// increasee button
extern pin_t ext_btn_2;	// decrease button
extern pin_t ext_btn_3;	// confirm button

extern pin_t uart_tx;
extern pin_t uart_rx;

extern pin_t sr_data;
extern pin_t sr_clk;
extern pin_t sr_rst;

extern pin_t lcd_e;
extern pin_t lcd_rs;

pin_t parse_pin(const char* pin_str);
void init_pins(void);

