# CMake minimum required version
cmake_minimum_required(VERSION 3.12)

# Find git
find_package(Git)

if(NOT Git_FOUND)
	message(FATAL_ERROR "Could not find 'git' tool for RP2040-WIZNET-EAAS patching")
endif()

message("RP2040-WIZNET-EAAS patch utils found")

set(RP2040_WIZNET_EAAS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PICO_EXTRAS_SRC_DIR "${RP2040_WIZNET_EAAS_SRC_DIR}/libraries/pico-extras")
set(PICO_SDK_SRC_DIR "${RP2040_WIZNET_EAAS_SRC_DIR}/libraries/pico-sdk")
set(PICO_SDK_TINYUSB_SRC_DIR "${RP2040_WIZNET_EAAS_SRC_DIR}/libraries/lib/tinyusb")