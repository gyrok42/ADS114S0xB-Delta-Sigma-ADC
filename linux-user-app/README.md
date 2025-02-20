# user-space-app

## Overview

`user-space-app` is a simple application designed to demonstrate how to interact with an ADC (Analog-to-Digital Converter) using the `ADS114S0XB` driver. This application provides functionalities to:

- Read raw ADC data
- Read register values from the ADC
- Write register values to the ADC
- Enable the driver to mock physical sensor data for testing

## Prerequisites

- Ensure the `ADS114S0XB` driver is available and properly configured.
- A compatible hardware setup that allows SPI communication with the ADC.
- A Linux-based system to run the application in user space.

## Build and Run

To compile and run the application, use:

```sh
 make clean
 make all
 ./user-space-app
```

## Functionality

### Initialization

Before performing any ADC operations, the application initializes the ADC driver:

```cpp
ADS114S0XB adc;
auto initStatus = adc.initialize();

if (initStatus.first != 0) {
  std::cerr << "ADS114S0XB initialize error ("
            << initStatus.second << "): "
            << strerror(initStatus.first)
            << std::endl;
}
```

This ensures that the ADC is ready before proceeding.

### Enabling Mock Sensor Mode

This function enables the driver’s mock sensor mode by writing a value to the `SENSOR_MOCK_MODE` register.

```cpp
void enableDriverMockSensor(adcs::ADS114S0XB &adc) {
  using ADS114S0XB::ADS114S0XBRegister::SENSOR_MOCK_MODE;
  adc.writeRegister(SENSOR_MOCK_MODE, "1");
}
```

### Reading ADC Data

The `readAdcData` function demonstrates how to:
- Set the ADC channel.
- Enable and trigger an ADC conversion.
- Read and display the converted ADC data.

```cpp
void readAdcData (adcs::ADS114S0XB &adc, int channel, int count) {
  adc.setChannel(channel);
  adc.enableBuffer();

  for (int i = 0; i < count; i++) {
    if (!adc.triggerConversion()) {
      std::cout << "Error triggering." << std::endl;
      break;
    }
    auto data = adc.readBuffer();
    if (data) {
      std::cout << "ADC data: ";
      for (auto byte : data.value()) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
      }
      std::cout << std::endl;
    } else {
      std::cout << "Error reading ADC data" << std::endl;
    }
  }
  adc.disableBuffer();
}
```

### Reading ADC Registers

The `readRegisters` function reads the values of ADC registers and prints them.

```cpp
void readRegisters(adcs::ADS114S0XB &adc) {
  using ADS114S0XB::ADS114S0XBRegister::FSCAL0;
  if (auto reg = adc.readRegister(FSCAL0)) {
    std::cout << "Read register success: " << reg.value() << std::endl;
  } else {
    std::cout << "Error reading register" << std::endl;
  }

  for (auto &reg_id : adc.registerMap) {
    if (auto reg = adc.readRegister(reg_id.first)) {
      std::cout << "Read " << reg_id.second << " register success: " << reg.value() << std::endl;
    } else {
      std::cout << "Error reading register" << std::endl;
    }
  }
}
```

### Writing ADC Registers

The `writeRegisters` function writes a value to the ADC registers.

```cpp
void writeRegisters(adcs::ADS114S0XB &adc) {
  using ADS114S0XB::ADS114S0XBRegister::DATARATE;
  adc.writeRegister(DATARATE, "5");

  for (auto &reg_id : adc.registerMap) {
    if (auto size = adc.writeRegister(reg_id.first, "5")) {
      std::cout << "Write " << reg_id.second << " register success: " << size.value() << " bytes" << std::endl;
    } else {
      std::cout << "Error writing register" << std::endl;
    }
  }
}
```

### Main Execution

The `main` function initializes the ADC, enables the mock sensor mode, reads ADC data, and demonstrates register read/write operations.

```cpp
int main() {
  ADS114S0XB adc;
  auto initStatus = adc.initialize();
  if (initStatus.first != 0) {
    std::cerr << "ADS114S0XB initialize error (" << initStatus.second << "): " << strerror(initStatus.first) << std::endl;
  }

  enableDriverMockSensor(adc);
  readAdcData(adc, 3, 20);
  readRegisters(adc);
  writeRegisters(adc);

  return EXIT_SUCCESS;
}
```

## Conclusion

This application serves as an example of how to interact with the ADS114S0XB ADC in a user-space environment. It demonstrates reading and writing registers, configuring ADC channels, and acquiring raw ADC data.

