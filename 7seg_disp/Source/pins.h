#ifndef PINS_H
#define PINS_H

#include "stm32f10x.h"

// Onboard USER button
#define USER_BUTTON_PIN     0  // PA0

// Onboard LED
#define LED_PIN             8  // PC8

// External buttons (connect to GND)
#define BTN_INC_PIN         0  // PC0
#define BTN_DEC_PIN         1  // PC1
#define BTN_SET_PIN         2  // PC2
#define BTN_PORT 						GPIOC


#define NO_BUTTON				0
#define INCREASE_BUTTON 1 
#define DECREASE_BUTTON 2 
#define GO_BUTTON 			3

// 7-segment anode pins
#define ANODE_1             1  // PA1
#define ANODE_2             2  // PA2
#define ANODE_3             3  // PA3
#define ANODE_4             4  // PA4
#define	ANODE_PORT					GPIOA

// Shift register pins
#define SR_DATA_PIN         7  // PA7
#define SR_RST_PIN          6  // PA6 		reset HIGH for normal operation
#define SR_CLK_PIN          5  // PA5
#define SR_PORT							GPIOA
#define SR_BIT_NUM					16


#define PIN_HIGH(port, pin)   ((port)->BSRR = (1U << (pin)))
#define PIN_LOW(port, pin)    ((port)->BSRR = (1U << ((pin) + 16)))
#define PIN_READ(port, pin)   (((port)->IDR & (1U << (pin))) ? 1 : 0)


/*
TODO:
defines for different segments
bit0 G		1111 1110 // segment lights up, when its particular bit is 0
bit1 DP		1111 1101
bit2 A		...
bit3 F
bit4 D
bit5 E
bit6 C
bit7 B
*/

/*
const uint8_t digits[10] = {	// hardcoded is alright for now, works with sr_send_MSB
    0x03, // 0000 0011 -> 0
    0x3F, // 0011 1111 -> 1
    0x4A, // 0100 1010 -> 2
    0x2A, // 0010 1010 -> 3
    0x36, // 0011 0110 -> 4
    0xA2, // 1010 0010 -> 5
    0x82, // 1000 0010 -> 6
    0x3B, // 0011 1011 -> 7
    0x02, // 0000 0010 -> 8
    0x22  // 0010 0010 -> 9
};

const uint8_t digits[10] = {	// hardcoded bit order, works with sr_send_LSB
    0xC0, // 1100 0000 -> 0
    0xFC, // 1111 1100 -> 1
    0x52, // 0101 0010 -> 2
    0x54, // 0101 0100 -> 3
    0x6C, // 0110 1100 -> 4
    0x45, // 0100 0101 -> 5
    0x41, // 0100 0001 -> 6
    0xDC, // 1101 1100 -> 7
    0x40, // 0100 0000 -> 8
    0x44  // 0100 0100 -> 9
};

*/


/*
		// SAVES
		sr_send_LSB(&shiftreg, pattern);
		delay(DELAY_CYCLES);
		set_digit(&anodes, anodes.A1, ON);
		set_digit(&anodes, anodes.A3, ON); 
		delay(DELAY_CYCLES);
		set_digit(&anodes, anodes.A1, OFF);
		set_digit(&anodes, anodes.A3, OFF);
*/

#endif

