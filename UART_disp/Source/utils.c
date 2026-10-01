#include "utils.h"

/*RCC initialization*/
void RCC_config(void){
	RCC->CR|=0x10000; //HSE on
	while(!(RCC->CR&0x20000)){} //wait for HSE ready
	  //flash access setup
  	FLASH->ACR &= 0x00000038;   //mask register
  	FLASH->ACR |= 0x00000002;   //2 wait states

 	FLASH->ACR &= 0xFFFFFFEF;   //mask register
    FLASH->ACR |= 0x00000010;   //enable Prefetch Buffer

	RCC->CFGR&=0xFFC3FFFF; //mask PLLMUL
	RCC->CFGR|=0x1<<18;    //set PLLMUL x3
	RCC->CFGR|=0x0<<17;    //set PREDIV1 x1
	RCC->CFGR|=0x10000;    //PLL clock source = PREDIV1
	RCC->CFGR&=0xFFFFFF0F; //HPRE=1x
	RCC->CFGR&=0xFFFFF8FF; //PPRE2=1x
	RCC->CFGR&=0xFFFFC7FF; //PPRE1=1x

	RCC->CR|=0x01000000; //PLL on
	while(!(RCC->CR&0x02000000)){} //wait for PLL ready

	RCC->CFGR&=0xFFFFFFFC;
	RCC->CFGR|=0x2; //set PLL as SYSCLK source

  	while(!(RCC->CFGR & 0x00000008)) //SYSCLK ready?
  	{
  	}

	// Enable GPIO clocks for ports A, B, and C
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN   // GPIOA clock
								 | RCC_APB2ENR_IOPBEN   // GPIOB clock
								 | RCC_APB2ENR_IOPCEN;  // GPIOC clock
		
	// Enable USART1 clock
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

/*Delay loop, roughly delays by nCount core ticks*/
void delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}

char *int_to_str(int v, char *b) {
    if (v < 0) v = 0;       // optional: clamp negatives if you don't need them
    if (v < 10) {
        b[0] = ' ';         // leading space
        b[1] = '0' + v;     // digit
    } else {
        b[0] = '0' + (v / 10);
        b[1] = '0' + (v % 10);
    }
    b[2] = '\0';
    return b;
}



// flash helpers

void flash_unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {      // only unlock if locked
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
    }
}

void flash_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

int flash_erase_page(uint32_t address) {
    flash_unlock();
    while (FLASH->SR & FLASH_SR_BSY);      // wait if busy

    FLASH->CR |= FLASH_CR_PER;             // page erase enable
    FLASH->AR = address;
    FLASH->CR |= FLASH_CR_STRT;            // start erase
    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR &= ~FLASH_CR_PER;

    flash_lock();
    return 0;
}

int flash_write_halfword(uint32_t address, uint16_t data) {
    flash_unlock();
    while (FLASH->SR & FLASH_SR_BSY);

    FLASH->CR |= FLASH_CR_PG;
    *(volatile uint16_t*)address = data;
    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR &= ~FLASH_CR_PG;

    flash_lock();
    return 0;
}


// save a 16-bit integer to flash, only writes if changed
void flash_save_int(uint32_t *address, uint16_t value) {
	static uint16_t prev_saved = 0xFFFF; // invalid default to force first write
	if (prev_saved == value) return;     // no change

	// erase page
	flash_erase_page((uint32_t)address);

	// write value
	flash_write_halfword((uint32_t)address, value);
	prev_saved = value;
}

// load a 16-bit integer from flash with optional sanity check */
uint16_t flash_load_int(uint32_t *address, uint16_t default_value, uint16_t max_value) {
	uint16_t val = *address;
	if (val > max_value) val = default_value; // optional sanity check
	return val;
}
