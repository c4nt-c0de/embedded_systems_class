#include "component_helpers.h"

void create_pattern(digit_map_t *map, int left_disp_num, int right_disp_num, uint16_t patterns[2]) {
    // split each number into its two digits
    int left_display_digit1  = (left_disp_num / 10) % 10;
    int left_display_digit2  = left_disp_num % 10;
    int right_display_digit1 = (right_disp_num / 10) % 10;
    int right_display_digit2 = right_disp_num % 10;

    // phase 1 pattern (left tens + right tens)
    patterns[0] = ((uint16_t)get_digit_pattern(map, left_display_digit1) << 8)
                | get_digit_pattern(map, right_display_digit1);

    // phase 2 pattern (left ones + right ones)
    patterns[1] = ((uint16_t)get_digit_pattern(map, left_display_digit2) << 8)
                | get_digit_pattern(map, right_display_digit2);
}


// helper for above
uint8_t get_digit_pattern(digit_map_t *map, int digit) {
    switch (digit) {
        case 0: return map->zero;
        case 1: return map->one;
        case 2: return map->two;
        case 3: return map->three;
        case 4: return map->four;
        case 5: return map->five;
        case 6: return map->six;
        case 7: return map->seven;
        case 8: return map->eight;
        case 9: return map->nine;
        default: return 0xFF;
    }
}

// EXTERNAL BUTTONS
#define BUTTON_COOLDOWN 1000  // number of calls to skip after a press/release

int read_buttons(void) {
	static int prev_state = NO_BUTTON;  // last button returned
	static int cooldown = 0;            // debounce after release
	int current_state = NO_BUTTON;
	
	// debounce
	if (cooldown > 0) {
		
		//PIN_HIGH(GPIOC, LED_PIN);
		
		cooldown--;
		//prev_state = NO_BUTTON;
		return prev_state;
	}
	
	//PIN_LOW(GPIOC, LED_PIN);
	
	// read button hardware
	if (!PIN_READ(BTN_PORT, BTN_INC_PIN)) {
		current_state = INCREASE_BUTTON;
	} else if (!PIN_READ(BTN_PORT, BTN_DEC_PIN)) {
		current_state = DECREASE_BUTTON;
	} else if (!PIN_READ(BTN_PORT, BTN_SET_PIN)) {
		current_state = GO_BUTTON;
	} else {
		current_state = NO_BUTTON;
	}

	if (prev_state != current_state) cooldown = BUTTON_COOLDOWN;

	prev_state = current_state;
	return current_state;
}




// SHIFT REGISTER HELPERS
void sr_reset(shift_register_t *sr){ // pulse LOW on RST
	PIN_HIGH(sr->port, sr->rst);
	PIN_LOW(sr->port, sr->rst);
	PIN_HIGH(sr->port, sr->rst);
}

void sr_send_MSB(shift_register_t *sr, uint16_t pattern) {
	int i;
	for (i=0; i<16; i++) {
			// check current bit and set data pin
			if (pattern & (1 << (15 - i))) 
					PIN_HIGH(sr->port, sr->data);
			else 
					PIN_LOW(sr->port, sr->data);
			// pulse clock
			PIN_HIGH(sr->port, sr->clk);
			PIN_LOW(sr->port, sr->clk);
	}
}

void sr_send_LSB(shift_register_t *sr, uint16_t pattern) {
  int i;
	for (i = 0; i < 16; i++) {
			if (pattern & (1 << i))
					PIN_HIGH(sr->port, sr->data);
			else
					PIN_LOW(sr->port, sr->data);

			// pulse clock
			PIN_HIGH(sr->port, sr->clk);
			PIN_LOW(sr->port, sr->clk);
	}
}



int set_digit(anode_t *an, int anode_pin, int state) {
    uint16_t pin;
		// state == 0 -> OFF
		// state != 0 -> ON
    switch(anode_pin) {
        case 1: pin = an->A1; break;
        case 2: pin = an->A2; break;
        case 3: pin = an->A3; break;
        case 4: pin = an->A4; break;
        default: return 2; // invalid digit
    }

    if (state)
        PIN_HIGH(an->port, pin);
    else
        PIN_LOW(an->port, pin);
		
		if (check_anode_conflict(an)) return 1; // check if I didn't accidentally turn on both digits at once
		return 0;
}

int check_anode_conflict(anode_t *an) {
	uint16_t odr = an->port->ODR;

	// check A1 & A2
	if ((odr & (1 << an->A1)) && (odr & (1 << an->A2))) {
			// turn off all an
			PIN_LOW(an->port, an->A1);
			PIN_LOW(an->port, an->A2);
			PIN_LOW(an->port, an->A3);
			PIN_LOW(an->port, an->A4);
			return 1;
	}

	// check A3 & A4
	if ((odr & (1 << an->A3)) && (odr & (1 << an->A4))) {
			// turn off all an
			PIN_LOW(an->port, an->A1);
			PIN_LOW(an->port, an->A2);
			PIN_LOW(an->port, an->A3);
			PIN_LOW(an->port, an->A4);
			return 1;
	}

	return 0; // no conflict
}

void RCC_config(void){
	RCC->CR|=0x10000; // HSE on
	while(!(RCC->CR&0x20000)){}
	  // flash access setup
  	FLASH->ACR &= 0x00000038;   // mask register
  	FLASH->ACR |= 0x00000002;   // flash 2 wait state

 	FLASH->ACR &= 0xFFFFFFEF;   // mask register
    FLASH->ACR |= 0x00000010;   // enable Prefetch Buffer

	RCC->CFGR&=0xFFC3FFFF; // mask PLLMUL
	RCC->CFGR|=0x1<<18; // set PLLMUL 3x
	RCC->CFGR|=0x0<<17; // set PREDIV1 1x
	RCC->CFGR|=0x10000; // PLL bude clocovan z PREDIV1
	RCC->CFGR&=0xFFFFFF0F; // HPRE=1x
	RCC->CFGR&=0xFFFFF8FF; // PPRE2=1x
	RCC->CFGR&=0xFFFFC7FF; // PPRE2=1x

	RCC->CR|=0x01000000; // PLL on
	while(!(RCC->CR&0x02000000)){} // PLL stable??

	RCC->CFGR&=0xFFFFFFFC;
	RCC->CFGR|=0x2; // set PLL as clk source for SYSCLK

  	while(!(RCC->CFGR & 0x00000008)) // je SYSCLK nastaveno?
  	{
  	}

	RCC->APB2ENR|=0x14; // pocoleni PA a PC
		
}

void GPIO_init_pins(void) {
	int i;
	uint8_t sr_pins[] = { SR_CLK_PIN, SR_RST_PIN, SR_DATA_PIN };

	// LED (PC8) push-pull output
	GPIOC->CRH &= ~(0xF << ((LED_PIN - 8) * 4));
	GPIOC->CRH |=  (0x3 << ((LED_PIN - 8) * 4));

	// user button (PA0) floating input
	GPIOA->CRL &= ~(0xF << (USER_BUTTON_PIN * 4));
	GPIOA->CRL |=  (0x4 << (USER_BUTTON_PIN * 4));

	// external buttons (PC0–PC2) input with pull-ups
	for (i = BTN_INC_PIN; i <= BTN_SET_PIN; i++) {
		GPIOC->CRL &= ~(0xF << (i * 4));
		GPIOC->CRL |=  (0x8 << (i * 4));
		GPIOC->ODR |=  (1 << i);
	}

	// 7-segment an (PA1–PA4)
	for (i = ANODE_1; i <= ANODE_4; i++) {
		PIN_HIGH(GPIOA, i); // optional default state
		GPIOA->CRL &= ~(0xF << (i * 4));
		GPIOA->CRL |=  (0x3 << (i * 4));
	}

	// shift register pins (PA5–PA7)
	for (i = 0; i < 3; i++) {
		PIN_LOW(GPIOA, sr_pins[i]);
		GPIOA->CRL &= ~(0xF << (sr_pins[i] * 4));
		GPIOA->CRL |=  (0x3 << (sr_pins[i] * 4));
	}
}
