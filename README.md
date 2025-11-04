# TinyUSB Examples for AlifSemi Devices 

This repository provides examples and support for using [TinyUSB](https://github.com/hathach/tinyusb) with [Alif Semiconductor](https://www.alifsemi.com) Ensemble platform. It supports both CSolution (CMSIS) and CMake-based builds (for bare-metal and FreeRTOS). Zephyr RTOS integration is available as well.

Contains following examples:
* cdc_msc
* cdc_msc_freertos
* cdc_msc_zephyr
* hid_composite
* hid_composite_freertos
* hid_composite_zephyr
* hid_boot_interface
* hid_multiple_interface
* msc_dual_lun
* net_lwip_webserver
* net_lwip_webserver_freertos
* net_lwip_webserver_zephyr
* audio_test
* audio_test_freertos
* audio_test_zephyr
* video_capture
* video_capture_freertos
* video_capture_zephyr

## CSolution Project Build (Bare-metal / FreeRTOS)

Please make sure you have setup your VSCode and other tools and environment based on the [VSCode Getting Started Template](https://github.com/alifsemi/alif_vscode-template) before working on this project.

After setting up the environment you can select File&rarr;Open Folder from VSCode and press **F1** and start choosing from the preset build tasks.
1. **F1** &rarr; Tasks:Run Task &rarr; First time pack installation
2. **F1** &rarr; Tasks:Run Task &rarr; cmsis-csolution.build:Build (Better to do this from the CMSIS Extension Build (hammer icon))
3. **F1** &rarr; Tasks:Run Task &rarr; Program with Security Toolkit


## CMake Build (Bare-metal / FreeRTOS)

### Requirements

- `arm-none-eabi-gcc` toolchain
- `cmake` (>= 3.20)
- GNU `make`

### Pull additional libraries
`python lib/tinyusb/tools/get_deps.py alif`

### Build example: CDC + MSC

Currently build is supported for 2 cores: **rtss_hp** and **rtss_he**.

```bash
cd examples/device/cdc_msc
cmake -B build -DBOARD=alif_e7_dk -DBOARD_QUALIFIERS=/ae722f80f55d5xx/rtss_hp
cmake --build build
```

## Zephyr Build

### Requirements

- [Zephyr SDK 0.16.9](https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.16.9)
- `west` tool
- Python 3 with Zephyr dependencies installed

### Install Zephyr SDK
```cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.16.9/zephyr-sdk-0.16.9_linux-x86_64.tar.xz
tar xvf zephyr-sdk-0.16.9_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.9
./setup.sh
```

### Build Example (CDC + MSC for Zephyr)

Currently build is supported for 2 cores: **rtss_hp** and **rtss_he**.

```bash
python3 -m venv .venv
source ./.venv/bin/activate

pip install -U west
cd examples/
west init -l .
cd ../
west update
pip install -r zephyr/scripts/requirements.txt

cd examples/device/cdc_msc_zephyr/
west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp
```

## References

- [TinyUSB Documentation](https://docs.tinyusb.org)
- [Zephyr Documentation](https://docs.zephyrproject.org)
