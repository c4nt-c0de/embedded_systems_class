#include "component_helpers.h"

// externs defined in pins.h
// copying them here just for clarity for now

//extern pin_t onboard_led; 

// UART STUFF ######################################################################################################################################################################################################
extern pin_t uart_tx;
extern pin_t uart_rx;

int uart_char_available(void) {
    return USART1->SR & USART_SR_RXNE; // returns non-zero if a char is ready
}

char uart_read_char(void) {
    return USART1->DR & 0xFF;
}

void uart_send_char(char c) {
    while (!(USART1->SR & USART_SR_TXE)); // wait until TX ready
    USART1->DR = c;
}

void uart_send_str(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_int(int val) {
    char buf[4]; // enough for -99..99 + '\0'
    uart_send_str(int_to_str(val, buf));
}


// EXTERNAL BUTTONS/ENCODER ######################################################################################################################################################################################################
extern pin_t onboard_btn; 
extern pin_t ext_btn_1;
extern pin_t ext_btn_2;
extern pin_t ext_btn_3;

extern pin_t encoder_btn;

#define BUTTON_COOLDOWN 50 // adjust to main loop speed
#define DOUBLE_PRESS_COOLDOWN 200

button_state_t read_buttons(void) {
	static uint16_t cooldown_ext1 = 0;
	static uint16_t cooldown_ext2 = 0;
	static uint16_t cooldown_ext3 = 0;
	static uint16_t cooldown_enc  = 0;
	static uint16_t cooldown_onboard = 0;
	
	static button_state_t prev_state = {0, 0, 0, 0, 0};
	button_state_t cur_state = {0, 0, 0, 0, 0};
	// button 1
	if (cooldown_ext1 > 0) {
		cooldown_ext1--;
		cur_state.ext1 = 1;
	} else if (!READ_PIN(ext_btn_1.port, ext_btn_1.pin)) {
			cur_state.ext1 = 1;
			cooldown_ext1 = BUTTON_COOLDOWN;
	}

	// button 2
	if (cooldown_ext2 > 0) {
			cooldown_ext2--;
			cur_state.ext2 = 1;
	} else if (!READ_PIN(ext_btn_2.port, ext_btn_2.pin)) {
			cur_state.ext2 = 1;
			cooldown_ext2 = BUTTON_COOLDOWN;
	}

	// button 3
	if (cooldown_ext3 > 0) {
			cooldown_ext3--;
			cur_state.ext3 = 1;
	} else if (!READ_PIN(ext_btn_3.port, ext_btn_3.pin)) {
			cur_state.ext3 = 1;
			cooldown_ext3 = BUTTON_COOLDOWN;
	}

	// encoder button
	if (cooldown_enc > 0) {
			cooldown_enc--;
			cur_state.enc_btn = 1;
	} else if (!READ_PIN(encoder_btn.port, encoder_btn.pin)) {
			cur_state.enc_btn = 1;
			cooldown_enc = BUTTON_COOLDOWN;
	}

	// onboard button
	if (cooldown_onboard > 0) {
			cooldown_onboard--;
			cur_state.onboard_btn = 1;
	} else if (READ_PIN(onboard_btn.port, onboard_btn.pin)) {
			cur_state.onboard_btn = 1;
			cooldown_onboard = BUTTON_COOLDOWN;
	}	
	if ((cur_state.ext1 == prev_state.ext1) && 	// if no change, return all zeros
			(cur_state.ext2 == prev_state.ext2) &&
			(cur_state.ext3 == prev_state.ext3) &&
			(cur_state.enc_btn == prev_state.enc_btn) &&
			(cur_state.onboard_btn == prev_state.onboard_btn)) {
		prev_state = cur_state;
		button_state_t no_change = {0,0,0,0,0};
		return no_change;
	}
	
	if (cur_state.ext1 && cur_state.ext2) {
		cooldown_ext1 = DOUBLE_PRESS_COOLDOWN;
		cooldown_ext2 = DOUBLE_PRESS_COOLDOWN;
	}
	prev_state = cur_state;

	return cur_state;
}

extern pin_t encoder_dt;
extern pin_t encoder_clk;

int encoder_read(void) {
    static uint8_t prev = 0;
    static int8_t acc = 0;  // accumulator for full detent

    // quadrature transition lookup table
    static int8_t table[16] = {
         0, +1, -1,  0,
        -1,  0,  0, +1,
        +1,  0,  0, -1,
         0, -1, +1,  0
    };

    int A = READ_PIN(encoder_clk.port, encoder_clk.pin);
    int B = READ_PIN(encoder_dt.port,  encoder_dt.pin);

    uint8_t curr = (A << 1) | B;
    uint8_t idx  = (prev << 2) | curr;
    prev = curr;

    int step = table[idx];  // +1, -1, or 0
    if (step == 0) return 0; // no change

    acc += step;             // accumulate transitions

    // assume 4 transitions per detent
    if (acc >= 4) {
        acc = 0;
        return +1;           // clockwise detent
    } else if (acc <= -4) {
        acc = 0;
        return -1;           // counterclockwise detent
    }

    return 0;                // still mid-detent
}



// LCD HELPERS ######################################################################################################################################################################################################
extern pin_t lcd_e;
extern pin_t lcd_rs;

// LCD instructions
#define LCD_CLEAR           0x01  // Clear display, cursor home
#define LCD_HOME            0x02  // Return cursor to home position
#define LCD_ENTRY_MODE      0x04  // Entry mode set (increment/decrement)
#define LCD_ENTRY_INC       0x02  // Increment cursor
#define LCD_ENTRY_DEC       0x00  // Decrement cursor
#define LCD_ENTRY_SHIFT     0x01  // Shift display when writing

#define LCD_DISPLAY_CTRL    0x08  // Display on/off control
#define LCD_DISPLAY_ON      0x04
#define LCD_DISPLAY_OFF     0x00
#define LCD_CURSOR_ON       0x02
#define LCD_CURSOR_OFF      0x00
#define LCD_BLINK_ON        0x01
#define LCD_BLINK_OFF       0x00

#define LCD_SHIFT           0x10  // Cursor or display shift
#define LCD_SHIFT_CURSOR    0x00
#define LCD_SHIFT_DISPLAY   0x08
#define LCD_SHIFT_RIGHT     0x04
#define LCD_SHIFT_LEFT      0x00

#define LCD_FUNCTION_SET    0x20  // Set interface data length, lines, font
#define LCD_8BIT_MODE       0x10
#define LCD_4BIT_MODE       0x00
#define LCD_2LINE           0x08
#define LCD_1LINE           0x00
#define LCD_5x10DOTS        0x04
#define LCD_5x8DOTS         0x00

#define LCD_SET_CGRAM_ADDR  0x40  // Set CGRAM address (custom chars)
#define LCD_SET_DDRAM_ADDR  0x80  // Set DDRAM address (cursor position)


lcd_t LCD = {
	.rs = &lcd_rs,
	.e  = &lcd_e,
	.send_command = lcd_send_command,
	.send_data    = lcd_send_data,
	.clear        = lcd_clear,
	.write_char   = lcd_write_char,
	.write_string = lcd_write_string,
	.place_cursor = lcd_place_cursor,
	.init         = lcd_init,
};

static void lcd_pulse_enable(void) {
	SET_PIN_HIGH(LCD.e->port, LCD.e->pin);
	// very short delay
	for (volatile int i=0; i<500; i++);
	SET_PIN_LOW(LCD.e->port, LCD.e->pin);
}

// send a command byte to the LCD
void lcd_send_command(uint8_t cmd) {
	SET_PIN_LOW(LCD.rs->port, LCD.rs->pin);  // RS=0 for command
	sr_send_MSB(cmd);                        // send via shift register
	lcd_pulse_enable();
}

// send a data byte (character) to the LCD
void lcd_send_data(uint8_t data) {
	SET_PIN_HIGH(LCD.rs->port, LCD.rs->pin); // RS=1 for data
	sr_send_MSB(data);                        // send via shift register
	lcd_pulse_enable();
}

// clear the LCD
void lcd_clear(void) {
	lcd_send_command(0x01);  // HD44780 clear display
	// wait ~1.6 ms (clear command)
	for (volatile int i=0; i<50000; i++);
}

// write a single character
void lcd_write_char(char c) {
	lcd_send_data((uint8_t)c);
}

void lcd_write_string(char* str) {
    while (*str) {
        lcd_write_char(*str);
        str++;
    }
}

// put cursor to location on screen
void lcd_place_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = col;
    switch(row)
    {
        case 0: addr += 0x00; break;
        case 1: addr += 0x40; break;
    }
    lcd_send_command(LCD_SET_DDRAM_ADDR | addr);
}

// simple init sequence
void lcd_init(void) {
	sr_reset();
	lcd_send_command(0x38); // function set 8-bit, 2 lines, 5x8 font
	lcd_send_command(0x0C); // display ON, cursor OFF
	lcd_send_command(0x06); // entry mode set: increment cursor
	lcd_clear();
}

// SHIFT REGISTER HELPERS ######################################################################################################################################################################################################
extern pin_t sr_data;
extern pin_t sr_clk;
extern pin_t sr_rst;

void sr_reset(void) { // pulses LOW on reset
    SET_PIN_HIGH(sr_rst.port, sr_rst.pin);
    SET_PIN_LOW(sr_rst.port, sr_rst.pin);
    SET_PIN_HIGH(sr_rst.port, sr_rst.pin);
}

void sr_send_MSB(uint8_t pattern) { // sends MSB first
    for (int i = 0; i < 8; i++) {
        if (pattern & (1 << (7 - i))) {
            SET_PIN_HIGH(sr_data.port, sr_data.pin);
        } else {
            SET_PIN_LOW(sr_data.port, sr_data.pin);
        }
        SET_PIN_HIGH(sr_clk.port, sr_clk.pin);
        SET_PIN_LOW(sr_clk.port, sr_clk.pin);
    }
}


void sr_send_LSB(uint8_t pattern) { // sends LSB first
    for (int i = 0; i < 8; i++) {
        if (pattern & (1 << i)) {
            SET_PIN_HIGH(sr_data.port, sr_data.pin);
        } else {
            SET_PIN_LOW(sr_data.port, sr_data.pin);
        }
        SET_PIN_HIGH(sr_clk.port, sr_clk.pin);
        SET_PIN_LOW(sr_clk.port, sr_clk.pin);
    }
}
