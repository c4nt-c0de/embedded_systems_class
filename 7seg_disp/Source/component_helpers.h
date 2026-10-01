#ifndef COMPONENT_HELPERS_H
#define COMPONENT_HELPERS_H

#include "stm32f10x.h"
#include "pins.h"

extern volatile uint32_t millis_count;


typedef struct {
	GPIO_TypeDef *port;	// pin PORT (i.e. GPIOA)
	uint16_t data;			// DATA pin number
	uint16_t clk;				// CLK pin number
	uint16_t rst;				// RST pin number
	uint16_t num_bits;	// determined by number of shift registers chained together
} shift_register_t;


typedef struct {
	GPIO_TypeDef *port;
	uint16_t A1;
	uint16_t A2;
	uint16_t A3;
	uint16_t A4;
} anode_t;


typedef struct {
    uint8_t zero;
    uint8_t one;
    uint8_t two;
    uint8_t three;
    uint8_t four;
    uint8_t five;
    uint8_t six;
    uint8_t seven;
    uint8_t eight;
    uint8_t nine;
} digit_map_t;

void GPIO_init_pins(void);
void RCC_config(void);

void create_pattern(digit_map_t *map, int left_disp_num, int right_disp_num, uint16_t patterns[2]);
uint8_t get_digit_pattern(digit_map_t *map, int digit);
int read_buttons(void);
void sr_send_MSB(shift_register_t *sr, uint16_t pattern);
void sr_send_LSB(shift_register_t *sr, uint16_t pattern);
void sr_reset(shift_register_t *sr);
int set_digit(anode_t *an, int anode_pin, int state);
int check_anode_conflict(anode_t *an);





#endif
