# Hints for RTOS Training Tasks

Don't look at these until you've tried to find the answer in the official [FreeRTOS documentation](https://www.freertos.org/a00106.html).

---

### Hint 1: The Variable Heartbeat
*   **API:** Use `vTaskDelay(pdMS_TO_TICKS(100))` for timing.
*   **Logic:** Use a `uint32_t` variable for the delay period and a `uint32_t` counter to track time or a simple modulo operator inside your loop.

### Hint 2: The Producer-Consumer (Queues)
*   **Init:** Call `xQueueCreate(10, sizeof(uint32_t))` in `app_main`.
*   **Safety:** The `xQueueReceive` call should have a timeout (e.g., `portMAX_DELAY`) to let the task "block" while waiting for data. This is how you save CPU cycles.

### Hint 3: The Protected Logger (Mutexes)
*   **Safety:** `xSemaphoreTake(xMutex, portMAX_DELAY)` before printing, then `xSemaphoreGive(xMutex)` after.
*   **HAL:** Use `HAL_UART_Transmit` for the actual character sending.

### Hint 4: The Fast Trigger (Binary Semaphores)
*   **Interrupt:** The STM32 HAL `HAL_GPIO_EXTI_Callback` is where you'll find the interrupt. 
*   **Critical:** Inside an ISR, you **MUST** use `xSemaphoreGiveFromISR`. Never use the standard version in an interrupt!
*   **Priority:** The Handler task should have a higher priority (e.g., `3`) than the Heartbeat (e.g., `1`).

### Hint 5: The Timed Watchdog (Software Timers)
*   **Config:** Check `FreeRTOSConfig.h` for `configUSE_TIMERS`.
*   **Callback:** Your timer callback function must be "lean"—no long loops or blocking calls (no `vTaskDelay`).

### Hint 6: Stack Sentinel (Memory Management)
*   **Function:** Use `uxTaskGetStackHighWaterMark(NULL)` to see how many "words" of stack are remaining in the current task. If it's near zero, you're about to crash.

### Hint 7: Priority Inversion Demonstration
*   **Observation:** When Task H is blocked waiting for Task L's Mutex, and Task M is running, Task H is effectively being preempted by a lower-priority task (Task M). FreeRTOS handles this with *Priority Inheritance*—Task L temporarily "inherits" Task H's priority to finish its work and release the Mutex.

### Hint 8: Static vs Dynamic Allocation
*   **Buffers:** You'll need to define a `StaticTask_t` and a `StackType_t xStack[configMINIMAL_STACK_SIZE]`.
*   **Requirement:** You must implement `vApplicationGetIdleTaskMemory` when using static allocation.

### Hint 9: The Logic Gate (Event Groups)
*   **API:** `xEventGroupSetBits` from each "condition" task and `xEventGroupWaitBits` from your gatekeeper task.
*   **Flag:** Set `xClearOnExit` to `pdTRUE` so the bits are reset after the gatekeeper executes.

### Hint 10: Performance Profiling
*   **Timer:** You need a high-frequency timer (10x faster than the Tick rate) to measure CPU usage. You can use a spare STM32 hardware timer or just read the DWT (Data Watchpoint and Trace) cycle counter for simplicity.
*   **Display:** Use `vTaskList` and `vTaskGetRunTimeStats`. You'll need to allocate a large buffer (e.g., 400 bytes) to hold the string output.
