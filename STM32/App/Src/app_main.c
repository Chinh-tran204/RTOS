#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "main.h"
#include <string.h>
#include <semphr.h>

#define LED_PIN GPIO_PIN_7
#define BUTTON_PIN GPIO_PIN_0
#define BUILTIN_LED_PIN GPIO_PIN_13

extern UART_HandleTypeDef huart1;
//Queue handle for testing purposes
QueueHandle_t xQueue;
// Mutex for Logging
xSemaphoreHandle_t xLogMutex;
// Binary Semaphore for button press
SemaphoreHandle_t xButtonSemaphore;
// Timer for blinking LED handle
xTimerHandle_t xBlinkTimer;


/**
 * @brief Heartbeat Task
 * This task toggles an LED (usually PC13 on the Blue Pill board).
 */
static void vHeartbeatTask(void *pvParameters)
{
    // Frequency running from: 200 500 1000
    int type_of_freq[3] = {200, 500, 1000};
    int freq_counter = 0;
    uint32_t start_time = xTaskGetTickCount();
    for(;;)
    {   
        //change the freq after 5 seconds
        if (xTaskGetTickCount() - start_time >= 5000) {
            freq_counter = (freq_counter + 1) % 3;
	        start_time = xTaskGetTickCount();
        }
        HAL_GPIO_TogglePin(GPIOC, BUILTIN_LED_PIN);
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
 * @brief LED Blink Timer Callback
 * This function is called when the timer expires to toggle the LED.
 */
void vBlinkHandler(xTimerHandle_t xTimer)
{
    HAL_GPIO_TogglePin(GPIOA, LED_PIN);
}   

void vCommandBlinkHandler(void)
{
    if(xTimerIsTimerActive(xBlinkTimer) == pdFALSE){
        xTimerStart(xBlinkTimer, 0);
        vLog("Blink Timer Started\n");
    } else {
        xTimerStop(xBlinkTimer, 0);
        vLog("Blink Timer Stopped\n");
    }
}

/**
 * @brief Handler GPIO task
 */
// Button with debounce handler
void vButtonHandler(void *pvParameters){
    //turn on/off a LED when button is pressed == semaphore is given
    if(xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE){
        vTaskDelay(pdMS_TO_TICKS(50)); // Debounce delay
        // Wait until the button is released
        while(HAL_GPIO_ReadPin(GPIOA, BUTTON_PIN) == GPIO_PIN_RESET){
            vTaskDelay(pdMS_TO_TICKS(10)); // Polling delay
        }
        vCommandBlinkHandler();
    }
}

/**
 * @brief Logger Handler
 * Use by multiple tasks
 * Print Log through UART1 (for debugging purposes) 
 */
HAL_StatusTypeDef vLog(const char* msg)
{   
    uint32_t start_time = xTaskGetTickCount();
    while(xTaskGetTickCount() - start_time < 1000){
        // Check for the sempho and also the timeour
        if(xSemaphoreTake(xLogMutex, 10) == pdTRUE) {
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            xSemaphoreGive(xLogMutex);
            return HAL_OK;
        }
    } // Wait until we can take the mutex or timeout after 10 ticks
    return HAL_ERROR;
}



/**
 * @brief Application Main Entry
 * This function initializes tasks and starts the scheduler.
 */
void app_main(void)
{
    // Create a queue to hold 2 integers
    xQueue = xQueueCreate(2, sizeof(int));
    // Create a mutex for logging 
    xLogMutex = xSemaphoreCreateMutex(); 
    // Create a binary semaphore for button
    xButtonSemaphore = xSemaphoreCreateBinary();
    // Timer task for Blinking LED every 2 seconds
    xBlinkTimer = xTimerCreate(
        "BlinkTimer", 
        pdMS_TO_TICKS(100), 
        pdTRUE, 
        (void*)0, 
        vBlinkHandler
    );

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
    xTaskCreate(vButtonHandler, 
                "ButtonHandler", 
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
