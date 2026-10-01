#pragma once

#include  "stm32f10x.h"


#define SET_PIN_HIGH(port, pin)   ((port)->BSRR = (1U << (pin)))
#define SET_PIN_LOW(port, pin)    ((port)->BSRR = (1U << ((pin) + 16)))
#define READ_PIN(port, pin)   (((port)->IDR & (1U << (pin))) ? 1 : 0)


void RCC_config(void);
void delay(vu32 nCount);
char *int_to_str(int v, char *b);
void flash_save_int(uint32_t *address, uint16_t value);
uint16_t flash_load_int(uint32_t *address, uint16_t default_value, uint16_t max_value);
