cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/pico-sdk)
set(PIMORONI_PICO_PATH ${CMAKE_SOURCE_DIR}/pimoroni-pico)
set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel)

set(NAME inky-digital-frame)
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

include(wifi_config.cmake OPTIONAL RESULT_VARIABLE WIFI_CONFIG_PATH)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your source files
add_subdirectory(src/web_server)

add_executable(${NAME}
    src/main.cpp
    src/inky_frame_manager.cpp
    src/mongoose/mongoose.c
    packed_fs.c
    ${SRC_FILES}
)

target_compile_definitions(${NAME} PUBLIC INKY_FRAME_7)


# Include required libraries
# This assumes `pimoroni-pico` is stored alongside your project
include(common/pimoroni_i2c)
include(common/pimoroni_bus)
include(libraries/bitmap_fonts/bitmap_fonts)
include(libraries/hershey_fonts/hershey_fonts)
include(libraries/inky_frame_7/inky_frame_7)

target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${CMAKE_CURRENT_LIST_DIR}/src/mongoose/include
)

if (WIFI_CONFIG_PATH STREQUAL "NOTFOUND")
    message("Missing WIFI Config file")
endif()

if (WIFI_SSID OR WIFI_PASSWORD)
    message("WIFI_SSID set to ${WIFI_SSID}")

    target_compile_definitions(${NAME} PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    )
else()
    message("Missing WIFI_SSID or WIFI_PASSWORD, you need setup wifi in cd card")
    set(WIFI_SSID, "")
    set(WIFI_PASSWORD, "")
endif()


# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    jpegdec
    inky_frame_7
    hardware_pwm
    hardware_spi 
    hardware_i2c 
    fatfs
    sdcard 
    pico_graphics
    FreeRTOS-Kernel-Heap4
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# for pico debugprob
# pico_enable_stdio_usb(${NAME} 0)
# pico_enable_stdio_uart(${NAME} 1)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)

set(TARGET_BINARY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2)