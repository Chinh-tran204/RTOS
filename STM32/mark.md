# Task Evaluation: Phase 3 (Tasks 6, 7, 8)

## Overall Project Status: ⚠️ Needs Correction (Does not compile)

### Technical Summary
The implementation shows a good understanding of the advanced RTOS concepts (Static Allocation, Mutexes, and Hook functions). However, there are several syntax errors, naming inconsistencies, and a fundamental logic error in the stack overflow demonstration that will prevent the code from compiling or functioning as intended.

---

## Task 6: Stack Sentinel (Memory Management)
**Status: Fail**

### Feedback:
*   **Logic Error (Stack vs Heap):** In `vOverflowTask`, you called `malloc` (via `arr(1000)`). `malloc` allocates memory on the **Heap**. To trigger a **Stack Overflow**, you must allocate a large array locally within the function (e.g., `uint32_t buffer[256];`).
*   **Symbol Mismatch:** In `app_main()`, you try to create a task using `OverflowTask`, but the function is defined as `vOverflowTask`.
*   **Syntax Error:** Missing semicolon at the end of the `xTaskCreate` call for the overflow task.
*   **Small Stack Size:** You used a stack size of 32. On ARM Cortex-M3, the minimum stack size for a task (to hold the initial context) is usually around 64-70 words. 32 might cause a crash before the task even enters its function.

---

## Task 7: Priority Inversion Demonstration
**Status: Partial Success (Logic is sound, Code has syntax errors)**

### Feedback:
*   **Logic:** The task priorities (Low=1, Medium=2, High=3) and the use of a Mutex correctly set up a scenario to observe Priority Inheritance.
*   **Syntax Errors:**
    *   `xsemaphoreHandle_t` (lowercase 's') is not a valid type. It should be `SemaphoreHandle_t`.
    *   Case sensitivity: You defined `vLog` but called `vlog` (lowercase 'l'). C is case-sensitive.
    *   `vLog` signature: Your `vLog` function takes a `const char*`, but you try to call it like `printf` with format strings (e.g., `vlog("... %s", name)`). This will not work.

---

## Task 8: Static vs Dynamic Allocation
**Status: Pass**

### Feedback:
*   **Correct Implementation:** Good job on `xTaskCreateStatic` for the Heartbeat task.
*   **Memory Hooks:** `vApplicationGetIdleTaskMemory` and `vApplicationGetTimerTaskMemory` are correctly implemented. This is a critical requirement for using static allocation with `configSUPPORT_STATIC_ALLOCATION = 1`.

---

## Critical System Issues (main.c)

*   **Dead Code:** `MX_GPIO_Init()` is defined but **never called** in `main()`. This means your LEDs and Buttons will not be initialized and will not work.
*   **Linker Error:** `xButtonSemaphore` is used in `main.c` (inside the EXTI callback) but is not declared as `extern`. It is currently commented out in `main.c`.
*   **Naming Mismatch:** `MX_UART1_UART_Init()` is called but the function is defined as `MX_USART1_UART_Init()`.

---

## Recommendations for Phase 3
1.  **Fix the Stack Overflow:** Change the `malloc` to a local array.
2.  **Unify Logger:** Fix the `vLog` vs `vlog` naming and ensure it handles strings correctly.
3.  **Task Loops:** Ensure `vButtonHandler` has an infinite `for(;;)` loop. Currently, it runs once and terminates.
4.  **Hardware Init:** Ensure `MX_GPIO_Init()` is called in `main()`.
5.  **Compile often:** Use the `make` command to catch these syntax errors early.

**Current Mark: 4/10** (Logic is there, but implementation is broken).
