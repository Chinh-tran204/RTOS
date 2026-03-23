cmake_minimum_required(VERSION 3.22)

#-------------------------------------------------------------------------------
# Project Configuration
#-------------------------------------------------------------------------------
set(PROJECT_NAME stm32f103c8-freertos-template)
project(${PROJECT_NAME} C CXX ASM)

# Toolchain check
if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "No toolchain file specified. Use: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake ..")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Hardware Definitions
#-------------------------------------------------------------------------------
set(CPU_PARAMETERS 
    -mthumb 
    -mcpu=cortex-m3
)

set(COMMON_FLAGS
    ${CPU_PARAMETERS}
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -ffunction-sections
    -fdata-sections
)

# Debug vs Release flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_FLAGS -g3 -Og -DDEBUG)
else()
    set(BUILD_FLAGS -Os -DNDEBUG)
endif()

add_compile_options(${COMMON_FLAGS} ${BUILD_FLAGS})
add_link_options(${CPU_PARAMETERS} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs)

#-------------------------------------------------------------------------------
# Project Targets
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME}.elf)

# Target-specific defines
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    STM32F103xB
    USE_HAL_DRIVER
)

#-------------------------------------------------------------------------------
# Subdirectories
#-------------------------------------------------------------------------------
# Add subdirectories (Drivers, FreeRTOS, Core, App)
# Note: In a real project, these would contain their own CMakeLists.txt
# For this template, we'll keep it simple in one file or modularize later.

# Include paths
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    Core/Inc
    App/Inc
    Config
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/CMSIS/Include
    Drivers/STM32F1xx_HAL_Driver/Inc
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3
)

# Source files (Professional separation)
file(GLOB_RECURSE CORE_SOURCES 
    "Core/Src/*.c"
    "Core/Src/*.s"
)

file(GLOB_RECURSE APP_SOURCES 
    "App/Src/*.c"
)

# HAL and FreeRTOS sources would go here
# In a real company environment, these are often git submodules or fetched via CMake FetchContent

target_sources(${PROJECT_NAME}.elf PRIVATE
    ${CORE_SOURCES}
    ${APP_SOURCES}
)

# Linker script
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F103C8TX_FLASH.ld)
target_link_options(${PROJECT_NAME}.elf PRIVATE -T${LINKER_SCRIPT})

# Post-build: Generate .bin and .hex
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMENT "Generating binary and hex files"
)
