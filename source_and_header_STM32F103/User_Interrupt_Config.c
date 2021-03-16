#include "User_Interrupt_Config.h"

extern volatile uint8_t acquisition_mode;
extern char* data_to_return;
extern char* reception;
extern volatile char ADC_Data_char[2*SIZE_BUFFER_ADC];
extern volatile uint8_t User_flag ; //flags : NU NU NU NU CCDCleared TransmitADC NewDataIn TransmitionComplete
extern volatile uint16_t TIM2_AutoReload_user;
extern volatile uint16_t TIM2_Prescaler_user;
extern volatile uint8_t binning;
extern volatile uint16_t binning_shift;
extern volatile uint16_t length_data_ADC_Binned;


void DMA1_Channel7_IRQHandler(void) //Tx
{ 
	User_flag |= __FLAG_TRANSMITION_COMPLETE;
	DMA1_Channel7_Transfer_Complete_Assessement();
}

void DMA1_Channel6_IRQHandler(void) //Rx
{ 
	User_flag |= __FLAG_NEW_DATA_INPUT;
	DMA1_Channel6_Receive_Complete_Assessement();
}


void ADC1_2_IRQHandler(void)
{
	//initialize data
	static uint16_t ADC_count = 2*SIZE_BUFFER_ADC; //set ADC down counter
	static uint8_t ADC_binning_count = 0;
	static uint16_t local_length_data = 0;
	static uint16_t temporary_Binning = 0;
	uint16_t temporary_ADC_buffer = 0;
	
	if((User_flag & __FLAG_UPDATE_EVENT) == __FLAG_UPDATE_EVENT)
	{
		ADC_count = 2*SIZE_BUFFER_ADC;
		User_flag &= ~(__FLAG_UPDATE_EVENT);
	}
	if(ADC_count == 2*SIZE_BUFFER_ADC) // reset counter and buffer if a new batch of data is aquired
	{
		local_length_data = 0;	
		ADC_binning_count = 0;
		local_length_data = 0;
		temporary_Binning = 0;
	}
	//read injected channel
	temporary_ADC_buffer = (uint16_t) ADC1->JDR1;
	if(ADC_binning_count<(binning-1))
	{
		temporary_Binning = (temporary_ADC_buffer >> binning_shift) + temporary_Binning;
		ADC_binning_count++;
	}
	else
	{		
		temporary_Binning = (temporary_ADC_buffer >> binning_shift) + temporary_Binning;
		ADC_Data_char[local_length_data] = (char) ((temporary_Binning & (0xFF00)) >> 8 ); //write the most significant bits 
		local_length_data++;
		ADC_Data_char[local_length_data] = (char) (temporary_Binning & (0x00FF)); //write the least significant bits
		local_length_data++;
		temporary_Binning = 0;
		ADC_binning_count = 0;
	}
	ADC_count--;
	ADC_count--;
	
	if(ADC_count == 0)
	{
		NVIC->ICER[0] = NVIC_ISER_SETENA_18; //disable ADC interrupt
		ADC1->CR1 &= ~ADC_CR1_JEOCIE; //disable end of conversion interrupt
		acquisition_mode = __STATE_RUN_TRANSMIT;
		User_flag |= __FLAG_ACQUISITION_MODE_UP; 
		ADC_count = 2*SIZE_BUFFER_ADC; //reset counter
		length_data_ADC_Binned = local_length_data;
		
	} 
	ADC1->SR &= ~(ADC_SR_JEOC | ADC_SR_EOC); //clear flags	
}

void TIM2_IRQHandler(void) //timer on exposure
{ 
	static uint32_t counter_TIM2 = NUMBER_OF_PULSE_TO_CLEAR_CCD;
	//OCxPE need to be enable so the TIM3 CCRx is changed on the next interrupt from TIM2
	if (acquisition_mode == __STATE_RUN)
	{
		if ((User_flag & __FLAG_CCD_CLEARED) == 0x00)
		{ 
			//set the capture/compare register on the channel 3 to output a SH pulse to clear de CCD
			TIM3->CCMR2 |= TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_2; //disable ICG pluse
			TIM3->CCMR2 &= ~(TIM_CCMR2_OC3M_1);
			
			counter_TIM2--;
			if (counter_TIM2 == 1)
			{
				TIM2->PSC = TIM2_Prescaler_user;
				TIM2->ARR = TIM2_AutoReload_user;
			}
			if (counter_TIM2 == 0)
			{
				counter_TIM2 = NUMBER_OF_PULSE_TO_CLEAR_CCD;
				User_flag |= __FLAG_CCD_CLEARED;
			}
		}
		else 
		{
			//set the capture/compare register on the channel 3 to output a pulse 		
			TIM3->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;
			TIM3->CCMR2 &= ~TIM_CCMR2_OC3M_0; //set as PWM mode 1
			interruptInitTim3(); //enable timer 3 interrupt		
		}
	}
	TIM2->SR &= ~TIM_SR_UIF; //clear flag
}



void TIM3_IRQHandler(void) //end of ICG and SH pulse
{ 
	NVIC->ICER[0] = NVIC_ISER_SETENA_29; //disable interruption from timer 3
	NVIC->ICER[0] = NVIC_ISER_SETENA_28; //disable interruption from timer 2
	TIM3->DIER &= ~TIM_DIER_UIE; //disable update interrupt from timer 3
	TIM2->DIER &= ~TIM_DIER_UIE; //disable update interrupt from timer 2
	
	TIM3->SR &= ~TIM_SR_UIF; //clear flag
	
	TIM3->CCMR2 |= TIM_CCMR2_OC3M_0 | TIM_CCMR2_OC3M_2; //disable ICG pluse
	TIM3->CCMR2 &= ~(TIM_CCMR2_OC3M_1);
	
	if(acquisition_mode == __STATE_RUN)
	{
		acquisition_mode = __STATE_RUN_ADC;
		User_flag |= __FLAG_ACQUISITION_MODE_UP;
	}
}
