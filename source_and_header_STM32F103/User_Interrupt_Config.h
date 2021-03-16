#ifndef USER_INTERRUPT_CONFIG_H
#define USER_INTERRUPT_CONFIG_H

#include "stm32f10x.h"
#include "main.h"

#define NUMBER_OF_PULSE_TO_CLEAR_CCD 10

void DMA1_Channel7_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);


void ADC1_2_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);


#endif //USER_INTERRUPT_CONFIG_H
