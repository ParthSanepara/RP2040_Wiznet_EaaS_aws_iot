# Ethernet
add_library(ETHERNET_FILES STATIC)

target_sources(ETHERNET_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet/socket.c
        ${WIZNET_DIR}/Ethernet/wizchip_conf.c
        )

message(STATUS "Libraries WIZNET_CHIP = ${WIZNET_CHIP}")

target_include_directories(ETHERNET_FILES INTERFACE
        ${WIZNET_DIR}/Ethernet
        ${WIZNET_DIR}/Ethernet/W5100S
        )

target_link_libraries(ETHERNET_FILES PUBLIC
    W5100S_FILES
)

# W5100S
add_library(W5100S_FILES STATIC)

target_sources(W5100S_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet/W5100S/w5100s.c
        )

target_include_directories(W5100S_FILES INTERFACE
        ${WIZNET_DIR}/Ethernet
        ${WIZNET_DIR}/Ethernet/W5100S
        )

target_link_libraries(W5100S_FILES PRIVATE
    cmsis_core
    pico_stdlib
    hardware_gpio
    hardware_spi
    ETHERNET_FILES
)        

# FreeRTOS
add_library(FREERTOS_FILES STATIC)

target_sources(FREERTOS_FILES PUBLIC
        ${FREERTOS_DIR}/event_groups.c
        ${FREERTOS_DIR}/list.c
        ${FREERTOS_DIR}/queue.c
        ${FREERTOS_DIR}/stream_buffer.c
        ${FREERTOS_DIR}/tasks.c
        ${FREERTOS_DIR}/timers.c
        ${FREERTOS_DIR}/portable/GCC/ARM_CM0/port.c  		
        ${FREERTOS_DIR}/portable/MemMang/heap_4.c
        )

target_include_directories(FREERTOS_FILES PUBLIC
        ${PORT_DIR}/FreeRTOS-Kernel/inc
        ${FREERTOS_DIR}/include
        ${FREERTOS_DIR}/portable/GCC/ARM_CM0
        )


