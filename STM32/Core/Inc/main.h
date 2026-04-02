#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semphr.h>
#include "FreeRTOS.h"
#include "task.h"

void Error_Handler(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);


#define LED_PIN GPIO_PIN_7
#define BUTTON_PIN GPIO_PIN_0
#define BUILTIN_LED_PIN GPIO_PIN_13

#define UART_INIT (1 << 0)
#define SENSOR_INIT (1 << 1)
#define USER_LOGIN (1 << 2)

#define SYS_RP_RFTIM 1000
#endif /* __MAIN_H */
