cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == BLOQUE OPCIONAL PARA VS CODE (puedes dejarlo si usas la extensión) ==
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
# ===============================================================

# Importar el SDK de la Pico
include(pico_sdk_import.cmake)

project(AudioTest C CXX ASM)

# Inicializar SDK
pico_sdk_init()

add_executable(AudioTest
    audio_test.c
    i2s_tx.pio
)

# Generar el header de PIO
pico_generate_pio_header(AudioTest ${CMAKE_CURRENT_LIST_DIR}/i2s_tx.pio)

# Nombre y versión
pico_set_program_name(AudioTest "AudioTest")
pico_set_program_version(AudioTest "0.1")

# stdio (opcional)
pico_enable_stdio_uart(AudioTest 0)
pico_enable_stdio_usb(AudioTest 1)

# Librerías necesarias (sin pico-extras)
target_link_libraries(AudioTest
    pico_stdlib
    hardware_pio
    hardware_clocks
    hardware_irq
)

target_include_directories(AudioTest PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(AudioTest)
