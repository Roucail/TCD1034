#ifndef MAIN_H
#define MAIN_H

#include "stm32f10x.h"
#include "User_TIM_Config.h"
#include "User_clock_config.h"
#include "User_Rx_IT_Tx_DMA_UART.h"
#include "User_ADC1_configuration.h"

/* command list Start = 'R'<<"RUN";
	Prescaler[] = "PSC";
	Autoreload[] = "ARR"; //set autoreload bit
	Single[] = "SGL";
	Update Timer[] = "UPE"; //send udate event to timer 2
	Stop[] = "STP";*/
#define	CMD_NUMBER						8 //number of command avaliable 
#define CMD_PRESCALER					0x00505343 	// PSC
#define CMD_PRESCALER_FETCH		0x00505346  // PSF
#define CMD_AUTORELOAD				0x00415252 	// ARR
#define CMD_AUTORELOAD_FETCH	0x00415246 	// ARF
#define CMD_SINGLE						0x0053474C	//SGL
#define CMD_RUN								0x0052554E	//RUN
#define CMD_UPDATE						0x00555045	//UPE
#define CMD_STOP							0x00535450	//STP
#define CMD_BINNING						0x0042494E  //BIN
#define CMD_BINNING_FETCH			0x00424946  //BIF


#ifndef USER_STATE_SYSTEM
#define USER_STATE_SYSTEM

#define __FLAG_TRANSMITION_COMPLETE	0x01
#define __FLAG_NEW_DATA_INPUT 			0x02
#define __FLAG_TRANSMIT_ADC 				0x04
#define __FLAG_CCD_CLEARED 					0x08
#define __FLAG_ACQUISITION_MODE_UP	0x10
#define __FLAG_UPDATE_EVENT					0x20
#define __FLAG_SINGLE								0x40
#define __FLAG_RUN									0x80

#define __STATE_STOP						0x00
#define __STATE_RUN_INIT				0x01
#define __STATE_RUN							0x02
#define __STATE_RUN_ADC 				0x03
#define __STATE_RUN_TRANSMIT 		0x04
#define __STATE_RESET						0x0A

#endif //USER_STATE_SYSTEM

#define SIZE_BUFFER_ADC 3694
#define SIZE_BUFFER_CMD 8 //"XXX 0000": XXX command name 0000 16 bits input
#define INIT_TIM2_ARR		5-1
#define INIT_TIM2_PSC		1-1

uint16_t Convert_ASCII_integer(const char*);
char Convert_HEXA_ASCII(const char);
void User_reset_system(void);
void User_Input_Cmd_Assess(void);
void Bin_data(void);

#endif //MAIN_H
