# ADS114S0xB Delta-Sigma ADC Device Driver and User Application

## Goal: Key Driver Components

A driver that performs the following:

- **Initialization of device**
- **Exercise valid SPI read/write operations**
- **Ability to Read/Write to all registers on the device**
- **Read valid ADC values from the device**

### User Space Application
A user space application that interacts with the developed driver should be created that shows:

- How to read ADC values
- How to read register values
- How to change ADC channels

## Overview

This project provides a device driver and a user-space application for the **Texas Instruments ADS114S0xB** series of Delta-Sigma ADCs. These ADCs are designed to measure low-voltage signals with high precision, making them suitable for sensor transducers, temperature controllers, and other industrial applications.

The driver implements a **Linux Industrial I/O (IIO) framework**-based SPI interface for the ADS114S0xB ADC, allowing user-space applications to communicate with the hardware efficiently. The user-space application demonstrates how to interact with the ADC, configure its settings, and retrieve voltage readings.

## Features

- Support for ADS114S06B and ADS114S08B models
- SPI-based communication using the Linux IIO framework
- Support for channel selection and configuration via the INPMUX register
- Programmable Gain Amplifier (PGA) with configurable gain settings
- Support for external and internal voltage references
- User-space application to trigger ADC conversions and read results

## How the ADS114S0xB ADC Works

The **ADS114S0xB** series ADCs include a **multiplexer (mux)** that allows selection of different input channels. However, only **one channel can be converted at a time**. Before starting a conversion, the **INPMUX register must be configured properly** to select the desired channel. The ADC then performs a conversion and outputs the digital result over SPI.

### Key Functional Components

- **Analog Multiplexer**: Selects one of the available input channels.
- **Programmable Gain Amplifier (PGA)**: Amplifies low-level signals for better resolution.
- **16-bit Delta-Sigma ADC**: Converts the selected channel's voltage into a digital value.
- **Voltage Reference Selection**: Can use an internal 2.5V reference or an external reference.
- **SPI Communication**: Used to send commands and receive conversion results.

## Code Components

### Kernel Device Driver (`ti-ads114s0xb.c`)

The driver provides an interface for the ADC through the **Linux IIO subsystem**. It includes:

- SPI communication handlers
- Channel configuration using INPMUX
- Data retrieval through IIO buffer mechanisms
- Integration with the Linux IIO framework

### User-Space Application (`user-space-app.cpp`)

The user application interacts with the device driver via **libiio**. It:

- Initializes the ADC
- Configures the desired channel
- Triggers a conversion
- Reads and displays the acquired voltage data

## Usage

### Building and Loading the Kernel Module

```sh
make
sudo insmod ti-ads114s0xb.ko
```

### Running the User-Space Application

```sh
g++ user-space-app.cpp -o user-space-app -liio
./user-space-app
```

### Configuring and Reading ADC Values

Once the driver is loaded, the ADC can be configured using the **sysfs** interface provided by the IIO framework or by using the user-space application.


## References
For more details, refer to the [ADS114S0xB Datasheet](https://www.ti.com/lit/ds/symlink/ads114s08b.pdf).
