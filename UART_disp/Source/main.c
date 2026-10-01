#include  "stm32f10x.h"
#include "pins.h"
#include "utils.h"
#include "component_helpers.h"

/*definitions*/
#define SECOND_CYCLES 100 // pick so runtime of this many cycles of main takes about 1s
#define DEFAULT_TIMER_SETTING 10 // seconds
#define DOUBLE_PRESS_SETTING	10 // seconds

#define FLASH_INT_ADDR   ((uint32_t*)0x0801F800) // last 1KB page

/*global variables*/
char str_num_buf[3] = {0, 0, '\0'}; // for storing string form
int set_time, ready_time, run_time;

/*function prototypes*/
void write_set_time(int time);
void write_ready_time(int time);
void write_run_time(int time);
void timer_range_clamp(void);
void write_status_uart(int set_time, int ready_time, int run_time);
void handle_uart_input(int *set_time, int *ready_time, int *timer_running_counter);
	
/*methods*/

void SystemInit(void){
 // Called from startup file
 // Empty to allow initialization in main
 // Otherwise initialization happens in system_stm32f10x.h
}

/*Main function*/
int main(void){
	int timer_running_counter = 0;
	set_time = DEFAULT_TIMER_SETTING; 
	ready_time = flash_load_int(FLASH_INT_ADDR, DEFAULT_TIMER_SETTING, 99);
	run_time = ready_time;
	button_state_t button_states;
	
	RCC_config(); //initialize clocks
	init_pins();
	LCD.init();
	sr_reset();
	sr_send_MSB(0xAA);
	LCD.init();
	LCD.write_string("SET");	
	LCD.write_string("     ");
	LCD.write_string("READY");
	LCD.write_string("    ");
	
	LCD.place_cursor(1, 0);
	LCD.write_string("RUN");
	
	// terminal purge (sometimes garbage from prev session remains)
	uart_send_str("\x1B[2J"); // clear terminal
	uart_send_str("\x1B[H");  // move cursor home
	
	write_status_uart(set_time, ready_time, run_time);
	
	while(1){
		button_states = read_buttons();
		if (button_states.ext1 && button_states.ext2) {
			set_time = DOUBLE_PRESS_SETTING;
			ready_time = DOUBLE_PRESS_SETTING;
		}
		else if (button_states.ext1) {
			set_time++;
		}
		else if (button_states.ext2) {
			set_time--;
		}
		else if (button_states.ext3 || button_states.enc_btn) {
			ready_time = set_time;
		}
		else if (button_states.onboard_btn) { // reversed logic
			timer_running_counter = ready_time * SECOND_CYCLES;
		}
		else{
			//GPIOC->BSRR|=0x1000000; //NO, turn off LED on PC8
		}
		
		if (timer_running_counter) {  // && !timer_running_counter // disables letting the timer run while its also running
			GPIOC->BSRR = (1 << 8);
			timer_running_counter--;
			run_time = timer_running_counter / SECOND_CYCLES;
		}
		else {
			GPIOC->BSRR = (1 << (8 + 16));
			run_time = ready_time;
		}
		
    if (uart_char_available()) {
        char c = uart_read_char();
        if (c == '+') {        // increase time
            set_time++;
        } else if (c == '-') { // decrease time
            set_time--;
        } else if (c == ' ') { // confirm
            ready_time = set_time;
        }
    }
		
		set_time += encoder_read();		
	
		write_set_time(set_time);
		write_ready_time(ready_time);
		write_run_time(run_time);
		
		write_status_uart(set_time, ready_time, run_time);
		
		handle_uart_input(&set_time, &ready_time, &timer_running_counter);
		
		flash_save_int(FLASH_INT_ADDR, ready_time);
		
		timer_range_clamp();
	}
}


void write_set_time(int time) {
    LCD.place_cursor(0, 4);
    LCD.write_string(int_to_str(time, str_num_buf));
}

void write_ready_time(int time) {
    LCD.place_cursor(0, 14);
    LCD.write_string(int_to_str(time, str_num_buf));
}

void write_run_time(int time) {
    LCD.place_cursor(1, 4);
    LCD.write_string(int_to_str(time, str_num_buf));
}

void timer_range_clamp(void) { // should be enough to clamp set_time
	set_time = set_time > 99 ? 99 : set_time;
	set_time = set_time < 0  ? 0  : set_time;
	
}

void add_2digit(int v, char **pp) {
    char *p = *pp;

    if (v < 0) v = 0;
    if (v > 99) v = 99;

    if (v < 10) {
        *p++ = ' '; // space instead of leading zero
        *p++ = '0' + v;
    } else {
        *p++ = '0' + (v / 10);
        *p++ = '0' + (v % 10);
    }

    *pp = p;
}

void write_status_uart(int set_time, int ready_time, int run_time) {
    static int prev_set = -1;
    static int prev_ready = -1;
    static int prev_run = -1;

    if (set_time == prev_set &&
        ready_time == prev_ready &&
        run_time == prev_run)
        return;

    prev_set = set_time;
    prev_ready = ready_time;
    prev_run = run_time;

    char buf[80]; // slightly bigger buffer
    char *p = buf;

    // hide cursor
    *p++ = 0x1B; *p++ = '['; *p++ = '?'; *p++ = '2'; *p++ = '5'; *p++ = 'l';

    // move cursor **up 2 lines**
    *p++ = 0x1B; *p++ = '['; *p++ = '2'; *p++ = 'A';

    // clear first line
    *p++ = '\r';
    *p++ = 0x1B; *p++ = '['; *p++ = 'K';

    // "SET xx  READY xx"
    *p++ = 'S'; *p++ = 'E'; *p++ = 'T'; *p++ = ' ';
    add_2digit(set_time, &p);
    *p++ = ' '; *p++ = 'R'; *p++ = 'E'; *p++ = 'A';
    *p++ = 'D'; *p++ = 'Y'; *p++ = ' ';
    add_2digit(ready_time, &p);

    // newline for second line
    *p++ = '\n';

    // clear second line
    *p++ = '\r';
    *p++ = 0x1B; *p++ = '['; *p++ = 'K';

    // "RUN xx"
    *p++ = 'R'; *p++ = 'U'; *p++ = 'N'; *p++ = ' ';
    add_2digit(run_time, &p);

    *p = '\0';
    uart_send_str(buf);
}

// TODO: Add macros to choose which key does what at the top
#define KEYBOARD_COOLDOWN 30
void handle_uart_input(int *set_time, int *ready_time, int *timer_running_counter) {
	static uint16_t cooldown = 0; // counts down each main loop

	if (cooldown > 0)
			cooldown--;

	if (uart_char_available()) {
		char c = uart_read_char();

		// 's' starts the timer, always allowed
		if (c == 's' || c == 'S') {
			*timer_running_counter = (*ready_time) * SECOND_CYCLES;
			cooldown = KEYBOARD_COOLDOWN;
		}
		// other keys only work if cooldown expired
		else if (cooldown == 0) {
			if (c == '+') {
					(*set_time)++;
			} 
			else if (c == '-') {
					(*set_time)--;
			} 
			else if (c == ' ') {
					*ready_time = *set_time;
			}
			cooldown = KEYBOARD_COOLDOWN;
		}
	}
}


