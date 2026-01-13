cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico CACHE STRING "Board type") 
include(pico_sdk_import.cmake)

project(StarDog C CXX ASM)
pico_sdk_init()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

add_executable(${PROJECT_NAME}
StarDog.c # código principal em C
lib/ssd1306.c # biblioteca para o display OLED
)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(${PROJECT_NAME}
pico_stdlib # biblioteca padrão do Pico
hardware_i2c # suporte ao I2C (display OLED)
hardware_adc # suporte ao ADC (joystick)
hardware_pwm # suporte ao PWM (buzzer)
hardware_pio # suporte ao PIO (matriz WS2812)
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
