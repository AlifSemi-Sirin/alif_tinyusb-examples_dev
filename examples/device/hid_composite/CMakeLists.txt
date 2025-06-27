cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set (TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set baremetal build
set(RTOS noos)
# Include BSP
include(${TINYUSB_LOCATION}/hw/bsp/family_support.cmake)

# Gets PROJECT name for the example (e.g. <BOARD>-<DIR_NAME>)
family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})
project(${PROJECT} C CXX ASM)

# Checks this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

# Set executable 
set(EXE_NAME ${PROJECT})
add_executable(${EXE_NAME})

# Source from native tinyusb example
target_sources(${EXE_NAME} PRIVATE
  ${TINYUSB_LOCATION}/examples/device/hid_composite/src/main.c
  ${TINYUSB_LOCATION}/examples/device/hid_composite/src/usb_descriptors.c
  )

# Example include
target_include_directories(${EXE_NAME} PUBLIC
  ${TINYUSB_LOCATION}/examples/device/hid_composite/src
  )

# Configure compilation flags and libraries for the example without RTOS.
# See the corresponding function in hw/bsp/FAMILY/family.cmake for details.
family_configure_device_example(${EXE_NAME} ${RTOS})

# Generate *.bin file
add_custom_command(TARGET ${EXE_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXE_NAME}> ${EXE_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${EXE_NAME}> ${EXE_NAME}.hex
  COMMENT "Generating ${EXE_NAME}.bin and ${EXE_NAME}.hex"
)
