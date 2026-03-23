#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/* Forward declaration for the application's main entry */
extern void app_main(void);

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
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* Standard 72MHz configuration for STM32F103-C8-T6 (Blue Pill) */
  /* In a real project, this would include PLL, HSE, and HCLK settings */
}

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
