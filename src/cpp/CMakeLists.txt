# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
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
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(cpp C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(cpp main.cpp 
    # core/parser.cpp
    # core/parser.h
    # core/expressions.h
    # core/syntax_token.h
    # core/syntax_kind.h
    # core/evaluator.cpp
    # core/evaluator.h
    # core/constants.h
    # core/functions.h
    # core/functions.cpp

    drivers/st7789/st7789.cpp
    drivers/st7789/st7789.h
    drivers/st7789/fonts.h
)

pico_set_program_name(cpp "cpp")
pico_set_program_version(cpp "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(cpp 0)
pico_enable_stdio_usb(cpp 1)

# Add the standard library to the build
target_link_libraries(cpp
        pico_stdlib
        hardware_pwm
        hardware_spi)

# Add the standard include files to the build
target_include_directories(cpp PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(cpp)