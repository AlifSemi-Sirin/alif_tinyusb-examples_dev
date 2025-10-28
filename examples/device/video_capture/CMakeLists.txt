cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set(TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set baremetal build
set(RTOS noos)
# Set app name
set(PROJECT video_capture)

project(${PROJECT} C CXX ASM)

# Checks this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

if (FORCE_READONLY)
target_compile_definitions(${PROJECT} PRIVATE
  CFG_EXAMPLE_VIDEO_READONLY
)
endif()

# Example source
target_sources(${PROJECT} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

# Example include
target_include_directories(${PROJECT} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Configure compilation flags and libraries for the example without RTOS.
# See the corresponding function in hw/bsp/FAMILY/family.cmake for details.
family_configure_device_example(${PROJECT} noos)

# Generate *.bin and *.hex file
add_custom_target(generate_binaries ALL
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT}> ${PROJECT}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT}> ${PROJECT}.hex
  DEPENDS ${PROJECT}
)
