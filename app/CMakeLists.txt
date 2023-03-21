set(TARGET_NAME rp2040_wiznet_eaas)

add_executable(${TARGET_NAME}
    ${APP_DIR}/main.c

    ${APP_DIR}/aws_iot/aws_iot_cert.c
    ${APP_DIR}/aws_iot/fleet_provisioning_handler.c
    ${APP_DIR}/aws_iot/iot_jobs_handler.c
    ${APP_DIR}/aws_iot/iot_jobs_job_list.c
    ${APP_DIR}/aws_iot/iot_jobs_job_ota_handler.c

    ${APP_DIR}/tasks/fleet_provisioning_task.c
    ${APP_DIR}/tasks/aws_iot_jobs_task.c
    ${APP_DIR}/tasks/idle_task.c
    ${APP_DIR}/tasks/main_app_task.c
)

target_include_directories(${TARGET_NAME} PUBLIC 
    ${APP_DIR}
    ${APP_DIR}/aws_iot
    ${APP_DIR}/tasks
    ${PORT_DIR}/network_interface
    ${PORT_DIR}/device_common
    ${PORT_DIR}/flash_interface
    ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc
)

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_clocks
        hardware_flash
        FREERTOS_FILES
        DEVICE_COMMON_FILES
        FLASH_INTERFACE_FILES
        NETWORK_INTERFACE_FILES
        AWS_SDK_FILES
        tinycbor
)

pico_enable_stdio_usb(${TARGET_NAME} 0)
pico_enable_stdio_uart(${TARGET_NAME} 1)

pico_add_extra_outputs(${TARGET_NAME})


# set(TARGET_NAME_LINKER ${TARGET_NAME}_linker)
# add_executable(${TARGET_NAME_LINKER}
#         ${APPS_DIR}/main.c

#         ${APPS_DIR}/system.c

#         ${APPS_DIR}/common_utils/logger.c
#         ${APPS_DIR}/common_utils/common.c
        
#         ${APPS_DIR}/flash_interface/flash_control.c
        
#         ${APPS_DIR}/network_interface/ethernet.c
#         ${APPS_DIR}/network_interface/tls_socket_interface.c
#         ${APPS_DIR}/network_interface/mqtt_interface.c
#         ${APPS_DIR}/network_interface/http_interface.c
        
#         ${APPS_DIR}/aws_iot/aws_iot_cert.c
#         ${APPS_DIR}/aws_iot/fleet_provisioning_handler.c
#         ${APPS_DIR}/aws_iot/iot_jobs_handler.c
#         ${APPS_DIR}/aws_iot/iot_jobs_job_list.c
#         ${APPS_DIR}/aws_iot/iot_jobs_job_ota_handler.c
        
#         ${APPS_DIR}/tasks/fleet_provisioning_task.c
#         ${APPS_DIR}/tasks/aws_iot_jobs_task.c
#         ${APPS_DIR}/tasks/idle_task.c
#         ${APPS_DIR}/tasks/main_app_task.c
#         )

# target_include_directories(${TARGET_NAME_LINKER} PUBLIC 
#         ${APPS_DIR}
#         ${APPS_DIR}/common_utils
#         ${APPS_DIR}/flash_interface
#         ${APPS_DIR}/network_interface
#         ${APPS_DIR}/aws_iot
#         ${APPS_DIR}/tasks
#         ${PORT_DIR}
# )

# target_link_libraries(${TARGET_NAME_LINKER} PRIVATE
#         pico_stdlib
#         hardware_spi
#         hardware_dma
#         hardware_flash
#         FREERTOS_FILES
#         ETHERNET_FILES
#         IOLIBRARY_FILES
#         MQTT_FILES
#         DHCP_FILES
#         DNS_FILES
#         AWS_SDK_PORT_FILES
#         tinycbor
#         TIMER_FILES
#         mbedcrypto
#         mbedx509
#         mbedtls
#         )

# pico_enable_stdio_usb(${TARGET_NAME_LINKER} 0)
# pico_enable_stdio_uart(${TARGET_NAME_LINKER} 1)

# pico_add_extra_outputs(${TARGET_NAME_LINKER})
# pico_set_linker_script(${TARGET_NAME_LINKER} ${CMAKE_CURRENT_SOURCE_DIR}/app_main.ld)
