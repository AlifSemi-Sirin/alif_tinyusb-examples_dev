cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set(TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set baremetal build
set(RTOS noos)
# Set app name
set(PROJECT audio_test)

# Add native TinyUSB example
add_subdirectory(${TINYUSB_LOCATION}/examples/device/${PROJECT} ${PROJECT})
# Set project name
project(${PROJECT}_wrapper)

# Generate *.bin and *.hex file
add_custom_target(generate_binaries ALL
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT}> ${PROJECT}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT}> ${PROJECT}.hex
  DEPENDS ${PROJECT}
)
