#include "User_TIM_Config.h"

void GPIOConfig(void)
{
	//	1. Enable GPIO clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // GPIO A
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // GPIO B
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN; // GPIO D (for the cristal)
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // AFIO output enable
	
	
    GPIOA->CRL &= ~(0xF << 4*5); //output PA5 led
    GPIOA->CRL |= (0x1 << 4*5);
}


void Tim1Config(void)
{
	// 0. Set GPIO output
	//	0.1. Set the TIM1 PIN PA8 as alternate function output
	GPIOA->CRH |= GPIO_CRH_CNF8_1;
	GPIOA->CRH &= ~GPIO_CRH_CNF8_0;
	
	//	0.2. Configure the output mode i.e state, speed, and push-pull
	//set max speed 50 MHz
	GPIOA->CRH |= GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0;
	
	// 1. Enable Timer clock
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;  // Enable the timer1 clock
	
	// 2. set the timer 1 in PWM mode and send a update event on it's trigger output
	TIM1->CCER &= ~TIM_CCER_CC1E; //disable Channel 1
	
	TIM1->CR1 |= TIM_CR1_ARPE; //auto-reload preregister enabled
	TIM1->CR2 |= TIM_CR2_MMS_1; 	
	TIM1->CR2 &= ~(TIM_CR2_MMS_2|TIM_CR2_MMS_0);//Update event selected a TRGO 	
	
	TIM1->CCMR1 &=~(TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC1S_1); //CC1 configured as output
	TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Channel 1 PWM mode 1
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE; // preload enable
	
	TIM1->CCER |= TIM_CCER_CC1P; //Active high
	
	// 3. Set the prescalar and the ARR
	TIM1->PSC = 1-1;  // 72MHz clock frequency
	TIM1->ARR = 36-1;  // 72MHz/36 = 2 MHz
	TIM1->CCR1 = 18; //capture/compare register (duty cycle: 50%)
	TIM1->RCR &= ~(0xFF);
	TIM1->RCR |= 4-1; //repetition counter register 
	
	TIM1->CCER |= TIM_CCER_CC1E; //Enable Channel 1
	TIM1->BDTR |= TIM_BDTR_MOE; //main output enable (for advanced timer only)
	TIM1->EGR = TIM_EGR_UG; //set update event to preload shadow register 
}


void Tim2Config(void)
{
	
	// 1. Enable Timer clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable the timer2 clock
	
	TIM2->CR1 |= TIM_CR1_ARPE; //auto-reload preregister enabled
	TIM2->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0; //timer on external clock mode 1	(input clock = 0.5 MHz)
	TIM2->SMCR &= ~(TIM_SMCR_TS_2 | TIM_SMCR_TS_1 | TIM_SMCR_TS_0); //select ITR0 as trigger input (TIM1)
	//set timer parameter
	TIM2->PSC = 500-1;  // 0.1 MHz clock frequency
	TIM2->ARR = 1000-1;  // set intergration time
	
	TIM2->CR2 |= TIM_CR2_MMS_1;
	TIM2->CR2 &= ~(TIM_CR2_MMS_0 | TIM_CR2_MMS_2);
	
	NVIC->IP[28] = NVIC->IP[28] | (7 << 4); //fix priority 7 to the interrupt on timer 2	
	TIM2->EGR = TIM_EGR_UG; //set update event to preload shadow register 
}

void Tim3Config(void)
{
	static uint16_t delaypulse = 36*2+5; //allows a delay of 2 µs after the timer interrupt
	// 0. Set GPIO output
	//	0.1. Set the TIM2 PIN (PA6, PA7, PB0) as alternate function output
	GPIOA->CRL |= GPIO_CRL_CNF6_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF6_0;
	GPIOA->CRL |= GPIO_CRL_CNF7_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF7_0;
	GPIOB->CRL |= GPIO_CRL_CNF0_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF0_0;
	//	0.2. Configure the output mode i.e state, speed, and push-pull
	//set max speed 50 MHz
	GPIOA->CRL |= GPIO_CRL_MODE6_1 | GPIO_CRL_MODE6_0;
	GPIOA->CRL |= GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0;
	GPIOB->CRL |= GPIO_CRL_MODE0_1 | GPIO_CRL_MODE0_0;
	
	// 1. Enable Timer clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Enable the timer3 clock
	
	// 2. Set the prescalar and the ARR
	TIM3->CCER &= ~TIM_CCER_CC1E; //disable Channel 1
	TIM3->CCER &= ~TIM_CCER_CC2E; //disable Channel 1
	TIM3->CCER &= ~TIM_CCER_CC3E; //disable Channel 1
	
	TIM3->CCMR1 &=~(TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC1S_1); //CC1 configured as output
	TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE; // Channel 1 PWM mode 1 / preload enable
	TIM3->CCMR1 &=~(TIM_CCMR1_CC2S_0 | TIM_CCMR1_CC2S_1); //CC2 configured as output
	TIM3->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
	TIM3->CCMR1 &= ~TIM_CCMR1_OC2M_0;
	TIM3->CCMR1 |= TIM_CCMR1_OC2PE; // Channel 1 PWM mode 1 / preload enable
	TIM3->CCMR2 &=~(TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC3S_1); //CC3 configured as output
	TIM3->CCMR2 |= TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_2;
	TIM3->CCMR2 &= ~(TIM_CCMR2_OC3M_1);
	TIM3->CCMR2 |= TIM_CCMR2_OC3PE; // Channel 1 PWM mode 1 / preload enable
		
	TIM3->CR1 |= TIM_CR1_ARPE; //auto-reload preregister enabled
	TIM3->CR1 |= TIM_CR1_OPM; //One pulse mode
		
	TIM3->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1; //timer as trigger mode
	TIM3->SMCR &= ~TIM_SMCR_SMS_0;
	TIM3->SMCR &= ~(TIM_SMCR_TS_2 | TIM_SMCR_TS_1 );
	TIM3->SMCR |= TIM_SMCR_TS_0; //select ITR1 as trigger input (TIM2)
	
	TIM3->CCER |= TIM_CCER_CC1P; //Active high
	TIM3->CCER |= TIM_CCER_CC2P; //Active high
	TIM3->CCER |= TIM_CCER_CC3P; //Active high
	
	TIM3->PSC = 1-1;  // 72MHz clock frequency
	TIM3->ARR = 720-1+delaypulse+10;  // 720/72 = 10 µs long pulse (add 10 clk for synchronization purpose)
	TIM3->CCR1 = 36+delaypulse; //capture/compare register SH pulse part 1
	TIM3->CCR2 = 360+delaypulse; //capture/compare register SH pulse part 2
	TIM3->CCR3 = 1+delaypulse; //capture/compare register ICG pulse
	
	
	TIM3->CCER |= TIM_CCER_CC1E; //Enable Channel 1
	TIM3->CCER |= TIM_CCER_CC2E; //Enable Channel 2
	TIM3->CCER |= TIM_CCER_CC3E; //Enable Channel 3
	
	
	NVIC->IP[29] = NVIC->IP[29] | (6 << 4); //fix priority 7 to the interrupt on timer 2	
	TIM3->EGR = TIM_EGR_UG; //set update event to preload shadow register 
}

void setTim2IntegrationTime(uint16_t prescaler, uint16_t autoreload)
{
	TIM2->PSC = prescaler;
	TIM2->ARR = autoreload;
}

void interruptInitTim2(void)
{
	TIM2->SR &= ~TIM_SR_UIF; //clear flag
	NVIC->ISER[0] = NVIC_ISER_SETENA_28; //set interruption from timer 2
	TIM2->DIER |= TIM_DIER_UIE; //update interrupt enable from timer 2
}

void interruptInitTim3(void)
{
	TIM3->SR &= ~TIM_SR_UIF; //clear flag
	NVIC->ISER[0] = NVIC_ISER_SETENA_29; //set interruption from timer 3
	TIM3->DIER |= TIM_DIER_UIE; //update interrupt enable from timer 3
}
