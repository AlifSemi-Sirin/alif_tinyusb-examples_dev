cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set(TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set baremetal build
set(RTOS noos)

add_subdirectory(${TINYUSB_LOCATION}/examples/device/hid_composite hid_composite)

project(hid_composite_wrapper)

# Generate *.bin and *.hex file
add_custom_target(generate_binaries ALL
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:hid_composite> hid_composite.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:hid_composite> hid_composite.hex
  DEPENDS hid_composite
)
