# ADS114S0xB Linux IIO Driver - Test

## Overview
This README aims to primarily aims to explain how the driver was tested, debugged, and evidentiated.  The driver supports reading and writing registers, handling raw ADC data, and includes a **SENSOR_MOCK_MODE** for testing without a physical sensor or error injection 
to test downstream modules and systems.

## Features
- SPI-based communication with the ADS114S0xB ADC.
- Sysfs attributes for reading and writing register values.
- **SENSOR_MOCK_MODE** for testing and error injection.
- Integration with the IIO subsystem for raw ADC data access.

---

## SENSOR_MOCK_MODE

### Purpose
The **SENSOR_MOCK_MODE** is a feature that allows testing the driver without a physical ADS114S0xB sensor. It also provides a mechanism to inject controlled data to enable test on downstream modules on how they handle unexpected or corrupted data.

### How It Works
When enabled, the driver does **not** retrieve with the physical ADC hardware data. Instead, it generates deterministic mock data. The mock value starts at zero and increments with each read operation (adc reads and register reads).

The mock mode is controlled via the sysfs attribute:

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

## Manual Testing Procedure

Before testing, please use the config menu to prepare the module and linux IIO.

See the [Scripts readme](scripts/README.md) and follow the instructions.

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
cat /sys/bus/iio/devices/iio:device0/in_voltage1_raw
cat /sys/bus/iio/devices/iio:device0/in_voltage2_raw
cat /sys/bus/iio/devices/iio:device0/in_voltage3_raw
cat /sys/bus/iio/devices/iio:device0/in_voltage4_raw
cat /sys/bus/iio/devices/iio:device0/in_voltage5_raw
```
Expected: Incrementing mock values.

Note that ADS114S06B has 6 channels and ADS114S08B has 12 channels.

### 3. **Reading Registers**
```sh
sudo su
cat DATARATE
cat dev
cat FSCAL0
cat FSCAL1
cat GPIOCON
cat GPIODAT
cat ID
cat IDACMAG
cat IDACMUX
cat INPMUX
cat OFCAL0
cat OFCAL1
cat PGA
cat REF
cat STATUS
cat SYS
cat VBIAS
```

### 3. **Writing into Registers**
```sh
sudo su
echo 1 > DATARATE
echo 0 > dev
echo 1 > FSCAL0
echo 2 > FSCAL1
echo 3 > GPIOCON
echo 4 > GPIODAT
echo 5 > ID
echo 6 > IDACMAG
echo 7 > IDACMUX
echo 8 > INPMUX
echo 9 > OFCAL0
echo 0 > OFCAL1
echo 1 > PGA
echo 2 > REF
echo 3 > STATUS
echo 4 > SYS
echo 5 > VBIAS
```
### 3. **Injecting a Specific Value**
This feature is not implemented.

### 4. **Simulating Faults**
This feature is not implemented.

### 6. **Evidence Collection**
All evidence of the performed tests, including test results and log outputs, is stored in text files available in the same test directory.

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

## Test Using the user-space-app Procedure

Used the user space app to test.

Please see the [linux-user-app readme](linux-user-app/README.md) directory for more details.
