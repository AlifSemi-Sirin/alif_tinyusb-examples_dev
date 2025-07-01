cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set(TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set baremetal build
set(RTOS noos)

add_subdirectory(${TINYUSB_LOCATION}/examples/device/cdc_msc cdc_msc)

project(cdc_msc_wrapper)

# Generate *.bin and *.hex file
add_custom_target(generate_binaries ALL
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:cdc_msc> cdc_msc.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:cdc_msc> cdc_msc.hex
  DEPENDS cdc_msc
)
