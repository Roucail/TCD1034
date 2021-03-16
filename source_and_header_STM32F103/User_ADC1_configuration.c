#include "User_ADC1_configuration.h"

void User_ADC1_injected_IT_configuration(void)
{
	uint16_t i= 0;
	//set the PA0 channel as analog input
	RCC->APB2ENR |=RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~(GPIO_CRL_MODE0_1 | GPIO_CRL_MODE0_0 | GPIO_CRL_CNF0_1 | GPIO_CRL_CNF0_0);
	
	//enable ADC1 clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	
	//set sample time 7.5 (total conversion time : 7.5+12.5 = 20 Aclk)
	ADC1->SMPR2 |= ADC_SMPR2_SMP0_0;
	ADC1->SMPR2 &= ~(ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1);

	//set triggered injection channel in the ADC by TIM1 TRGO from channel 0; 
	ADC1->CR1 |= ADC_CR1_SCAN;
	ADC1->CR1 &= ~ADC_CR1_JAUTO; //disable automatic converion after regular conversion
	ADC1->CR2 &= ~(ADC_CR2_JEXTSEL_2 | ADC_CR2_JEXTSEL_1 | ADC_CR2_JEXTSEL_0 ); //TIM1 TRGO as trigger for injected conversion
	ADC1->JSQR &=~((uint32_t) 0x7FFFF); //set 1 conversion on channel 0 (bit JL[1:0] and JSQx[4:0]

	ADC1->CR2|= ADC_CR2_ADON; //switch on ADC1
	while(i<10000)
	{
		__ASM("NOP"); //wait a bit to start ADC calibration
		i++;
	}
	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);
	
	ADC1->CR1 |= ADC_CR1_JEOCIE; //enable end of conversion interrupt from injected channel
	ADC1->CR2 |= ADC_CR2_JEXTTRIG; //conversion on external trigger selected
	
	//NVIC->ISER[0] |= NVIC_ISER_SETENA_18; //enable ADC interrupt
	NVIC->IP[18] |=  (4 << 4); //set priority 4 for ADC
}

