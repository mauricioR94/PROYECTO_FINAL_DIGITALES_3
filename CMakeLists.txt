# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(audio_sd_testo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add FatFS library
add_subdirectory(lib/no-OS-FatFS/FatFs_SPI build)
# Add executable. Default name is the project name, version 0.1

add_executable(audio_sd_testo
    test_audio_SD_DMA.c
    hw_config.c
    sd_manager.c
    i2s_output.c
    audio_player.c
    input_buttons.c      
    lcd.c
    instrument_ui.c      
    mpu6050.c
)
# Generate PIO header
pico_generate_pio_header(audio_sd_testo ${CMAKE_CURRENT_LIST_DIR}/i2s_tx.pio)
pico_set_program_name(audio_sd_testo "audio_sd_testo")
pico_set_program_version(audio_sd_testo "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(audio_sd_testo 0)
pico_enable_stdio_usb(audio_sd_testo 1)

# Add the standard library to the build
target_link_libraries(audio_sd_testo
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_pio
        hardware_irq
        hardware_i2c 
        hardware_clocks
        FatFs_SPI)

# Add the standard include files to the build
target_include_directories(audio_sd_testo PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(audio_sd_testo)

