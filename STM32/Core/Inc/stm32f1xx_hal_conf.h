#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H

#include "stm32f1xx_hal.h" // This might cause recursion, actually hal.h includes hal_conf.h

// Usually hal_conf.h defines which modules to use
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED

// Oscillator settings
#define HSE_VALUE    ((uint32_t)8000000)
#define HSI_VALUE    ((uint32_t)8000000)
#define LSI_VALUE    ((uint32_t)40000)
#define LSE_VALUE    ((uint32_t)32768)
#define VDD_VALUE    ((uint32_t)3300)
#define TICK_INT_PRIORITY            ((uint32_t)0)
#define USE_RTOS                     0
#define PREFETCH_ENABLE              1

#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_pwr.h"
#include "stm32f1xx_hal_cortex.h"

#endif /* __STM32F1xx_HAL_CONF_H */
