# ioLibrary_Driver
add_library(IOLIBRARY_FILES STATIC)

target_sources(IOLIBRARY_FILES PUBLIC
    ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
    ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c
)

target_include_directories(IOLIBRARY_FILES PUBLIC
    ${WIZNET_DIR}/Ethernet
    ${PORT_DIR}/ioLibrary_Driver/inc
    ${PORT_DIR}
)

target_link_libraries(IOLIBRARY_FILES PRIVATE
    cmsis_core
    pico_stdlib
    hardware_spi
    hardware_dma
    hardware_clocks
    FREERTOS_FILES
)

# DEVICE COMMON for EaaS
add_library(DEVICE_COMMON_FILES STATIC)

target_sources(DEVICE_COMMON_FILES PUBLIC
    ${PORT_DIR}/device_common/device_common.c
    ${PORT_DIR}/device_common/system_common.c
    ${PORT_DIR}/device_common/logger.c
)

target_include_directories(DEVICE_COMMON_FILES PUBLIC
    ${WIZNET_DIR}/Ethernet
    ${PORT_DIR}/device_common
    ${PORT_DIR}
    #${PICO_SDK_DIR}/src/rp2_common/stub/CMSIS/Device/RaspberryPi/RP2040/Include
)

target_link_libraries(DEVICE_COMMON_FILES PUBLIC
    cmsis_core
    pico_stdio
    pico_stdlib
    pico_stdlib_headers
    hardware_structs
    hardware_resets
    FREERTOS_FILES
)

# Flash Interface for EaaS
add_library(FLASH_INTERFACE_FILES STATIC)

target_sources(FLASH_INTERFACE_FILES PUBLIC
    ${PORT_DIR}/flash_interface/flash_control.c
)

target_include_directories(FLASH_INTERFACE_FILES PUBLIC
    ${WIZNET_DIR}/Ethernet    
    #${PORT_DIR}/device_common
)

target_link_libraries(FLASH_INTERFACE_FILES PRIVATE
    pico_stdlib
    DEVICE_COMMON_FILES
    hardware_flash
)

# timer
add_library(TIMER_FILES STATIC)

target_sources(TIMER_FILES PUBLIC
    ${PORT_DIR}/timer/timer.c
)

target_include_directories(TIMER_FILES PUBLIC
    ${PORT_DIR}/timer
)

target_link_libraries(TIMER_FILES PRIVATE
    pico_stdlib      
)

# Network Interface for EaaS
add_library(NETWORK_INTERFACE_FILES STATIC)

target_sources(NETWORK_INTERFACE_FILES PUBLIC
    ${PORT_DIR}/network_interface/ethernet.c
    ${PORT_DIR}/network_interface/http_interface.c
    ${PORT_DIR}/network_interface/mqtt_interface.c
    ${PORT_DIR}/network_interface/tls_socket_interface.c
)

target_include_directories(NETWORK_INTERFACE_FILES PUBLIC
    ${WIZNET_DIR}/Ethernet
    ${WIZNET_DIR}/Internet/DHCP
    ${WIZNET_DIR}/Internet/DNS
    ${PORT_DIR}/ioLibrary_Driver/inc
    ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc
    ${PORT_DIR}/timer
    ${PORT_DIR}/device_common
    ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc
    ${MBEDTLS_DIR}/include
)

target_link_libraries(NETWORK_INTERFACE_FILES PRIVATE
    pico_stdlib
    hardware_spi
    hardware_dma
    DEVICE_COMMON_FILES
    ETHERNET_FILES
    DHCP_FILES
    DNS_FILES
    IOLIBRARY_FILES
    AWS_SDK_FILES
    TIMER_FILES
    mbedcrypto
    mbedx509
    mbedtls
)

# aws-iot-device-sdk-embedded-C
add_library(AWS_SDK_PORT_FILES STATIC)

target_sources(AWS_SDK_PORT_FILES PUBLIC
)

target_include_directories(AWS_SDK_PORT_FILES PUBLIC
    ${MBEDTLS_DIR}/include
    ${TINY_CBOR_DIR}/src
)

target_link_libraries(AWS_SDK_PORT_FILES PUBLIC
    pico_stdlib
    ETHERNET_FILES
    NETWORK_INTERFACE_FILES
    DHCP_FILES
    DNS_FILES
    AWS_SDK_FILES
    mbedcrypto
    mbedx509
    mbedtls
    tinycbor
)