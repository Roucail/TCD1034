#include "User_clock_config.h"

void SysClockConfig(void)
{
	/*************>>>>>>> STEPS FOLLOWED <<<<<<<<************
	Enable external 8MHz crystal and pll to run the system at 72 MHz
	APB1 run at 72 MHz
	APB2 run at 36 MHz
	ADC run at 12 MHz
	********************************************************/
	//	1. ENABLE HSE and wait for the HSE to become Ready
	RCC->CR |= RCC_CR_HSEON;	//Enable
	while(!(RCC->CR & RCC_CR_HSERDY)); //wait
	
	//	2. Set the POWER ENABLE CLOCK
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	
	//	3. Configure the FLASH PREFETCH and the LATENCY Related Settings
	FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;
	FLASH->ACR &= ~(FLASH_ACR_LATENCY_0 | FLASH_ACR_LATENCY_2);
	
	//	4. Configure the PRESCALARS HCLK, PCLK1, PCLK2 
	//prescaler AHB 1, ADC 6, APB1 1, APB2 2,
	RCC->CFGR |= RCC_CFGR_ADCPRE_1 | RCC_CFGR_PPRE1_2;
	RCC->CFGR &= ~(RCC_CFGR_HPRE_3 |RCC_CFGR_ADCPRE_0 | RCC_CFGR_PPRE2_2 | RCC_CFGR_PPRE1_1 | RCC_CFGR_PPRE1_0);
	
	//	5. Configure the MAIN PLL
	//PLL mul 9 (pllmul 0111), PLL source HSE(pllsrc), and not divided (pllxtpre)
	RCC->CFGR |= RCC_CFGR_PLLMULL_0 | RCC_CFGR_PLLMULL_1 | RCC_CFGR_PLLMULL_2 | RCC_CFGR_PLLSRC_HSE;
	RCC->CFGR &= ~(RCC_CFGR_PLLMULL_3 |RCC_CFGR_PLLXTPRE);
	
	
	//	6. Enable the PLL and wait for it to become ready
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY));
	
	//	7. Select the Clock Source and wait for it to be set (PLL as system clock(sw = 10))
	RCC->CFGR |= RCC_CFGR_SW_1;
	RCC->CFGR &= ~(RCC_CFGR_SW_0);
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	
	
}
