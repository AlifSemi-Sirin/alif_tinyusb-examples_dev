cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set(TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set FreeRTOS build
set(RTOS freertos)

add_subdirectory(${TINYUSB_LOCATION}/examples/device/cdc_msc_freertos cdc_msc_freertos)

project(cdc_msc_freertos_wrapper)

# Generate *.bin and *.hex file
add_custom_target(generate_binaries ALL
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:cdc_msc_freertos> cdc_msc_freertos.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:cdc_msc_freertos> cdc_msc_freertos.hex
  DEPENDS cdc_msc_freertos
)
