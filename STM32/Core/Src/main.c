#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include <semphr.h>
#include <stdio.h>

#define LED_PIN GPIO_PIN_7
#define BUTTON_PIN GPIO_PIN_0
#define BUILTIN_LED_PIN GPIO_PIN_13

UART_HandleTypeDef huart1;


/* Forward declaration for the application's main entry */
extern void app_main(void);

// Binary semaphore for button press (if needed in the future)
// xSemaphoreHandle xButtonSemaphore;
extern SemaphoreHandle_t xButtonSemaphore;

/**
 * @brief Retargets the C library printf function to the USART.
 * @param file: file descriptor
 * @param ptr: pointer to data
 * @param len: length of data
 * @retval length of data
 */
int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

/**
  * @brief  The hardware entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock to 72MHz (Standard for STM32F103) */
  SystemClock_Config();
  MX_GPIO_Init();

  // Init uart
  MX_USART1_UART_Init();
  /* Initialize all configured peripherals */
  /* In production, UARTs, I2Cs, SPIs etc. are initialized here or in app_main() */

  /* Call the application-specific setup */
  app_main();

  /* Start the FreeRTOS scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  while (1)
  {
  }
}

/**
 * UART setting init function (for debugging purposes) 
 */
void MX_USART1_UART_Init(void)
{
  /* Standard UART initialization code for STM32F103 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/** 
 * @brief GPIO init config
 * @param None
 * Config the GPIO pins with IRQ enabled for button input (if needed in the future) 
 */
void MX_GPIO_Init(void){

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Port A clock enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin : GPIO_PIN_0 (PA0) */
  GPIO_InitStruct.Pin = BUTTON_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LOCK_Pin BUT_LED_Pin BUZZ_Pin */
  GPIO_InitStruct.Pin  = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Initialize PC13
  GPIO_InitStruct.Pin = BUILTIN_LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0); // Set priority lower than MAX_SYSCALL_INTERRUPT_PRIORITY
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

 /**
  * @brief GPIO EXTI Callback (for handling external interrupts, e.g., buttons)
  * @param GPIO_Pin: The pin number that triggered the interrupt
  * @retval Non
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // Modify the semaphore binary
  if(GPIO_Pin == GPIO_PIN_0) {
    //set the binary semopho | Give semapho
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* Standard 72MHz configuration for STM32F103-C8-T6 (Blue Pill) */
  /* In a real project, this would include PLL, HSE, and HCLK settings */
}
// No need for receiving for now cause nothing get to recive, but this is how it would look like if we want to use it in the future
// void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
// 	// LOG UART Handle
// 	if (huart->Instance == USART1) {
// 		if(Size < UART_RX_BUFFER_SIZE)
// 		{
// 			LOG_buffer[Size] = '\0';
// 		} else {
// 			LOG_buffer[UART_RX_BUFFER_SIZE - 1] = '\0';
// 		}
// 		LOG_DataValid = true;
// 		HAL_UARTEx_ReceiveToIdle_IT(&huart1, LOG_buffer, UART_RX_BUFFER_SIZE);
// 	}
// }

/**
  * @brief  Error Handler
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
