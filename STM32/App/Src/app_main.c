#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

//Queue handle for testing purposes
QueueHandle_t xQueue;
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
    // Frequency running from: 200 500 1000
    int type_of_freq[3] = {200, 500, 1000};
    int freq_counter = 0;
    uint32_t start_time = HAL_GetTick();
    for(;;)
    {   
        //change the freq after 5 seconds
        if (HAL_GetTicks()%5000 == 0) {
            freq_counter = (freq_counter + 1) % 3;
        }
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        vTaskDelay(pdMS_TO_TICKS(type_of_freq[freq_counter]));
    }
}

/**
 * @brief Producer task (for testing purposes)
 * This task read info from dummy sensor and update the queue every 1 second.
 */

static void vProducerTask(void *pvParameters)
{
    int dummy_sensor_value = 0;
    for(;;){
        dummy_sensor_value++; // Simulate reading from a sensor
        //update value to queue
        xQueueSend(xQueue, &dummy_sensor_value, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

static void vConsumerTask(void *pvParameters) {
    //read from the queue 
    int received_value = 0;
    for(;;){
        if(xQueueReceive(xQueue, &received_value, portMAX_DELAY) == pdPASS){
            // Process the received value (e.g., print it)
            printf("Received value: %d\n", received_value);
        }
    }
}

/**
 * @brief Application Main Entry
 * This function initializes tasks and starts the scheduler.
 */
void app_main(void)
{
    xQueue = xQueueCreate(2, sizeof(int)); // Create a queue to hold 2 integers
    /* Create application tasks */
    xTaskCreate(vHeartbeatTask, 
                "Heartbeat", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);
    xTaskCreate(vProducerTask, 
                "Producer", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);
    xTaskCreate(vConsumerTask, 
                "Consumer", 
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
