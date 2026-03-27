#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <semphr.h>

#define BUILTIN_LED_PIN GPIO_PIN_13
#define LED_PIN GPIO_PIN_7

//Queue handle for testing purposes
extern QueueHandle_t xQueue;
// Mutex for Logging
extern xSemaphoreHandle_t xLogMutex;
//UART handle for testing purposes
extern UART_HandleTypeDef huart1; 
// Timer for blinking LED handle
extern xTimerHandle_t xBlinkTimer;
extern xSemaphoreHandle_t xLEDTimerMutex; // Mutex for LED timer
extern xTimeHandle_t xDebounceTimer; // Timer for button debouncing

// Initialize multiple LED peripheral (for testing purposes)

/* Initialize PC13 */
__HAL_RCC_GPIOA_CLK_ENABLE();
__HAL_RCC_GPIOC_CLK_ENABLE();
// Initialize PC13
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = BUILTIN_LED_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
// Initialize PA7
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = LED_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
 * @brief Handler GPIO task
 */
// Button debounce handler
void vDebounceHandler(xTimerHandle_t xTimer){
    // check the semaphore state
    if(xSemaphoreTake(xButtonSemaphore, 0) == pdFALSE){
        return pdTRUE; // If the semaphore is already taken
    } else {
        return pdFALSE; // If the button is still pressed after debounce time, we consider it an Invalid press
    }
}
// button handler task
void vButtonHandler(void *pvParameters){
    //turn on/off a LED when button is pressed == semaphore is given
    if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE) {
        // recheck the button state after debounce time
        if(xTimerStart(xDebounceTimer, 0) == pdTRUE){
            xSemaphoreGive(xLEDTimerMutex); // Give the mutex to allow the blink timer to be started/stopped
        }
    }
}

/**
 * @brief Logger Handler
 * Use by multiple tasks
 * Print Log through UART1 (for debugging purposes) 
 */
void vLog(const char* msg)
{
    while(xSemaphoreTake(xLogMutex, 10) != pdTRUE) {} // Wait until we can take the mutex
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    xSemaphoreGive(xLogMutex);
}

/**
 * @brief LED Blink Timer Callback
 * This function is called when the timer expires to toggle the LED.
 */
void vBlinkHandler(xTimerHandle_t xTimer)
{
    HAL_GPIO_TogglePin(GPIOA, LED_PIN);
}   

void vCommandBlinkHandler(void *pvParameters)
{
    if(xSemaphoreTake(xLEDTimerMutex, portMAX_DELAY == pdTRUE){
        if(xTimerIsTimerActive(xBlinkTimer) == pdFALSE){
            xTimerStart(xBlinkTimer, 0);
            vLog("Blink Timer Started\n");
        } else {
            xTimerStop(xBlinkTimer, 0);
            vLog("Blink Timer Stopped\n");
        }
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
    xTaskCreate(vCommandBlinkHandler, 
                "BlinkCommand", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);
    /* In a production app, more tasks (e.g., UART communication, sensor reading) would be added here */
    // Timer task for Blinking LED every 2 seconds
    xBlinkTimer = xTimerCreate(
        "BlinkTimer", 
        pdMS_TO_TICKS(100), 
        pdTRUE, 
        (void*)0, 
        vBlinkHandler
    );

    xDebounceTimer = xTimerCreate(
        "DebounceTimer", 
        pdMS_TO_TICKS(50), 
        pdFALSE, 
        (void*)0, 
        vDebounceHandler
    );
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
