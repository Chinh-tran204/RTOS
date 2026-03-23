# STM32 FreeRTOS Template Project (Blue Pill - STM32F103C8)

## Project Overview
This project provides a professional-grade starting point for embedded development on the **STM32F103C8 (Blue Pill)** using **FreeRTOS** and the **STM32 HAL (Hardware Abstraction Layer)**. It is structured to maintain a clean separation between low-level hardware initialization and high-level application logic.

### Key Technologies
*   **MCU:** STM32F103C8 (ARM Cortex-M3)
*   **RTOS:** FreeRTOS (Real-Time Operating System)
*   **Build System:** CMake (Modern, modular build approach)
*   **Compiler:** ARM GCC (`arm-none-eabi-gcc`)
*   **Library:** STM32 HAL (Hardware Abstraction Layer)

---

## Directory Structure
*   `App/`: Contains the core application logic.
    *   `Inc/`: Application-level header files.
    *   `Src/app_main.c`: Entry point for application tasks and task creation.
*   `Core/`: Contains MCU-specific initialization and hardware setup.
    *   `Inc/`: System-level header files (e.g., `main.h`).
    *   `Src/main.c`: Hardware entry point (`main`), clock configuration, and HAL initialization.
*   `Config/`: Project and RTOS configuration.
    *   `FreeRTOSConfig.h`: Custom FreeRTOS settings (Tick rate, Heap size, etc.).
*   `cmake/`: Build system configuration files.
    *   `arm-none-eabi.cmake`: Toolchain definition for cross-compilation.
*   `Drivers/`: **(Requirement)** Place STM32 HAL and CMSIS source code here.
*   `Middlewares/`: **(Requirement)** Place FreeRTOS source code here (under `Third_Party/FreeRTOS`).

---

## Building and Running

### Prerequisites
1.  **ARM GCC Toolchain:** `arm-none-eabi-gcc` must be in your system PATH.
2.  **CMake:** Version 3.22 or higher.
3.  **Make** or **Ninja** build tool.
4.  **Hardware Drivers:** Ensure `Drivers/` and `Middlewares/` are populated with the necessary source files.
5.  **Linker Script:** Place `STM32F103C8TX_FLASH.ld` in the root directory (or update `CMakeLists.txt` with your script's path).

### Build Commands
To build the project, run the following commands from the project root:

```bash
# 1. Create a build directory
mkdir build && cd build

# 2. Configure the project with the ARM toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake ..

# 3. Build the binary
make -j$(nproc)
```

The build will generate:
*   `stm32f103c8-freertos-template.elf` (for debugging)
*   `stm32f103c8-freertos-template.bin` (for flashing)
*   `stm32f103c8-freertos-template.hex` (for flashing)

### Flashing and Debugging
To flash the firmware using an ST-Link and OpenOCD:

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/stm32f103c8-freertos-template.elf verify reset exit"
```

---

## Development Conventions

### Task Management
*   Add new tasks in `App/Src/app_main.c`.
*   Maintain a clear separation: `main.c` initializes hardware, then calls `app_main()` to spawn tasks and start the scheduler.
*   Use `vTaskDelay(pdMS_TO_TICKS(ms))` for non-blocking delays within tasks.

### Error Handling
*   The `Error_Handler()` function in `main.c` is the centralized failure point. In production, this should be extended to log errors or perform a safe system reset.
*   FreeRTOS hooks for `Stack Overflow` and `Malloc Failed` are implemented in `app_main.c`.

### Configuration
*   Modify `Config/FreeRTOSConfig.h` to tune system performance (e.g., `configTOTAL_HEAP_SIZE`, `configTICK_RATE_HZ`).
