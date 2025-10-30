cmake_minimum_required(VERSION 3.20)

set (TINYUSB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/tinyusb/src)

# Add tinyusb to a existing target, DCD and HCD drivers are not included
function(tinyusb_target_add TARGET)
  target_sources(${TARGET} PRIVATE
    # common
    ${TINYUSB_SRC}/tusb.c
    ${TINYUSB_SRC}/common/tusb_fifo.c
    # device
    ${TINYUSB_SRC}/device/usbd.c
    ${TINYUSB_SRC}/device/usbd_control.c
    # ${TINYUSB_SRC}/class/audio/audio_device.c
    ${TINYUSB_SRC}/class/cdc/cdc_device.c
    ${TINYUSB_SRC}/class/dfu/dfu_device.c
    ${TINYUSB_SRC}/class/dfu/dfu_rt_device.c
    ${TINYUSB_SRC}/class/hid/hid_device.c
    ${TINYUSB_SRC}/class/midi/midi_device.c
    ${TINYUSB_SRC}/class/msc/msc_device.c
    ${TINYUSB_SRC}/class/mtp/mtp_device.c
    ${TINYUSB_SRC}/class/net/ecm_rndis_device.c
    ${TINYUSB_SRC}/class/net/ncm_device.c
    ${TINYUSB_SRC}/class/usbtmc/usbtmc_device.c
    ${TINYUSB_SRC}/class/vendor/vendor_device.c
    ${TINYUSB_SRC}/class/video/video_device.c
    # host
    ${TINYUSB_SRC}/host/usbh.c
    ${TINYUSB_SRC}/host/hub.c
    ${TINYUSB_SRC}/class/cdc/cdc_host.c
    ${TINYUSB_SRC}/class/hid/hid_host.c
    ${TINYUSB_SRC}/class/midi/midi_host.c
    ${TINYUSB_SRC}/class/msc/msc_host.c
    ${TINYUSB_SRC}/class/vendor/vendor_host.c
    # typec
    ${TINYUSB_SRC}/typec/usbc.c
    )
  target_include_directories(${TARGET} PUBLIC
    ${TINYUSB_SRC}
    # TODO for net driver, should be removed/changed
    ${TINYUSB_SRC}/../lib/networking
    )
endfunction()
