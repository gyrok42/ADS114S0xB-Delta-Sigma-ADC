# ADS114S0xB Device Driver

## Overview
This directory contains the Linux kernel driver for the 
**Texas Instruments ADS114S0xB** series of Delta-Sigma ADCs. 
The driver enables SPI communication with the ADC, 
providing functionality for initialization, register access, 
and ADC data retrieval via the **Industrial I/O (IIO) subsystem**.

## Features
- Initialization of the ADS114S0xB device.
- Support for SPI-based read and write operations.
- Ability to read and write to all registers on the device.
- Retrieval of ADC values via the IIO subsystem.

## Source Files
- **`ti-ads114s0xb.c`** - The main kernel module implementing SPI communication and IIO interface.
- **`Makefile`** - Build script for compiling and installing the kernel module.

## Driver Implementation

### 1. Initialization of Device
The driver initializes the **ADS114S0xB** device when the kernel module is loaded. This includes:
- Allocating the IIO device structure.
- Configuring the SPI interface.
- Registering the driver with the IIO subsystem.
- Performing a **reset** sequence on the ADC.

Implementation:
- `ads114s0xb_probe(struct spi_device *spi)` initializes the device upon SPI detection.
- `ads114s0xb_reset(struct iio_dev *indio_dev)` resets the ADC.

### 2. SPI Read/Write Operations
The driver supports SPI transactions to read from and write to the ADS114S0xB registers.

- `ads114s0xb_write_reg(struct iio_dev *indio_dev, u8 reg, u8 data)` writes to a register.
- `ads114s0xb_read_reg(struct iio_dev *indio_dev, u8 reg, u8 *data)` reads from a register.

### 3. Register Access
The driver provides access to all ADC registers using the IIO attribute mechanism.
- `ads114s0xb_attr_get(struct device *dev, struct device_attribute *attr, char *buf)` retrieves register values.
- `ads114s0xb_attr_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)` writes values to registers.

### 4. Reading ADC Values
There are two ways to read ADC values from the ADS114S0xB driver:

#### 1. Direct Read via `in_voltageX_raw`
- This method invokes the `ads114s0xb_read_raw` function, which:
  - Configures the ADC input multiplexer (`INPMUX` register).
  - Sends a **start conversion** command (`ADS114S0XB_CMD_START`).
  - Reads the conversion result using (`ADS114S0XB_CMD_RDATA`).

Function responsible:
- `ads114s0xb_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)`

#### 2. Buffered Read with Sysfs Trigger
- This method utilizes the **IIO buffer and trigger mechanism**, allowing continuous data acquisition.
- The ADC values are stored in a buffer and can be accessed via sysfs.
- The buffer is enabled via:
  ```sh
  echo 1 > /sys/bus/iio/devices/iio:deviceX/buffer/enable
  ```
- The trigger mechanism starts the acquisition process.
- The function responsible for handling buffered reads is:
  - `ads114s0xb_trigger_handler(int irq, void *private)`, which manages data collection and buffering.
See in the next section information about the **Config Menu**. Pre configuration is necessary
to enable the buffered read with sysfs trigger.

## Device Tree Configuration
The project includes a **Device Tree overlay** file located in the `device-tree/` directory. 
This overlay configures the ADS114S0xB device on a Raspberry Pi 5 for SPI communication.

```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "brcm,bcm2835";

    fragment@0 {
        target = <&spi0>;
        __overlay__ {
            status = "okay";

            ads114s06b@0 {
                compatible = "ti,ads114s06b";
                reg = <0>;  // SPI CS 0
                spi-max-frequency = <2000000>;  // Set appropriate SPI speed
                reset-gpios = <&gpio 25 0>; // GPIO25 as reset pin
            };
        };
    };
};
```

## Building and Installing the Driver
### 1. Build the Kernel Module
Run the following command inside the `linux-embedded-driver` directory:
```sh
make
```

### 2. Load the Kernel Module
Use `insmod` to manually load the module:
```sh
sudo insmod ti-ads114s0xb.ko
```

To verify the module is loaded:
```sh
dmesg | grep ads114s0xb
```

### 3. Unload the Module
To remove the driver:
```sh
sudo rmmod ti-ads114s0xb
```

## Makefile Options
The `Makefile` includes standard build rules for compiling the kernel module. It supports:
- `make` or `make all`- Compiles the module.
- `make clean` - Cleans up the build artifacts.
- `make insmod` - Load the module on linux kernel.
- `make rmmod` - Removes the module from linux kernel.
- `make install` - Installs the module in the system.

## Config Menu
The script `config_menu.sh` can be used to perform the building and installing tasks mentioned above.
In addition to that, it also configures the IIO Sysfs files to enable the buffered read.

## Debugging
Enable logging by checking kernel logs:
```sh
dmesg | grep ads114s0xb
```
