# RTOS Training: 10 Progression Tasks

Welcome to the team. These tasks are designed to take you from a "fresher" to a competent RTOS developer. Complete them in order.

## Phase 1: The Basics (Easy)

### Task 1: The Variable Heartbeat
Modify the existing `vHeartbeatTask` in `app_main.c`. Instead of a fixed 500ms toggle, make the LED blink frequency change every 5 seconds (e.g., 100ms for 5 seconds, then 1000ms for 5 seconds).
*   **Goal:** Understand `vTaskDelay` and basic task loops.

### Task 2: The Producer-Consumer (Queues)
Create two new tasks: `vProducerTask` and `vConsumerTask`.
1.  The Producer should "read" a dummy sensor value (incrementing integer) and send it to a Queue every 1 second.
2.  The Consumer should receive the value from the Queue.
3.  **Goal:** Learn safe inter-task communication using `xQueueCreate`, `xQueueSend`, and `xQueueReceive`.

## Phase 2: Synchronization & Resources (Medium)

### Task 3: The Protected Logger (Mutexes)
Initialize UART1 in `main.c`. Create a function `void vLog(char *msg)`. This function will be called by multiple tasks to print debug info. Use a Mutex to ensure that one task doesn't interrupt another's print statement (preventing "garbled" text).
*   **Goal:** Understand resource contention and `xSemaphoreCreateMutex`.

### Task 4: The Fast Trigger (Binary Semaphores)
Configure a GPIO input (e.g., PA0) with an External Interrupt (EXTI). 
1.  The ISR (Interrupt Service Routine) should NOT do work. It should only "give" a binary semaphore.
2.  A high-priority `vHandlerTask` should wait for that semaphore and toggle a second LED when the button is pressed.
*   **Goal:** Learn "Deferred Interrupt Processing" using `xSemaphoreGiveFromISR`.

### Task 5: The Timed Watchdog (Software Timers)
Instead of a task, use a FreeRTOS Software Timer to toggle an LED. Then, create a "Command Task" that can start or stop this timer based on a simulated command.
*   **Goal:** Learn when to use `xTimerCreate` instead of a full task to save RAM.

## Phase 3: Advanced Architectures (Hard)

### Task 6: Stack Sentinel (Memory Management)
Purposefully create a task with a very small stack (e.g., 32 words) and call a function with large local arrays to trigger a stack overflow. Implement the `vApplicationStackOverflowHook` to catch it and toggle the LED rapidly.
*   **Goal:** Learn to debug memory issues using `uxTaskGetStackHighWaterMark`.

### Task 7: Priority Inversion Demonstration
Create three tasks: Low (L), Medium (M), and High (H).
1.  L takes a Mutex.
2.  H tries to take the same Mutex and blocks.
3.  M (which doesn't use the Mutex) starts running and prevents L from finishing, thus blocking H.
*   **Goal:** Observe how FreeRTOS handles Priority Inheritance.

### Task 8: Static vs Dynamic Allocation
The current template uses dynamic allocation (Heap). Re-implement the Heartbeat task using `xTaskCreateStatic`. You will need to provide the stack buffer and task control block manually.
*   **Goal:** Learn how to build systems for safety-critical environments (where `malloc` is often forbidden).

## Phase 4: Expert Level (System Mastery)

### Task 9: The Logic Gate (Event Groups)
Create a task that only executes when three different conditions are met (e.g., "UART Ready", "Sensor Calibrated", and "User Logged In"). Use an Event Group to synchronize these flags from different tasks.
*   **Goal:** Master multi-event synchronization using `xEventGroupWaitBits`.

### Task 10: Performance Profiling
Enable `configGENERATE_RUN_TIME_STATS` in `FreeRTOSConfig.h`. Implement a task that prints a table to the UART showing every task's name, state, priority, and the percentage of total CPU time it has consumed.    
*   **Goal:** Learn how to profile and optimize a real-time system.