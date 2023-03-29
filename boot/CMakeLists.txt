set(TARGET_NAME rp2040_wiznet_eaas_boot)

add_executable(${TARGET_NAME}
    ${BOOT_DIR}/main.c
    ${BOOT_DIR}/test_menu.c

    ${PORT_DIR}/device_common/device_common.c
    ${PORT_DIR}/device_common/system_common_for_nonos.c
    ${PORT_DIR}/device_common/logger.c

    ${PORT_DIR}/flash_interface/flash_control.c
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${PORT_DIR}
    ${PORT_DIR}/flash_interface
    ${PORT_DIR}/device_common
    ${WIZNET_DIR}/Ethernet/
)

target_link_libraries(${TARGET_NAME} PRIVATE
    cmsis_core
    pico_stdlib
    pico_stdlib_headers
    hardware_structs
    hardware_spi
    hardware_clocks
    hardware_dma
    hardware_flash
    hardware_resets
)

pico_enable_stdio_usb(${TARGET_NAME} 0)
pico_enable_stdio_uart(${TARGET_NAME} 1)
pico_add_extra_outputs(${TARGET_NAME})


set(TARGET_NAME_LINKER ${TARGET_NAME}_linker)
add_executable(${TARGET_NAME_LINKER}
    ${BOOT_DIR}/main.c
    ${BOOT_DIR}/test_menu.c

    ${PORT_DIR}/device_common/device_common.c
    ${PORT_DIR}/device_common/system_common_for_nonos.c
    ${PORT_DIR}/device_common/logger.c

    ${PORT_DIR}/flash_interface/flash_control.c
)

target_include_directories(${TARGET_NAME_LINKER} PUBLIC
    ${PORT_DIR}
    ${PORT_DIR}/flash_interface
    ${PORT_DIR}/device_common
    ${WIZNET_DIR}/Ethernet/
)

target_link_libraries(${TARGET_NAME_LINKER} PRIVATE
    cmsis_core
    pico_stdlib
    pico_stdlib_headers
    hardware_structs
    hardware_spi
    hardware_clocks
    hardware_dma
    hardware_flash
    hardware_resets
)

function(target_cl_options option)
    target_compile_options(${TARGET_NAME_LINKER} PRIVATE ${option})
    target_link_options(${TARGET_NAME_LINKER} PRIVATE ${option})
endfunction()

target_cl_options("-ffunction-sections")
target_cl_options("-fdata-sections")
target_link_options(${TARGET_NAME_LINKER} PRIVATE "LINKER:--gc-sections")

pico_enable_stdio_usb(${TARGET_NAME_LINKER} 0)
pico_enable_stdio_uart(${TARGET_NAME_LINKER} 1)

pico_add_extra_outputs(${TARGET_NAME_LINKER})
pico_set_binary_type(${TARGET_NAME_LINKER} copy_to_ram)
set_target_properties(${TARGET_NAME_LINKER} PROPERTIES COMPILE_FLAGS "-Wall")

pico_set_linker_script(${TARGET_NAME_LINKER} ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.ld)


