#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f1xx_hal.h"

void Error_Handler(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);

#endif /* __MAIN_H */
