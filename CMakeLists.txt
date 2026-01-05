cmake_minimum_required(VERSION 3.13)

# Set up the Pico SDK
set(PICO_BOARD pico2_W)
set(PICO_PLATFORM rp2350)

# Path to FreeRTOS Kernel
set(FREERTOS_KERNEL_PATH "C:/Users/zhasanr/Desktop/Hand Controlled Car/FreeRTOS-Kernel")

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(freertos_rp2350 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Your executable
add_executable(your_app
    src/main.c
)

# Include directories
target_include_directories(your_app PRIVATE
    ${FREERTOS_KERNEL_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure
)

# Link with Pico SDK libraries
target_link_libraries(your_app
    pico_stdlib
    FreeRTOS-Kernel-Heap4
    hardware_gpio
)

# Enable USB stdio if needed
pico_enable_stdio_usb(your_app 1)
pico_enable_stdio_uart(your_app 0)

# Extra output files (UF2, HEX, BIN)
pico_add_extra_outputs(your_app)
