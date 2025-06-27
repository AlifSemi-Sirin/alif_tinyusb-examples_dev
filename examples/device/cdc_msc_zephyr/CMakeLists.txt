cmake_minimum_required(VERSION 3.20)

# Set TinyUSB library location
set (TINYUSB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb)
# Set Zephyr location
set(Zephyr_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../zephyr/share/zephyr-package/cmake)
# Set Zephyr build
set(RTOS zephyr)

# Include BSP
include(${TINYUSB_LOCATION}/hw/bsp/family_support.cmake)

# Get PROJECT name for the example (e.g. <BOARD>-<DIR_NAME>)
family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})
project(${PROJECT} C CXX ASM)

# Check this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

# Set executable 
set(EXE_NAME app)

# Example source
target_sources(${EXE_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/msc_disk.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
  )

# Example include
target_include_directories(${EXE_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

# Configure compilation flags and libraries for Zephyr.
# See the corresponding function in hw/bsp/FAMILY/family.cmake for details.
family_configure_device_example(${EXE_NAME} ${RTOS})
