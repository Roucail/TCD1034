#ifndef USER_SYSTEM_CONFIG_H
#define USER_SYSTEM_CONFIG_H

#include "stm32f10x.h"

void GPIOConfig(void);
void Tim1Config(void);
void Tim2Config(void);
void Tim3Config(void);

void setTim2IntegrationTime(uint16_t, uint16_t);

void interruptInitTim2(void);
void interruptInitTim3(void);

#endif //USER_SYSTEM_CONFIG_H
