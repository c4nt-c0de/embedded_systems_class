#include  "stm32f10x.h"

#include "component_helpers.h"

#define ON 	1
#define OFF 0
#define SECOND_CYCLES 10000
#define DEFAULT_TIMER_SETTING 10 // seconds
#define TIMER_SHORT_INCREMENT 1 // seconds
#define TIMER_LONG_INCREMENT 5 // seconds

#define SHORT_PRESS_T	1 // seconds
#define SHORT_PRESS_CYCLES 			SECOND_CYCLES * SHORT_PRESS_T

#define LONG_PRESS_T 2	// seconds
#define LONG_PRESS_CYCLES 			SECOND_CYCLES * LONG_PRESS_T

#define T 100
#define ANODE_SAFETY_DELAY	1000

// globals

shift_register_t shiftreg = { SR_PORT, SR_DATA_PIN, SR_CLK_PIN, SR_RST_PIN, SR_BIT_NUM };
anode_t anodes = { ANODE_PORT, ANODE_1, ANODE_2, ANODE_3, ANODE_4 };

digit_map_t digit_map = { 
		// works hardcoded for now, TODO in pins.h
    0xC0, // .zero 
    0xFC, // .one
    0x52, // .two
    0x54, // .three
    0x6C, // .four
    0x45, // .five
    0x41, // .six
    0xDC, // .seven
    0x40, // .eight
    0x44 // .nine
};

uint16_t patterns[2];

// function prototypes
void delay(vu32 nCount);
void clip_timer_range(int *settings_timer, int *countdown_timer);

void SystemInit(void) { // keep
}

int main(void) {
	uint32_t main_loop_counter = 0;
	uint32_t led_on_time = 0;
	uint8_t led_state = 0;
	
	int settings_time = DEFAULT_TIMER_SETTING;
	int countdown_time = DEFAULT_TIMER_SETTING;
	int countdown_cycle_counter = DEFAULT_TIMER_SETTING * SECOND_CYCLES;
	int current_button_state;
	int prev_button_state = NO_BUTTON;
	int button_press_counter = 0;
	int countdown_running = 0;
	create_pattern(&digit_map, settings_time, countdown_time, patterns);
	
	RCC_config();
	GPIO_init_pins();
	ANODE_PORT->BSRR = (0xF << (ANODE_1 + 16)); // Turn off all anodes
	sr_reset(&shiftreg);	
	

	while (1) {
    if ((uint32_t)(main_loop_counter - led_on_time) >= T) {
			if (led_state) {
				//PIN_LOW(GPIOC, LED_PIN); // OFF
				set_digit(&anodes, anodes.A2, OFF);
				set_digit(&anodes, anodes.A4, OFF);
				delay(ANODE_SAFETY_DELAY);
				sr_send_LSB(&shiftreg, patterns[0]);
				set_digit(&anodes, anodes.A1, ON);
				set_digit(&anodes, anodes.A3, ON);
				
			} else {
				//PIN_HIGH(GPIOC, LED_PIN); // ON
				set_digit(&anodes, anodes.A1, OFF);
				set_digit(&anodes, anodes.A3, OFF);
				delay(ANODE_SAFETY_DELAY);
				sr_send_LSB(&shiftreg, patterns[1]);
				set_digit(&anodes, anodes.A2, ON);
				set_digit(&anodes, anodes.A4, ON);
			}
			led_state = !led_state;
			led_on_time = main_loop_counter;
    }
		
		current_button_state = read_buttons();
		if (current_button_state != prev_button_state){ // some button was pressed
			if (current_button_state == NO_BUTTON) { // the pressed button was released
				if (button_press_counter < SHORT_PRESS_CYCLES) {				
					switch (prev_button_state) {
						case INCREASE_BUTTON:
							settings_time+=TIMER_SHORT_INCREMENT;
							break;
						case DECREASE_BUTTON:
							settings_time-=TIMER_SHORT_INCREMENT;
							break;
						case GO_BUTTON:
							countdown_time = settings_time + 1;
							countdown_running = 1;
							countdown_cycle_counter = countdown_time * SECOND_CYCLES;
							break;
						default:
							break;
					}
					button_press_counter = 0;
				}
				else {
					switch (prev_button_state) {
						case INCREASE_BUTTON:
							settings_time+=TIMER_LONG_INCREMENT;
							break;
						case DECREASE_BUTTON:
							settings_time-=TIMER_LONG_INCREMENT;
							break;
						case GO_BUTTON:
							countdown_time = settings_time + 1;
							countdown_running = 1;
							countdown_cycle_counter = countdown_time * SECOND_CYCLES;
						
							break;
						default:
							break;
						
					}
					button_press_counter = 0;
				}
			}
		} else { //
			if (current_button_state != NO_BUTTON) { // some button is being held down
				button_press_counter++;
			}
		}
		prev_button_state = current_button_state;
		clip_timer_range(&settings_time, &countdown_time);
		if (countdown_running) {
			PIN_HIGH(GPIOC, LED_PIN); // ON
			countdown_cycle_counter--;
			countdown_time = countdown_cycle_counter / SECOND_CYCLES;
		} 
		else {
			countdown_time = settings_time;
		}
		
		if (countdown_cycle_counter == 0) {
			countdown_time = settings_time;
			countdown_running = 0;
			PIN_LOW(GPIOC, LED_PIN); // OFF
		}
		create_pattern(&digit_map, settings_time, countdown_time, patterns);
		
		main_loop_counter++;
		
	}
}

void delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}

void clip_timer_range(int *settings_timer, int *countdown_timer) {
	if (*settings_timer < 0) *settings_timer = 0;
	if (*settings_timer >= 99) *settings_timer = 99;
	
	if (*countdown_timer < 0) *countdown_timer = 0;
	if (*countdown_timer >= 99) *countdown_timer = 99;
}

