# ADS114S0xB Linux IIO Driver - Test

## Overview
This README is located in the test directory and primarily aims to explain how the driver was tested, debugged, and evidentiated. This is a Linux Industrial I/O (IIO) driver for the **Texas Instruments ADS114S0xB** multi-channel ADC family. The driver supports reading and writing registers, handling raw ADC data, and includes a **SENSOR_MOCK_MODE** for testing without a physical sensor.

## Features
- SPI-based communication with the ADS114S0xB ADC.
- Sysfs attributes for reading and writing register values.
- **SENSOR_MOCK_MODE** for testing and error injection.
- Integration with the IIO subsystem for raw ADC data access.

---

## SENSOR_MOCK_MODE

### Purpose
The **SENSOR_MOCK_MODE** is a feature that allows testing the driver without a physical ADS114S0xB sensor. It also provides a mechanism to inject controlled errors and test how downstream modules handle unexpected or corrupted data.

### How It Works
When enabled, the driver does **not** interact with the physical ADC hardware. Instead, it generates deterministic mock data. The mock value starts at zero and increments with each read operation 
(adc reads and register reads).

The mode is controlled via the sysfs attribute:

```sh
echo 1 > /sys/bus/iio/devices/iio:device0/SENSOR_MOCK_MODE
```

When enabled, ADC reads return incrementing mock values instead of real sensor data.


### Disabling Mock Mode
To restore real ADC readings:
```sh
echo 0 > /sys/bus/iio/devices/iio:device0/SENSOR_MOCK_MODE
```

---

## Testing Procedure

### 1. **Enabling SENSOR_MOCK_MODE**
```sh
echo 1 > /sys/bus/iio/devices/iio:device0/SENSOR_MOCK_MODE
```
Check `dmesg` for confirmation:
```sh
dmesg | grep ads114s0xb
```
Expected output:
```
ads114s0xb: SENSOR_MOCK_MODE enabled
```

### 2. **Reading Mock ADC Values**
```sh
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```
Expected: Incrementing mock values.

### 3. **Injecting a Specific Value**
This feature is not implemented.

### 4. **Simulating Faults**
This feature is not implemented.

### 5. **Evidence Collection**
All evidence, including test results and log outputs, is stored in text files available in the same test directory.

---

## Debugging
To troubleshoot:
- Use `dmesg` logs:
  ```sh
  dmesg | grep ads114s0xb
  ```
  This provides detailed kernel logs for driver operations.
- Verify sysfs file permissions:
  ```sh
  ls -l /sys/bus/iio/devices/iio:device0/
  ```
- Ensure the module is correctly reloaded:
  ```sh
  sudo rmmod ads114s0xb
  sudo insmod ads114s0xb.ko
  ```

---

