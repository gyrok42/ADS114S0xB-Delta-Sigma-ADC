# Configuration Menu Script for ADS114S0XB Driver

## Overview

This script automates the configuration and setup of the `ti-ads114s0xb` ADC driver. It loads necessary kernel modules, compiles and inserts the driver, and sets up an `IIO` (Industrial I/O) interface for data acquisition.

## Features

- Loads required kernel modules for Industrial I/O
- Compiles and loads the ADS114S0XB driver
- Configures a sysfs trigger for IIO
- Enables buffered data capture
- Removes the ADS114S0XB driver from kernel

## Prerequisites

Before running this script, ensure:

- You have `root` or `sudo` privileges.
- The Linux kernel has support for Industrial I/O.
- Required dependencies (e.g., `make`, `modprobe`) are installed.
- The `linux-embedded-driver` directory is present with the correct driver source.

## Usage

1. Grant execution permission if needed:
   ```bash
   chmod +x config_menu.sh
   ```
2. Run the script with:
   ```bash
   ./config_menu.sh
   ```
3. Follow the on-screen menu prompts to select desired operations.

## Script Functions

- **load\_modules**: Loads necessary kernel modules for IIO functionality.
- **load\_adc\_driver**: Builds and inserts the ADS114S0XB driver module.
- **configure\_trigger**: Sets up a sysfs-based trigger for the ADC device.
- **enable\_buffered\_capture**: Enables buffered mode for capturing ADC data.
- **unload\_adc\_driver**: Removes the  ADS114S0XB driver module from the kernel.

## Troubleshooting

- Ensure the `linux-embedded-driver` directory contains the correct driver source.
- Check `dmesg` logs for driver loading issues:
  ```bash
  dmesg | grep ads114s0xb
  ```
- Verify the IIO device appears under `/sys/bus/iio/devices/`.
