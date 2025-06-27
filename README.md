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

## CSolution Project Build (Bare-metal / FreeRTOS)

Make sure that you have setup and run the example projects from the [VSCode Getting Started Template](https://github.com/alifsemi/alif_vscode-template) before working on this project.

## CMake Build (Bare-metal / FreeRTOS)

### Requirements

- `arm-none-eabi-gcc` toolchain
- `cmake` (>= 3.20)
- GNU `make`

### Pull additional libraries
`python lib/tinyusb/tools/get_deps.py alif`

### Build example: CDC + MSC

```bash
cd examples/device/cdc_msc
cmake -B build -DBOARD=alif_e7_dk -DCORE_M55_HP=ON
cmake --build build
```

## Zephyr Build

### Requirements

- [Zephyr SDK 0.16.9] (https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.16.9)
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
west build -b alif_e7_dk -- -DCORE_M55_HP=ON
```

## References

- [TinyUSB Documentation](https://docs.tinyusb.org)
- [Zephyr Documentation](https://docs.zephyrproject.org)
