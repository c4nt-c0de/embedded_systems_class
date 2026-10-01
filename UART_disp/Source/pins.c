#include "pins.h"
#include "utils.h"

// define pin structures
pin_t onboard_btn;
pin_t onboard_led;

pin_t encoder_btn;
pin_t encoder_dt;
pin_t encoder_clk;

pin_t ext_btn_1;
pin_t ext_btn_2;
pin_t ext_btn_3;

pin_t uart_tx;
pin_t uart_rx;

pin_t sr_data;
pin_t sr_clk;
pin_t sr_rst;

pin_t lcd_e;
pin_t lcd_rs;

pin_t parse_pin(const char* pin_str) {
	pin_t result = {0};
	switch (pin_str[1]) {
			case 'A': result.port = GPIOA; break;
			case 'B': result.port = GPIOB; break;
			case 'C': result.port = GPIOC; break;
	}

	int pin_number = 0;
	for (int i = 2; pin_str[i] != '\0'; i++) {
			if (pin_str[i] >= '0' && pin_str[i] <= '9') {
					pin_number = pin_number * 10 + (pin_str[i] - '0');
			} else {
					break;
			}
	}
	result.pin = pin_number;
	return result;
}

void init_pins(void) {

	// Parse all pins
	onboard_btn  = parse_pin(ONBOARD_BTN);
	onboard_led = parse_pin(ONBOARD_LED);
	encoder_btn  = parse_pin(ENCODER_BTN);
	encoder_dt   = parse_pin(ENCODER_DT);
	encoder_clk  = parse_pin(ENCODER_CLK);
	ext_btn_1    = parse_pin(EXT1_BTN);
	ext_btn_2    = parse_pin(EXT2_BTN);
	ext_btn_3    = parse_pin(EXT3_BTN);
	uart_tx      = parse_pin(UART_TX);
	uart_rx      = parse_pin(UART_RX);
	sr_data      = parse_pin(SR_DATA);
	sr_clk       = parse_pin(SR_CLK);
	sr_rst       = parse_pin(SR_RST);
	lcd_e        = parse_pin(LCD_E);
	lcd_rs       = parse_pin(LCD_RS);

	onboard_btn.port->CRL &= ~(0xF << ((onboard_btn.pin % 8) * 4));
	onboard_btn.port->CRL |=  (0x4 << ((onboard_btn.pin % 8) * 4));

	onboard_led.port->CRH &= ~(0xF << ((onboard_led.pin % 8) * 4));
	onboard_led.port->CRH |=  (0x3 << ((onboard_led.pin % 8) * 4));


	encoder_btn.port->CRL &= ~(0xF << ((encoder_btn.pin % 8) * 4));
	encoder_btn.port->CRL |=  (0x8 << ((encoder_btn.pin % 8) * 4));
	encoder_btn.port->BSRR = (1 << encoder_btn.pin);

	encoder_dt.port->CRL &= ~(0xF << ((encoder_dt.pin % 8) * 4));
	encoder_dt.port->CRL |=  (0x8 << ((encoder_dt.pin % 8) * 4));  
	encoder_dt.port->BSRR = (1 << encoder_dt.pin);                   

	encoder_clk.port->CRL &= ~(0xF << ((encoder_clk.pin % 8) * 4));
	encoder_clk.port->CRL |=  (0x8 << ((encoder_clk.pin % 8) * 4));   
	encoder_clk.port->BSRR = (1 << encoder_clk.pin);                 


	for (int i = 0; i < 3; i++) {
		pin_t* p = (i == 0) ? &ext_btn_1 : (i == 1) ? &ext_btn_2 : &ext_btn_3;
		p->port->CRH &= ~(0xF << ((p->pin % 8) * 4));
		p->port->CRH |=  (0x8 << ((p->pin % 8) * 4));
		p->port->ODR |=  (1 << p->pin);
	}

	// UART config
	uart_tx.port->CRH &= ~(0xF << ((uart_tx.pin % 8) * 4));
	uart_tx.port->CRH |=  (0xB << ((uart_tx.pin % 8) * 4));

	uart_rx.port->CRH &= ~(0xF << ((uart_rx.pin % 8) * 4));
	uart_rx.port->CRH |=  (0x4 << ((uart_rx.pin % 8) * 4));

	USART1->BRR = 0x09C4;   // set baud-rate (lecture 5, slide 42)

	// enable transmitter and receiver
	USART1->CR1 |= USART_CR1_TE;
	USART1->CR1 |= USART_CR1_RE;
	USART1->CR1 |= USART_CR1_UE;
	
	// shift register pins init
	sr_data.port->CRL &= ~(0xF << ((sr_data.pin % 8) * 4));
	sr_data.port->CRL |=  (0x3 << ((sr_data.pin % 8) * 4));

	sr_clk.port->CRH &= ~(0xF << ((sr_clk.pin % 8) * 4));
	sr_clk.port->CRH |=  (0x3 << ((sr_clk.pin % 8) * 4));

	sr_rst.port->CRH &= ~(0xF << ((sr_rst.pin % 8) * 4));
	sr_rst.port->CRH |=  (0x3 << ((sr_rst.pin % 8) * 4));

	// lcd port pins init
	lcd_e.port->CRL &= ~(0xF << ((lcd_e.pin % 8) * 4));
	lcd_e.port->CRL |=  (0x3 << ((lcd_e.pin % 8) * 4));

	lcd_rs.port->CRL &= ~(0xF << ((lcd_rs.pin % 8) * 4));
	lcd_rs.port->CRL |=  (0x3 << ((lcd_rs.pin % 8) * 4));
}
