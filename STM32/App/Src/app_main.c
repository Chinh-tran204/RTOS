#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

/**
 * @brief Heartbeat Task
 * This task toggles an LED (usually PC13 on the Blue Pill board).
 */
static void vHeartbeatTask(void *pvParameters)
{
    /* Initialize PC13 */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    for(;;)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Application Main Entry
 * This function initializes tasks and starts the scheduler.
 */
void app_main(void)
{
    /* Create application tasks */
    xTaskCreate(vHeartbeatTask, 
                "Heartbeat", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);

    /* In a production app, more tasks (e.g., UART communication, sensor reading) would be added here */

    /* The scheduler is started in the real main.c */
}

/* FreeRTOS Hook Functions */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Production-ready: Log overflow and reset or enter safe state */
    (void)xTask;
    (void)pcTaskName;
    for(;;);
}

void vApplicationMallocFailedHook(void)
{
    /* Production-ready: Log error and reset or enter safe state */
    for(;;);
}
