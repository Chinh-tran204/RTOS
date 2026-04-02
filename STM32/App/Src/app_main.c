#include "timers.h"
#include "main.h"

#define LED_PIN GPIO_PIN_7
#define BUTTON_PIN GPIO_PIN_0
#define BUILTIN_LED_PIN GPIO_PIN_13

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
//Queue handle for testing purposes
QueueHandle_t xQueue;
// Mutex for Logging
xSemaphoreHandle_t xLogMutex;
xSemaphoreHandle_t xtestMutex; // for testing purposes, delete it later
// Binary Semaphore for button press
SemaphoreHandle_t xButtonSemaphore;
// Timer for blinking LED handle
xTimerHandle_t xBlinkTimer;
xEventGroupHandle_t xInitializeCheckList; // For testing event group, delete it later


// Static task allocation
static StaticTask_t xHeartbeatTaskTCB;
static StackType_t xHeartbeatTaskStack[configMINIMAL_STACK_SIZE];
/********************************************************** BASIC FUNCTIONS **************************************************/

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
 * @brief LED Blink Timer Callback
 * This function is called when the timer expires to toggle the LED.
 */
void vBlinkHandler(xTimerHandle_t xTimer)
{
    HAL_GPIO_TogglePin(GPIOA, LED_PIN);
}

void vCommandBlinkHandler(void)
{
    if(xTimerIsTimerActive(xBlinkTimer) == pdTRUE){
        xTimerStop(xBlinkTimer, 0);
        vLog("Blink Timer Stopped\n");
    } else {
        xTimerStart(xBlinkTimer, 0);
        vLog("Blink Timer Started\n");
    }
}

HAL_StatusTypeDef sensorCalibrate(void){
    // Simulate sensor calibration
    vLog("Calibrating sensor...\n");
    HAL_Delay(1000); // Simulate time-consuming calibration
    xEventGroupSetBits(xInitializeCheckList, SENSOR_INIT); // Set the event bit for sensor initialization
    return HAL_OK;
}

int sensorRead(void){
    return rand() % 100; // Simulate reading a sensor value (0-99)
}

/********************************************************** TASK HANDLE **************************************************/
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
    sensorCalibrate(); // Simulate sensor calibration at startup
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second before starting the loop
    int sensor_value = sensorRead();
    for(;;){
        sensor_value = sensorRead(); // Simulate reading from a sensor
        //update value to queue
        xQueueSend(xQueue, &sensor_value, portMAX_DELAY);
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
// Button with debounce handler
static void vButtonHandlerTask(void *pvParameters){
    for(;;)
    {
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
}


/**
 * @brief Overflow task
 * This task is use to test overflow
 * see how things get get handled when stack overflow happens 
 */
static void vOverflowTask(void *pvParameters)
{
    int arr[256]; // Large array to quickly consume stack space
    for(;;){}
}


/**
 * @brief Task for testing priority inversion
 * all task after this section is for testing and have no real purpose
 * delete it afterwards for clean up
 */
static void vlowPriorityTask(void *pvParameters){
    for(;;){
        // Take the mutex
        if(xSemaphoreTake(xtestMutex, portMAX_DELAY) == pdTRUE){
            vLog("Low priority task has taken the mutex\n");
            HAL_Delay(5000); // Simulate doing some work while holding the mutex for 5 seconds
            xSemaphoreGive(xtestMutex);
            vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep after releasing
        }
    }
}
static void vmediumPriorityTask(void *pvParameters){
    vTaskDelay(pdMS_TO_TICKS(100)); // Ensure low priority task runs first and takes the mutex
    //after suspend
    vLog("Medium priority task is running\n");
    for(;;){
        // Simulate medium priority task doing some work
        vTaskDelay(pdMS_TO_TICKS(1000));
    } // Block the task indefinitely to simulate it being active and preventing the low priority task from runningdaj 
}
static void vhighPriorityTask(void *pvParameters){
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(100)); // Ensure low priority task runs first and takes the mutex
        vLog("High priority task is trying to take the mutex\n");
        if(xSemaphoreTake(xtestMutex, portMAX_DELAY) == pdTRUE){
            vLog("High priority task has taken the mutex\n");
            xSemaphoreGive(xtestMutex);
            vTaskDelay(pdMS_TO_TICKS(2000)); // Sleep after releasing
        }
    }
}

static void vSystemCheckTask(void *pvParameters){
    const EventBits_t xBitsToWaitFor = UART_INIT | SENSOR_INIT | USER_LOGIN;
    xEventGroupWaitBits(
        xInitializeCheckList,
        xBitsToWaitFor,
        pdFALSE, // Don't clear bits on exit
        pdTRUE,  // Wait for all bits
        portMAX_DELAY //wait forever for this to happen
    );
    vLog("All system components initialized. System check passed!\n");
    vTaskDelete(NULL); // Delete this task after the check is done
}

static void vSystemReportTask(void *pvParameters){
    //refresh time stay at 1 second
    //lend space for task, task struct
    uint32_t RunTime;
    if(taskStatusArray != NULL){
        for(;;){
            vLog("[LOG]SYSTEM_STATE_RP\n");
            //count the task
            UBaseType_t tasks = uxTaskGetNumberOfTasks();
            //get the system state
            TaskStatus_t *taskStatusArray = pvPortMalloc(tasks * sizeof(TaskStatus_t));
            tasks = uxTaskGetSystemState(taskStatusArray, tasks, &RunTime);
            //print the system state
            for(UBaseType_t i = 0; i < tasks; i++){
                if(Runtime > 0){
                    uint32_t taskRunTime = taskStatusArray[i].ulRunTimeCounter;     //take task specify runtime
                    uint32_t taskCPUPercentage = (taskRunTime * 100UL) / RunTime; //calculate the percentage
                } else{
                    uint32_t taskCPUPercentage = 0; // Avoid division by zero
                }
                char log_msg[100];
                snprintf(log_msg, sizeof(log_msg), "Task: %s, State: %u, Priority: %u, CPU Usage: %lu%%\n\r", 
                        taskStatusArray[i].pcTaskName, 
                        taskStatusArray[i].eCurrentState, 
                        taskStatusArray[i].uxCurrentPriority, 
                        taskCPUPercentage);
                vLog(log_msg);
                vPortFree(taskStatusArray); // Free the allocated memory after use
            }   
            //delay for refresh time
            vTaskDelay(pdMS_TO_TICKS(SYS_RP_RFTIM));
        }
    }    
}
/**
 * @brief Application Main Entry
 * This function initializes tasks and starts the scheduler.
 */
void app_main(void)
{
    // Set the user setting event group
    xEventGroupSetBits(xInitializeCheckList, USER_LOGIN); 
    // Create a queue to hold 2 integers
    xQueue = xQueueCreate(2, sizeof(int));
    // Create a mutex for logging 
    xLogMutex = xSemaphoreCreateMutex(); 
    // test out mutex for priority inversion, delete it later
    xtestMutex = xSemaphoreCreateMutex();
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
    xTaskCreate(vSystemReportTask, 
                "SystemReport", 
                configMINIMAL_STACK_SIZE * 2, 
                NULL, 
                4, 
                NULL);
    xTaskCreate(vSystemCheckTask, 
                "SystemCheck", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);
    xTaskCreateStatic(vHeartbeatTask, 
                "Heartbeat", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1,
                xHeartbeatTaskStack, 
                &xHeartbeatTaskTCB);
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
    xTaskCreate(vButtonHandlerTask, 
                "ButtonHandler", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);
    xTaskCreate(vOverflowTask,
                "Overflowstack",
                ((unsigned short)100), //60-70 for initiate the task, 100 for overflow
                NULL,
                1,
                NULL);
    // Tasks for testing priority inversion
    xTaskCreate(vlowPriorityTask, 
                "LowPriority", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                1, 
                NULL);
    xTaskCreate(vmediumPriorityTask, 
                "MediumPriority", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                2, 
                NULL);
    xTaskCreate(vhighPriorityTask, 
                "HighPriority", 
                configMINIMAL_STACK_SIZE, 
                NULL, 
                3, 
                NULL);
    /* In a production app, more tasks (e.g., UART communication, sensor reading) would be added here */
    /* The scheduler is started in the real main.c */
}

/* FreeRTOS Hook Functions */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Production-ready: Log overflow and reset or enter safe state */
    // let us pass the compile first, we will handle the overflow later
    (void)xTask;
    (void)pcTaskName;
    // Disable the task or interrupt it for safe
    taskDISABLE_INTERRUPTS();
    // log the things
    vLog("Fatal Error: Stack overflow!!!\n\r");
    char log_msg[50];
    snprintf(log_msg, sizeof(log_msg), "Stack overflow in task: %s\n\r", pcTaskName);
    vLog(log_msg);
    while(1){
        HAL_GPIO_TogglePin(GPIOC, BUILTIN_LED_PIN);
        HAL_Delay(100); // Blink every 100ms to indicate error state
    }
}

void vApplicationMallocFailedHook(void)
{
    /* Production-ready: Log error and reset or enter safe state */
    for(;;);
}

// Create for static allocation is enabled, for running idle task when system is not running anything 
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t, configMINIMAL_STACK_SIZE
    is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
// Create for static allocation is enabled, for running timer task when system is not running anything
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    // 1. Create the physical memory (Must be static!)
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    // 2. Hand the pointers back to FreeRTOS
    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH; 
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void sysHighClock(void){
    //start up the cloc for system profiling
    HAL_TIM_Base_Start(&htim2);
}

uint32_t getSystemTicks(void){
    return TIM2->CNT; // Return the current value of the timer counter
}