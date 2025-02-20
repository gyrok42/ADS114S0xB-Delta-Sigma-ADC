# ADS114S0xB Delta-Sigma ADC

#### Device Driver and User Application

### Goals and Key Driver Components

A driver that performs the following:

- Initialization of device
- Exercise valid SPI read/write operations
- Ability to Read/Write to all registers on the device
- Read valid ADC values from the device

### User Space Application
A user space application that interacts with the developed driver should be created that shows:

- How to read ADC values
- How to read register values
- How to change ADC channels

### System Design

The ADS114S0xB driver was designed to run on linux. Since the chip communicates using the SPI bus,
the device driver binds to a SPI bus driver. The instance of the SPI bus is chosen on the device tree.

The IIO subsystem was designed to support devices like the ADS114S0xB ADC and it is pertty much
a standard for such devices.

It also provides a standard way to create sysfs interfaces with the user space. It also allows us
to create custom sysfs interfaces. For this exercise, I created a interface for each register that
can be read or written. Only for the sake of the exercise.
Normally that is not necessary, usually the registers are managed internally by the device driver
ant it takes care of the values being read from and write to the registers.
For example, this chip has a register called INPMUX. One channel can be converted at a give time,
and that register is used to select one of the channels to be converted. It is not necessary to 
expose it. The users are able to select which channel by setting the corresponding voltageX_en
file and then reading its corresponding voltageX_raw to obtain the ADC values.

Let's take a look at the system design figure below:

![ads114s0xb](images/ads114s0xb_design.png)

#### Device Driver Implementation

The `ads114s0xb_probe(struct spi_device *spi)` does the initialization. It registers the module,
binds with the SPI bus driver and reset the chip. 
Note that I connected it to a ficticious reset GPIO pin since at the moment the sensor is not available 
and wiring up is not at the current scope.
I also created a trigger handle using the IIO API `devm_iio_triggered_buffer_setup`. That enables
the `ads114s0xb_trigger_handler` to be called when buffered ADC data is requested from
user space. More details about this will be covered later.

The `ads114s0xb_attr_get` and `ads114s0xb_attr_set` are used to handle events of read/write to the
custom attibutes that were created. I created a custom attribute for each corresponding chip register.
A sysfs file is created bu the IIO subsustem and read or wirte operations are handled by those two handlers.
For example, I created the custom attribute DATARATE. If the user calls `echo 3 > DATARATE`, the handler
`ads114s0xb_attr_set` will be called. I associated to each attribute its corresponding register address.
From the handler, it uses `iio_attr->address` to know which register should be written. Likewise for the
`ads114s0xb_attr_get`.

An important handler to mention is the `ads114s0xb_read_raw`. It is invoked when the user reads
the channle's corresponding sysfs file. For example, `cat in_voltage2_raw` will invoke the 
`ads114s0xb_read_raw` handler. The parameter chan will have the index of the channel. In this case
in the will be 2. Which corresponds to the chip's channels 2 (from 0 to 6 or 12). The `ads114s0xb_read_raw`
send via SPI command to set the channel to INPMUX register, sends the SPI start command to the chip.
The chip will start coverting (it will have some delay described in the datasheet), and a command
to read the SPI will acquired the channel data. After that it sends via SPI the stop command.

I added another feature that is the user triggered data read. It creates a buffer and the user
is able to trigger the driver to send more ADC data. In that scenario, the user first selects
the channel via `scan_elements/in_voltageX_en` sysfs file. For example, `echo 1 > scan_elements/in_voltage5_en` will call the handler `ads114s0xb_update_scan_mode`. On that handler, a **scanmask**
is passed to the handler so that we may test what are the channels enabled. In that case it will
find that the channel 5 was enabled. If the channel 2 was previously selected and it was not
set back to zero, setting the channel 5 will not make any effect in the driver version.
Once the channel is properly selecte and the buffer is set in the IIO subsystem by
calling `echo 1 > buffer/enable`, every write to `"/sys/bus/iio/devices/" + SYSFS_TRIGGER_INSTANCE + "/trigger_now"` will call the `ads114s0xb_trigger_handler`. That handler will call the read data
from ADC via SPI command to acquire the data in between start and stop conversion commands to
the chip.

#### User Space App Implementation

I created the most simple thing possible. I created the ADS114S0XB class to interface with the sysfs and IIO interface. It has simple methods to do the necessary operations to read/wirte registers via custom attribute files, it also has methods to select the channels and read ADC data.
I created the IIOSysfsFileUtil.h to be used by the ADS114S0XB. It handles the sysfs constant strings and some
sysfs utils. It can be used by other drivers too. 
I avoided to use exceptions. Only one thrown in the code, but the idea is to not having any exception.
Normally we keep a better control of the status more directly in codes that need to be safe. What do you
think about it ? Good subject for discussion. I particularly like the StatusOr approach.
See [Class StatusOr<T>](https://cloud.google.com/cpp/docs/reference/common/latest/classgoogle_1_1cloud_1_1StatusOr) for more details.
Overall the ADS114S0XB is pretty simple and straight forward as it should be, no fancy design was necessary. Normally we do the best to make simple solutions even to complex problems as much as possible. In this case there is no complexity in this level of the design.

And last but not least, the `user-space-app.cpp` has the main function. Again, something pretty simple and straight foward for the exercise.
```cpp
int main() {
  using namespace adcs;
  ADS114S0XB adc;
  auto initStatus = adc.initialize();

  if (initStatus.first != 0) {
    std::cerr
      << "ADS114S0XB initialize error ("
      << initStatus.second << "): "
      << strerror(initStatus.first)
      << std::endl;
  }

  enableDriverMockSensor(adc);
  
  auto channel = 3;
  // How to read ADC values / How to change ADC channels
  readAdcData(adc, channel, 20);
  // How to read register values
  readRegisters(adc);
  // How to write register values
  writeRegisters(adc);
 
  return EXIT_SUCCESS;
}
```


### Configuration and Initialization Scripts
The project includes a script to assist with configuring and initializing the ADS114S0XB device. 
The script provides a menu with the following options:

```
==================================
 ADS114S0XB Configuration Menu
==================================
1. Load required modules
2. Load ADS114S0XB driver
3. Install ADS114S0XB driver
4. Configure IIO interface
5. Unload ADS114S0XB driver
6. Exit
==================================
```

The script automates essential setup tasks, ensuring that the driver is properly installed and 
the IIO interface is configured before data acquisition.

**Important Notes:**
- **Options 1 to 4 are necessary** for setting up the environment before using the ADC.
- The script should be **executed as root** to ensure proper module loading and driver installation.

The project includes a script to assist with configuring and initializing the ADS114S0XB device. 
The script provides a menu with the following options:


## Overview

This project provides a device driver and a user-space application for the 
**Texas Instruments ADS114S0xB** series of Delta-Sigma ADCs. 
These ADCs are designed to measure low-voltage signals with high precision, 
making them suitable for sensor transducers, temperature controllers, 
and other industrial applications.

The driver implements a **Linux Industrial I/O (IIO) framework**-based SPI 
interface for the ADS114S0xB ADC, allowing user-space applications to communicate 
with the hardware efficiently. The user-space application demonstrates how to 
interact with the ADC, configure its settings, and retrieve voltage readings.

## Features

- Support for ADS114S06B and ADS114S08B models
- SPI-based communication using the Linux IIO framework
- Support for channel selection
- Exposed registers for read/write configurations
- User-space application to trigger ADC conversions and read results

## How the ADS114S0xB ADC Works

The **ADS114S0xB** series ADCs include a **multiplexer (mux)** that allows 
selection of different input channels. However, only 
**one channel can be converted at a time**. 
Before starting a conversion, the **INPMUX register must be configured properly** 
to select the desired channel. The ADC then performs a conversion and 
outputs the digital result over SPI.

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

Once the driver is loaded, the ADC can be configured using the **sysfs** 
interface provided by the IIO framework or by using the user-space application.

## Directory Structure

```
.
├── device-tree
│   ├── ads114xb-overlay.dtbo
│   └── ads114xb-overlay.dts
├── LICENSE
├── linux-embedded-driver
│   ├── Makefile
│   ├── README.md
│   └── ti-ads114s0xb.c
├── linux-user-app
│   ├── ADS114S0XB.h
│   ├── IIOSysfsFilesUtil.h
│   ├── Makefile
│   ├── README.md
│   └── user-space-app.cpp
├── README.md
├── scripts
│   ├── config_menu.sh
│   └── README.md
└── tests
    ├── buffer_reading_from_app.txt
    ├── channel_set_buffer_read.txt
    ├── dmesg_log.txt
    ├── drafts.txt
    ├── linux-user-app-evidence.txt
    ├── parameter_validation_evidences.txt
    ├── README.md
    └── registration_evidences.txt
```

## References
For more details, refer to the [ADS114S0xB Datasheet](https://www.ti.com/lit/ds/symlink/ads114s08b.pdf).

