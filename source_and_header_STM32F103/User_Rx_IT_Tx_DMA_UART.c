#include "User_Rx_IT_Tx_DMA_UART.h"

/* Order of operation
1: configure_UART_GPIO_alternate_push_pull()
2: USAT2_DMA_DMA1Channel7Configuration_USART2Tx(SizeOfBuffer, BufferAddress)
Call DMA1_Channel7_Transfer_Complete_Assessement in the interrut handler (DMA1_Channel7_IRQHandler)
Call DMA_reset_Channel7_normal_mode to reset the DMA and send one burst of data
*/


void USART2_GPIO_Rx_Tx_alternate_push_pull_configure(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	 //switch on Port A clock
 	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;	 //switch on Alternate Function clock
	
	/*PA3*/GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3 );//Reset port PA3 as input
	/*PA3*/GPIOA->CRL |= GPIO_CRL_CNF3_0; //Floating 
	/*PA2*/GPIOA->CRL	&=  ~( GPIO_CRL_MODE2 | GPIO_CRL_CNF2);//Reset port PA2
	/*PA2*/GPIOA->CRL |= GPIO_CRL_CNF2_1;//USART2 Tx Alternate function push-pull (PA2)
	/*PA2*/GPIOA->CRL |= GPIO_CRL_MODE2_0; //fast output
}

void USART2_DMA_Rx_Tx_configure_115200bps(uint16_t SizeReception, char* BufferReception,uint16_t SizeTransmit, char* BufferTransmit)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //switch on USART2 clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; //switch on DMA1 clock
	
	USART2->CR1 &= ~USART_CR1_M; // Select 8 bits data
	USART2->CR2 &= ~USART_CR2_STOP; // Select 1 stop bit
  
	USART2->BRR |= 19 << 4; //Set baud rate at 115200bps (clock = 36MHz) 
	USART2->BRR |= 8; // Set baud rate at 115200bps (clock = 36MHz) (divider = (19 + 8/16))
	
	USART2_DMA1_Channel7_Configuration_Tx(SizeTransmit, BufferTransmit); //setup DMA configuration Tx
	USART2_DMA1_Channel6_Configuration_Rx(SizeReception, BufferReception); //setup DMA configuration Rx
	USART2_DMA_Interrupt();
	
	USART2->CR3 |= USART_CR3_DMAR; //enable DMA in USART2
	USART2->CR3 |= USART_CR3_DMAT; //enable DMA in USART2
	
	USART2->CR1 |= USART_CR1_RE; //Reveicer enable
	USART2->CR1 |= USART_CR1_TE; //Send waiting data
	
	USART2->SR &= ~USART_SR_TC; //Clear Transfert Complete bit
	USART2->CR1 |= USART_CR1_UE; //Enable USART
	
	DMA1_Channel6->CCR |= DMA_CCR6_EN; //enable DMA channel USART2 Rx
	DMA1_Channel7->CCR |= DMA_CCR7_EN; //enable DMA channel USART2 Tx
}

void USART2_DMA1_Channel6_Configuration_Rx(uint16_t BufferSize, char* BufferAddress)
{
	DMA1_Channel6->CCR &= (uint32_t) ~0x7F;
	DMA1_Channel6->CPAR = (uint32_t)&USART2->DR; //set peripheral address
	DMA1_Channel6->CMAR = (uint32_t) BufferAddress; //set data address
	DMA1_Channel6->CNDTR &= (uint32_t) ~0x0000FFFF; 
	DMA1_Channel6->CNDTR |= BufferSize; //set buffer size
	DMA1_Channel6->CCR |= (DMA_CCR6_PL_1 | //high priority 
				DMA_CCR6_MINC | //memory increment
				//DMA_CCR6_DIR | //read from memory
				DMA_CCR6_TCIE | //enable transfert complet interrupt
				DMA_CCR6_CIRC ); //circular mode enabled
	/* other bits: MSIZE = 00 : 8 bits, PSIZE = 00 : 8 bits, PINC: 0 no peripheral increment
	TEIE = 0 no transfer error interrupt, HTIE = 0 half transfert interrupt disabled
	DMA_CCR7_EN dma disabled*/ 
}

void USART2_DMA1_Channel7_Configuration_Tx(uint16_t BufferSize, char* BufferAddress)
{
	DMA1_Channel7->CCR &= (uint32_t) ~0x7F;
	DMA1_Channel7->CPAR = (uint32_t)&USART2->DR; //set peripheral address
	DMA1_Channel7->CMAR = (uint32_t) BufferAddress; //set data address
	DMA1_Channel7->CNDTR &= (uint32_t) ~0x0000FFFF; 
	DMA1_Channel7->CNDTR |= BufferSize; //set buffer size
	DMA1_Channel7->CCR |= (DMA_CCR7_PL_0 | //medium priority 
				DMA_CCR7_MINC | //memory increment
				DMA_CCR7_DIR | //read from memory
				DMA_CCR7_TCIE );//| //enable transfert complet interrupt
	/* other bits: MSIZE = 00 : 8 bits, PSIZE = 00 : 8 bits, PINC: 0 no peripheral increment
	TEIE = 0 no transfer error interrupt, HTIE = 0 half transfert interrupt disabled
	DMA_CCR7_CIRC circular mode disabled, DMA_CCR7_EN dma disabled*/ 
}

void USART2_DMA_Interrupt(void)
{
	//set DMA interrupt
	NVIC->ISER[0] |= NVIC_ISER_SETENA_16; //Interrupt Rx
	NVIC->IP[16] |=  (8 << 4);	
	NVIC->ISER[0] |= NVIC_ISER_SETENA_17; //Interrupt Tx
	NVIC->IP[17] |=  (6 << 4);
}

void DMA1_Channel6_Receive_Complete_Assessement(void)
{
	//DMA1_Channel6->CCR &= (uint32_t) ~DMA_CCR6_EN; //disable DMA channel USART2 Rx (avoid constant stream to be sent)
	DMA1->IFCR |= DMA_IFCR_CGIF6 | DMA_IFCR_CHTIF6 | DMA_IFCR_CTCIF6 | DMA_IFCR_CTEIF6; //reset all flags (only CTCIF should be set)
}

void DMA1_Channel7_Transfer_Complete_Assessement(void)
{
	//DMA1_Channel7->CCR &= (uint32_t) ~DMA_CCR7_EN; //disable DMA channel USART2 Tx (avoid constant stream to be sent)
	while(!(USART2->SR & USART_SR_TC))
	{		
	}
	DMA1->IFCR |= DMA_IFCR_CGIF7 | DMA_IFCR_CHTIF7 | DMA_IFCR_CTCIF7 | DMA_IFCR_CTEIF7; //reset all flags (only CTCIF should be set)
}

void DMA1_channel7_UART_Tx_reset(uint16_t BufferSize, char* BufferAddress)
{
	DMA1_Channel7->CCR &= ~(uint32_t)DMA_CCR7_EN; //disable DMA channel USART2 Tx
	DMA1_Channel7->CMAR = (uint32_t) BufferAddress; //set data address
	DMA1_Channel7->CNDTR &= (uint32_t) ~0x0000FFFF; 
	DMA1_Channel7->CNDTR |= BufferSize; //set buffer size
	DMA1->IFCR |= DMA_IFCR_CGIF7 | DMA_IFCR_CHTIF7 | DMA_IFCR_CTCIF7 | DMA_IFCR_CTEIF7; //reset all flags (only CTCIF should be set)
	DMA1_Channel7->CCR |= DMA_CCR7_EN; //enable DMA channel USART2 Tx
	//USART2->SR &= ~USART_SR_TC; //Clear Transfert Complete bit
}

void DMA1_channel6_UART_Rx_reset(uint16_t BufferSize, char* BufferAddress)
{
	DMA1_Channel6->CCR &= ~(uint32_t)DMA_CCR6_EN; //disable DMA channel USART2 Rx
	DMA1_Channel6->CMAR = (uint32_t) BufferAddress; //set data address
	DMA1_Channel6->CNDTR &= (uint32_t) ~0x0000FFFF; 
	DMA1_Channel6->CNDTR |= BufferSize; //set buffer size
	DMA1->IFCR |= DMA_IFCR_CGIF6 | DMA_IFCR_CHTIF6 | DMA_IFCR_CTCIF6 | DMA_IFCR_CTEIF6; //reset all flags (only CTCIF should be set)
	DMA1_Channel6->CCR |= DMA_CCR6_EN; //enable DMA channel USART2 Rx
}
