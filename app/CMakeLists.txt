set(TARGET_NAME rp2040_wiznet_eaas)

add_executable(${TARGET_NAME}
    ${APP_DIR}/main.c

    ${PORT_DIR}/device_common/device_common.c
    ${PORT_DIR}/device_common/system_common.c
    ${PORT_DIR}/device_common/logger.c
    ${PORT_DIR}/device_common/simple_cert_checker.c
    ${PORT_DIR}/device_common/led_control.c

    ${PORT_DIR}/flash_interface/flash_control.c

    ${PORT_DIR}/timer/timer.c

    ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
    ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c

    ${PORT_DIR}/network_interface/ethernet.c
    ${PORT_DIR}/network_interface/http_interface.c
    ${PORT_DIR}/network_interface/mqtt_interface.c
    ${PORT_DIR}/network_interface/tls_socket_interface.c

    ${APP_DIR}/aws_iot/aws_iot_cert.c
    ${APP_DIR}/aws_iot/fleet_provisioning_handler.c
    ${APP_DIR}/aws_iot/iot_jobs_handler.c
    ${APP_DIR}/aws_iot/iot_jobs_job_list.c
    ${APP_DIR}/aws_iot/iot_jobs_job_ota_handler.c

    ${APP_DIR}/tasks/aws_iot_jobs_task.c
    ${APP_DIR}/tasks/idle_task.c
    ${APP_DIR}/tasks/main_app_task.c
)

target_include_directories(${TARGET_NAME} PUBLIC 
    ${APP_DIR}
    ${APP_DIR}/aws_iot
    ${APP_DIR}/tasks

    ${PORT_DIR}
    ${PORT_DIR}/timer
    ${PORT_DIR}/FreeRTOS-Kernel/inc
    ${PORT_DIR}/ioLibrary_Driver/inc
    ${PORT_DIR}/network_interface
    ${PORT_DIR}/device_common
    ${PORT_DIR}/flash_interface
    ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc

    ${WIZNET_DIR}/Ethernet/
    ${WIZNET_DIR}/Internet/DHCP
    ${WIZNET_DIR}/Internet/DNS

    ${MBEDTLS_DIR}/include
)

target_link_libraries(${TARGET_NAME} PRIVATE
    pico_stdlib
    hardware_clocks
    hardware_flash
    hardware_spi
    ETHERNET_FILES
    DHCP_FILES
    DNS_FILES
    FREERTOS_FILES
    AWS_SDK_FILES
    tinycbor
    mbedcrypto
    mbedx509
    mbedtls
)

pico_enable_stdio_usb(${TARGET_NAME} 0)
pico_enable_stdio_uart(${TARGET_NAME} 1)

pico_add_extra_outputs(${TARGET_NAME})


set(TARGET_NAME_LINKER ${TARGET_NAME}_linker)
add_executable(${TARGET_NAME_LINKER}
    ${APP_DIR}/main.c

    ${PORT_DIR}/device_common/device_common.c
    ${PORT_DIR}/device_common/system_common.c
    ${PORT_DIR}/device_common/logger.c
    ${PORT_DIR}/device_common/simple_cert_checker.c
    ${PORT_DIR}/device_common/led_control.c

    ${PORT_DIR}/flash_interface/flash_control.c

    ${PORT_DIR}/timer/timer.c

    ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
    ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c

    ${PORT_DIR}/network_interface/ethernet.c
    ${PORT_DIR}/network_interface/http_interface.c
    ${PORT_DIR}/network_interface/mqtt_interface.c
    ${PORT_DIR}/network_interface/tls_socket_interface.c

    ${APP_DIR}/aws_iot/aws_iot_cert.c
    ${APP_DIR}/aws_iot/fleet_provisioning_handler.c
    ${APP_DIR}/aws_iot/iot_jobs_handler.c
    ${APP_DIR}/aws_iot/iot_jobs_job_list.c
    ${APP_DIR}/aws_iot/iot_jobs_job_ota_handler.c

    ${APP_DIR}/tasks/aws_iot_jobs_task.c
    ${APP_DIR}/tasks/idle_task.c
    ${APP_DIR}/tasks/main_app_task.c
)

target_include_directories(${TARGET_NAME_LINKER} PUBLIC 
    ${APP_DIR}
    ${APP_DIR}/aws_iot
    ${APP_DIR}/tasks

    ${PORT_DIR}
    ${PORT_DIR}/timer
    ${PORT_DIR}/FreeRTOS-Kernel/inc
    ${PORT_DIR}/ioLibrary_Driver/inc
    ${PORT_DIR}/network_interface
    ${PORT_DIR}/device_common
    ${PORT_DIR}/flash_interface
    ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc

    ${WIZNET_DIR}/Ethernet/
    ${WIZNET_DIR}/Internet/DHCP
    ${WIZNET_DIR}/Internet/DNS


    ${MBEDTLS_DIR}/include
)

target_link_libraries(${TARGET_NAME_LINKER} PRIVATE
    pico_stdlib
    hardware_clocks
    hardware_flash
    hardware_spi
    ETHERNET_FILES
    DHCP_FILES
    DNS_FILES
    FREERTOS_FILES
    AWS_SDK_FILES
    tinycbor
    mbedcrypto
    mbedx509
    mbedtls
)

pico_enable_stdio_usb(${TARGET_NAME_LINKER} 0)
pico_enable_stdio_uart(${TARGET_NAME_LINKER} 1)

pico_add_extra_outputs(${TARGET_NAME_LINKER})
pico_set_linker_script(${TARGET_NAME_LINKER} ${CMAKE_CURRENT_SOURCE_DIR}/app_main.ld)
