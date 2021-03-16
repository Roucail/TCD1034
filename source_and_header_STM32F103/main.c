#include "main.h"


volatile char ADC_Data_char[2*SIZE_BUFFER_ADC];
char data_to_return[SIZE_BUFFER_CMD] = "RDY:STOP";
char reception[SIZE_BUFFER_CMD];
volatile uint8_t acquisition_mode = __STATE_STOP;
volatile uint8_t User_flag = 0x00; //NU NU NU NU CCDClear TransmitADC NewDataIn TransmitionComplete
volatile uint16_t TIM2_AutoReload_user = INIT_TIM2_ARR; //initialise is 10 µs
volatile uint16_t TIM2_Prescaler_user = INIT_TIM2_PSC;
volatile uint8_t binning = 1;
volatile uint16_t binning_shift = 0;
volatile uint16_t length_data_ADC_Binned = 2*SIZE_BUFFER_ADC;

//	GPIOA->ODR = GPIOA->ODR ^ (1 << 5);

int main (void) 
{	
	//local variable 
	uint16_t j=0;
		
	//set clock and port configuration
	SysClockConfig();
	GPIOConfig();
	
	
	for(j=0;j<SIZE_BUFFER_ADC;j++)
	{
		ADC_Data_char[j] = '-';
	}
	User_flag |= __FLAG_TRANSMITION_COMPLETE;
	
	
	User_ADC1_injected_IT_configuration(); //configure ADC
	USART2_GPIO_Rx_Tx_alternate_push_pull_configure(); //configure UART I/O
	USART2_DMA_Rx_Tx_configure_115200bps(SIZE_BUFFER_CMD,reception,SIZE_BUFFER_CMD,data_to_return); //configure UART to send and receive data
	
	//configure timers
	Tim1Config();
	Tim2Config();
	setTim2IntegrationTime(TIM2_Prescaler_user,TIM2_AutoReload_user);
	Tim3Config();
	TIM1->CR1 |= TIM_CR1_CEN; //enable timer 1
	TIM2->CR1 |= TIM_CR1_CEN; //enable timer 2 (timer 3 can be triggered)

	GPIOA->ODR |= (1 << 5);   
	while(1)
	{
		if((User_flag  & (__FLAG_TRANSMIT_ADC | __FLAG_NEW_DATA_INPUT | __FLAG_TRANSMITION_COMPLETE)) 
						== (__FLAG_NEW_DATA_INPUT | __FLAG_TRANSMITION_COMPLETE))
		{
			User_Input_Cmd_Assess();

			User_flag &= ~__FLAG_NEW_DATA_INPUT;
			User_flag &= ~__FLAG_TRANSMITION_COMPLETE;
			DMA1_channel7_UART_Tx_reset(SIZE_BUFFER_CMD,data_to_return);
			while((User_flag & __FLAG_TRANSMITION_COMPLETE) == 0)//!(User_flag & __FLAG_TRANSMITION_COMPLETE))
			{ 
			}
			User_flag |= __FLAG_ACQUISITION_MODE_UP;
		}
		if(User_flag & __FLAG_ACQUISITION_MODE_UP)
		{
			User_flag &= ~__FLAG_ACQUISITION_MODE_UP;
			switch(acquisition_mode)
			{
				case __STATE_STOP: 
				{
					break;
				}
				case __STATE_RESET:
				{
					User_reset_system();
					setTim2IntegrationTime(INIT_TIM2_PSC,INIT_TIM2_ARR); //Set TIM2 at 10 µs 
					TIM1->EGR = TIM_EGR_UG; //set update event to preload shadow register 
					acquisition_mode = __STATE_STOP;
					User_flag  |= __FLAG_ACQUISITION_MODE_UP;
					break;
				}
				case __STATE_RUN_INIT: 
				{
					User_reset_system(); 
					setTim2IntegrationTime(INIT_TIM2_PSC,INIT_TIM2_ARR);
					TIM2->EGR = TIM_EGR_UG; //set update event to preload shadow register 
					acquisition_mode = __STATE_RUN;
					User_flag  |= __FLAG_ACQUISITION_MODE_UP;
					break;
				}
				case __STATE_RUN:
				{
					TIM2->SR &= ~0x0001; //~TIM_SR_UIF; //clear flag
					TIM3->SR &= ~0x0001; //~TIM_SR_UIF; //clear flag
					interruptInitTim2();
					break;
				}
				case __STATE_RUN_ADC:
				{
					ADC1->SR &= ~(ADC_SR_JEOC | ADC_SR_EOC); //clear ADC flags	
					NVIC->ISER[0] = NVIC_ISER_SETENA_18; //enable ADC interrupt
					ADC1->CR1 |= ADC_CR1_JEOCIE;
					break;
				}
				case __STATE_RUN_TRANSMIT:
				{
					User_flag &= ~__FLAG_TRANSMITION_COMPLETE;
					data_to_return[0] = 'S';
					data_to_return[1] = 'E';
					data_to_return[2] = 'N';
					data_to_return[3] = 'D';
					data_to_return[4] = 'I';
					data_to_return[5] = 'N';
					data_to_return[6] = 'G';
					data_to_return[7] = ':';
					DMA1_channel7_UART_Tx_reset(SIZE_BUFFER_CMD,data_to_return);
					while((User_flag & __FLAG_TRANSMITION_COMPLETE) == 0);//!(User_flag & __FLAG_TRANSMITION_COMPLETE))
					
					User_flag &= ~__FLAG_TRANSMITION_COMPLETE;
					DMA1_channel7_UART_Tx_reset(length_data_ADC_Binned,ADC_Data_char);
					GPIOA->ODR = GPIOA->ODR ^ (1 << 5);
					while((User_flag & __FLAG_TRANSMITION_COMPLETE) == 0);//!(User_flag & __FLAG_TRANSMITION_COMPLETE))	
					GPIOA->ODR = GPIOA->ODR ^ (1 << 5);
					
					User_flag &= ~__FLAG_TRANSMITION_COMPLETE;
					data_to_return[0] = 'F';
					data_to_return[1] = 'I';
					data_to_return[2] = 'N';
					data_to_return[3] = 'I';
					data_to_return[4] = 'S';
					data_to_return[5] = 'H';
					data_to_return[6] = 'E';
					data_to_return[7] = 'D';
					DMA1_channel7_UART_Tx_reset(SIZE_BUFFER_CMD,data_to_return);
					while((User_flag & __FLAG_TRANSMITION_COMPLETE) == 0);//!(User_flag & __FLAG_TRANSMITION_COMPLETE))
					
					if((User_flag & __FLAG_SINGLE) == __FLAG_SINGLE)
					{
						User_flag &= ~(__FLAG_SINGLE);
						acquisition_mode = __STATE_RESET;
						User_flag  |= __FLAG_ACQUISITION_MODE_UP;
					}
					else if((User_flag & __FLAG_RUN) == __FLAG_RUN)
					{
						acquisition_mode = __STATE_RUN;
						User_flag  |= __FLAG_ACQUISITION_MODE_UP;
					}
					break;
			}
			default: break;
			}
		}
	}
} 

void User_Input_Cmd_Assess(void)
{
	uint32_t Command_In = 0;
	uint16_t command_Value = 0;
	uint8_t i=0;
	
	Command_In = ((uint32_t)	reception[0] << 16) + ((uint32_t)reception[1] << 8) + (uint32_t)reception[2] ; //convert first three character into number
	command_Value = Convert_ASCII_integer(reception); //convert last 4 hexadecimal number [0_9][A_F] (a_f not included) as uint16_t 

	switch(Command_In)
	{
		case CMD_AUTORELOAD: 
		{
			if(!((acquisition_mode == __STATE_RESET) || (acquisition_mode == __STATE_STOP)))
			{
				TIM2->ARR = command_Value; 
			}
			TIM2_AutoReload_user = command_Value; 
			reception[2] = 'F'; //change this to fetch the data to return result
			Command_In = 0;
			User_Input_Cmd_Assess();
			break;
		}
		case CMD_PRESCALER: 
		{
			if(!((acquisition_mode == __STATE_RESET) || (acquisition_mode == __STATE_STOP)))
			{
				TIM2->PSC = command_Value; 
			}
			TIM2_Prescaler_user = command_Value;
			reception[2] = 'F'; //change this to fetch the data to return result
			Command_In = 0;
			User_Input_Cmd_Assess();
			break;
		}
		case CMD_BINNING: 
		{
			if (command_Value==1) 
			{
				binning = (uint8_t)command_Value;
				binning_shift = 0;
			}
			else if (command_Value==2) 
			{
				binning = (uint8_t)command_Value;
				binning_shift = 0;
			}
			else if (command_Value==4) 
			{
				binning = (uint8_t)command_Value;
				binning_shift = 0;
			}
			else if (command_Value==8) 
			{
				binning = (uint8_t)command_Value;
				binning_shift = 0;
			}
			else if (command_Value==16) 
			{
				binning = (uint8_t)command_Value;
				binning_shift = 0;
			}
			else if (command_Value==32) 
			{
				binning = (uint8_t)command_Value;
				binning_shift = 1;
			}
			else if (command_Value==64) 
			{
				binning = (uint8_t)command_Value; 
				binning_shift = 2;
			}
			else if (command_Value==128) 
			{
				binning = (uint8_t)command_Value; 
				binning_shift = 3;
			}
			reception[2] = 'F'; //change this to fetch the data to return result
			Command_In = 0;
			User_Input_Cmd_Assess();
			break;
		}
		case CMD_SINGLE:
		{
			acquisition_mode = __STATE_RUN_INIT;
			User_flag &= ~ __FLAG_RUN;
			User_flag |= __FLAG_SINGLE;
			
			data_to_return[0] = ' ';
			data_to_return[1] = 'S';
			data_to_return[2] = 'I';
			data_to_return[3] = 'N';
			data_to_return[4] = 'G';
			data_to_return[5] = 'L';
			data_to_return[6] = 'E';
			data_to_return[7] = ' ';
			
			User_flag  |= __FLAG_ACQUISITION_MODE_UP;
			break;
		}
		case CMD_RUN: 
		{
			acquisition_mode = __STATE_RUN_INIT;
			User_flag &= ~__FLAG_SINGLE;
			User_flag |= __FLAG_RUN;
			
			data_to_return[0] = 'R';
			data_to_return[1] = 'U';
			data_to_return[2] = 'N';
			data_to_return[3] = ' ';
			data_to_return[4] = 'A';
			data_to_return[5] = 'C';
			data_to_return[6] = 'Q';
			data_to_return[7] = '.';
			
			User_flag  |= __FLAG_ACQUISITION_MODE_UP;
			break;
		}
		case CMD_UPDATE:
		{
			NVIC->ICER[0] = NVIC_ISER_SETENA_29; //disable interruption from timer 3
			NVIC->ICER[0] = NVIC_ISER_SETENA_28; //disable interruption from timer 2
			TIM3->DIER &= ~TIM_DIER_UIE; //disable update interrupt from timer 3
			TIM2->DIER &= ~TIM_DIER_UIE; //disable update interrupt from timer 2

			User_flag &= ~__FLAG_CCD_CLEARED;
			User_flag  |= (__FLAG_ACQUISITION_MODE_UP | __FLAG_UPDATE_EVENT);
			acquisition_mode = __STATE_RUN_INIT;
			
			interruptInitTim2();

			data_to_return[0] = 'T';
			data_to_return[1] = 'I';
			data_to_return[2] = 'M';
			data_to_return[3] = 'E';
			data_to_return[4] = 'R';
			data_to_return[5] = ' ';
			data_to_return[6] = 'U';
			data_to_return[7] = 'P';
			
			TIM2->EGR = TIM_EGR_UG;
			break;
		}
		case CMD_STOP: 
		{
			acquisition_mode = __STATE_RESET; 
			data_to_return[0] = 'S';
			data_to_return[1] = 'T';
			data_to_return[2] = 'O';
			data_to_return[3] = 'P';
			data_to_return[4] = 'I';
			data_to_return[5] = 'N';
			data_to_return[6] = 'G';
			data_to_return[7] = '!';
			break;
		}
		case CMD_BINNING_FETCH:
		{
			data_to_return[0] = 'B';
			data_to_return[1] = 'I';
			data_to_return[2] = 'N';
			data_to_return[3] = ' ';
			data_to_return[4] = '0';
			data_to_return[5] = '0';
			data_to_return[6] = Convert_HEXA_ASCII((char) ( (binning>> 4) & 0xF	)); //TIM2->ARR
			data_to_return[7] = Convert_HEXA_ASCII((char) (		binning & 0xF			));
			break;
		}
		case CMD_AUTORELOAD_FETCH:
		{
			data_to_return[0] = 'A';
			data_to_return[1] = 'R';
			data_to_return[2] = 'R';
			data_to_return[3] = ' ';
			data_to_return[4] = Convert_HEXA_ASCII((char) ((TIM2_AutoReload_user >> 12) & 0xF ));
			data_to_return[5] = Convert_HEXA_ASCII((char) ( (TIM2_AutoReload_user >> 8) & 0xF	));
			data_to_return[6] = Convert_HEXA_ASCII((char) ( (TIM2_AutoReload_user >> 4) & 0xF	)); //TIM2->ARR
			data_to_return[7] = Convert_HEXA_ASCII((char) (		TIM2_AutoReload_user & 0xF			));
			break;
		}
		case CMD_PRESCALER_FETCH:
		{
			data_to_return[0] = 'P';
			data_to_return[1] = 'S';
			data_to_return[2] = 'C';
			data_to_return[3] = ' ';
			data_to_return[4] = Convert_HEXA_ASCII((char) ( (TIM2_Prescaler_user >> 12) & 0xF ));
			data_to_return[5] = Convert_HEXA_ASCII((char) ( (TIM2_Prescaler_user >> 8) & 0xF	));
			data_to_return[6] = Convert_HEXA_ASCII((char) ( (TIM2_Prescaler_user >> 4) & 0xF	)); //TIM2->PSC 
			data_to_return[7] = Convert_HEXA_ASCII((char) (		TIM2_Prescaler_user  & 0xF			));
			break;
		}
		default:
		{
			DMA1_channel6_UART_Rx_reset(SIZE_BUFFER_CMD,reception);
			data_to_return[0] = 'C';
			data_to_return[1] = 'M';
			data_to_return[2] = 'D';
			data_to_return[3] = ' ';
			data_to_return[4] = 'E';
			data_to_return[5] = 'R';
			data_to_return[6] = 'R';
			data_to_return[7] = '!';
			break;
		}
	}	
}
char Convert_HEXA_ASCII(char to_convert_to_ascii)
{
	if(to_convert_to_ascii<10)			return to_convert_to_ascii+'0';
	else if(to_convert_to_ascii<16) return to_convert_to_ascii-10+'A';
	else 														return '0';
}

uint16_t Convert_ASCII_integer(const char* to_convert_to_integer)
{
	/* Convert ASCII character to their respective hexadecimal value
	Support only 0123456789ABCDEF (case sensitive) any other character
	will return a wrong value. No error assessement is done.
	*/
	uint16_t converted_ASCII = 0; 
	char current_character = '0';
	uint8_t i=0;
	
	for(i=0;i<4;i++)
	{
		current_character = to_convert_to_integer[4+i];
		if(current_character<='9')
		{
			converted_ASCII += (current_character-'0') << ((3-i)*4); //assume a character between 0 and 9
		}
		else
		{
			converted_ASCII += (current_character-'A'+10) << ((3-i)*4); // assume a character between A and F
		}
	}
	return converted_ASCII;
}

void User_reset_system(void)
{
	DMA1_channel7_UART_Tx_reset(0,data_to_return);
	DMA1_channel6_UART_Rx_reset(SIZE_BUFFER_CMD,reception);
	User_flag &= ~(__FLAG_CCD_CLEARED | __FLAG_TRANSMIT_ADC);
	User_flag |= __FLAG_TRANSMITION_COMPLETE;
	
	while(!(User_flag & __FLAG_TRANSMITION_COMPLETE));//!(User_flag & __FLAG_TRANSMITION_COMPLETE))  
	
	NVIC->ICER[0] = NVIC_ISER_SETENA_29; //disable interruption from timer 3
	NVIC->ICER[0] = NVIC_ISER_SETENA_28; //disable interruption from timer 2
	TIM3->DIER &= ~TIM_DIER_UIE; //disable update interrupt from timer 3
	TIM2->DIER &= ~TIM_DIER_UIE; //disable update interrupt from timer 2
}
