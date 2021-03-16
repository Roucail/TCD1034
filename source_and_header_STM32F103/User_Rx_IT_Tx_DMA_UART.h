#ifndef USER_RX_IT_TX_DMA_UART_H
#define USER_RX_IT_TX_DMA_UART_H

#include "stm32f10x.h"


void USART2_GPIO_Rx_Tx_alternate_push_pull_configure(void);
void USART2_DMA_Rx_Tx_configure_115200bps(uint16_t, char*,uint16_t, char*);
void USART2_DMA1_Channel6_Configuration_Rx(uint16_t, char*);
void USART2_DMA1_Channel7_Configuration_Tx(uint16_t, char*);
void USART2_DMA_Interrupt(void);
	
void DMA1_Channel6_Receive_Complete_Assessement(void);
void DMA1_Channel7_Transfer_Complete_Assessement(void);

void DMA1_channel7_UART_Tx_reset(uint16_t, char*);
void DMA1_channel6_UART_Rx_reset(uint16_t, char*);

#endif //USER_LIGHT_TX_DMA_UART_H
