cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/../pico/pico-sdk)
set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/../FreeRTOS-Kernel)

# Change your executable name to something creative!
set(NAME inky-digital-frame) # <-- Name your project/executable here!
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your source files
add_executable(${NAME}
    src/main.cpp
    src/mongoose/mongoose.c
    src/web_server/web_server.cpp
    src/packed_fs.c
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

# add_subdirectory(web_server)

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
pico_enable_stdio_uart(${NAME} 0) # set 1 for pico debugprob

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