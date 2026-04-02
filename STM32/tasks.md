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

## Phase 5: Inter-Processor Communication & Advanced Drivers (Harder)

### Task 11: SPI Slave Device Emulation
*   **Description:** Configure the STM32 to act as an SPI slave. Create a task (`vSPISlaveTask`) that listens for specific commands (e.g., "READ_STATUS", "SET_LED") from a simulated SPI master. Upon receiving a command, process it (e.g., return a status byte, toggle an LED) and send a predefined response. This will involve setting up SPI peripheral interrupts and synchronizing with the task.
*   **Goal:** Understand SPI peripheral configuration, interrupt handling, and inter-task communication for external hardware interfaces.

### Task 12: I2C Master with External Sensor (Simulated)
*   **Description:** Implement a generic I2C master driver (e.g., `i2c_master_read`, `i2c_master_write`). Create a task (`vSensorReadTask`) that periodically "reads" data from a simulated I2C sensor (e.g., an accelerometer or temperature sensor) by sending I2C commands and receiving dummy data. Print the "sensor readings" to UART.
*   **Goal:** Develop modular I2C driver code, understand I2C communication protocols, and integrate it into a FreeRTOS task.

### Task 13: Modbus RTU Slave over UART
*   **Description:** Configure one of the UART peripherals (e.g., UART2) to operate as a Modbus RTU slave. Implement a task (`vModbusSlaveTask`) that listens for Modbus queries (e.g., Function Code 0x03: Read Holding Registers). For received queries, parse the request, provide simulated register values, and formulate a correct Modbus RTU response.
*   **Goal:** Learn serial communication protocols, data parsing, and implementing industry-standard communication stacks within FreeRTOS.

## Phase 6: Power Management & System Robustness (Expert)

### Task 14: Low Power Mode Integration with Event Wakeup
*   **Description:** Design the system to enter a low-power mode (e.g., STOP mode) when no tasks require immediate CPU attention for a configurable duration. Implement a mechanism (e.g., an external interrupt from a button press, or incoming UART data) to wake the MCU from low-power mode, allowing tasks to resume execution.
*   **Goal:** Optimize power consumption, understand MCU low-power states, and implement interrupt-driven wake-up strategies.

### Task 15: Firmware Update Data Reception (Over UART)
*   **Description:** Create a task (`vFirmwareUpdateTask`) that can receive blocks of simulated firmware data over UART. This task should verify checksums of the received blocks and write them to a designated, non-volatile memory region (e.g., a reserved section of internal Flash memory). Focus on the reception, verification, and storage aspects, not the full bootloader functionality.
*   **Goal:** Learn about non-volatile memory programming, data integrity checks, and handling sequential data streams within FreeRTOS.

### Task 16: Fault Injection & Recovery
*   **Description:** In a dedicated test task (`vFaultInjectionTask`), purposefully cause a runtime error (e.g., a null pointer dereference, division by zero). Implement a robust `HardFault_Handler` that captures fault information (e.g., program counter, stack pointer), logs it to UART, and then attempts a graceful system reset or recovery (e.g., restarting only the affected task if possible, otherwise resetting the entire system).
*   **Goal:** Understand exception handling, debugging critical system failures, and implementing robust error recovery mechanisms in embedded systems.

## Phase 7: Network & Advanced RTOS Features (Mastery)

### Task 17: Embedded Command Line Interface (CLI) over UART
*   **Description:** Develop a simple command-line interface (CLI) over UART. A dedicated `vCLITask` reads user input, parses commands (e.g., "status", "setled <state>", "readsensor"), and dispatches actions to other tasks using appropriate RTOS primitives (e.g., queues, task notifications).
*   **Goal:** Implement interactive system control, input parsing, and sophisticated inter-task command handling.

### Task 18: Direct-to-Task Notification Refactor
*   **Description:** Revisit one of the earlier tasks that uses binary semaphores (e.g., the EXTI handler from Task 4) or queues. Refactor it to use FreeRTOS Direct-to-Task Notifications instead. Compare the code size, RAM usage, and performance implications of this change.
*   **Goal:** Master the use of Direct-to-Task Notifications, understanding their efficiency benefits for specific synchronization patterns.

### Task 19: Message Buffer for High-Throughput Data
*   **Description:** Simulate a high-frequency data source (e.g., an ADC ISR generating samples at 10kHz). Use a FreeRTOS Message Buffer to efficiently transfer these "samples" from the ISR context (or a very high-priority task) to a lower-priority `vDataProcessingTask` for analysis. Demonstrate how message buffers handle bursts of data more effectively than traditional queues for raw byte streams.
*   **Goal:** Optimize data transfer for high-throughput scenarios, understand the characteristics and advantages of FreeRTOS Message Buffers.

## Phase 8: Project Management & Best Practices (Lead Engineer)

### Task 20: Modularization & Abstraction of a Peripheral Driver
*   **Description:** Select a peripheral (e.g., UART, I2C, SPI) or one of the drivers you developed in previous tasks (e.g., I2C master driver from Task 12). Refactor its implementation into a fully modular and abstract component. This involves:
    *   Creating a dedicated header file (`.h`) that defines a clear, user-friendly API.
    *   Separating the implementation details into a corresponding source file (`.c`).
    *   Ensuring minimal dependencies on global variables.
    *   Adding basic error handling and return codes.
    *   Documenting the API.
*   **Goal:** Learn best practices for embedded software design, creating reusable and maintainable peripheral drivers, and contributing to a professional codebase.