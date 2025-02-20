#include <cstring>

#include "ADS114S0XB.h"

void readAdcData (adcs::ADS114S0XB &adc, int channel, int count) {
  using namespace adcs;

  // Set ADC channel to 0 and trigger a conversion
  // How to change ADC channels
  adc.setChannel(channel);
  adc.enableBuffer();

  for (int i = 0; i < count; i++) {
    if (!adc.triggerConversion()) {
      std::cout << "Error triggering." << std::endl;
      break;
    }
    // Read and display buffer data
    auto data = adc.readBuffer();
    if (data) {
      std::cout << "ADC data: ";
      for (auto byte : data.value()) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
      }
      std::cout << std::endl;
    }
    else {
      std::cout << "Error reading ADC data" << std::endl;
    }
  }
  adc.resetChannel(channel);
  adc.disableBuffer();
}

void readRegisters(adcs::ADS114S0XB &adc) {
  using namespace adcs;
  using ADS114S0XB::ADS114S0XBRegister::FSCAL0;
  // Reading a register from the ADC, for example FSCAL0
  if (auto reg = adc.readRegister(FSCAL0)) {
    std::cout << "Read register success: " << reg.value() << std::endl;
  }
  else {
    std::cout << "Error reading register" << std::endl;
  }
  // Reading ALL registers
  for (auto &reg_id : adc.registerMap) {
    if (auto reg = adc.readRegister(reg_id.first)) {
      std::cout 
        << "Read " << reg_id.second 
        << " register sucess: " << reg.value()
        << std::endl;
    }
    else {
      std::cout << "Error reading register" << std::endl;
    }
  }
}

void writeRegisters(adcs::ADS114S0XB &adc) {
  using namespace adcs;
  using ADS114S0XB::ADS114S0XBRegister::DATARATE;

  adc.writeRegister(DATARATE, "5");

  // Writing ALL registers
  for (auto &reg_id : adc.registerMap) {
    if (auto size = adc.writeRegister(reg_id.first, "5")) {
      std::cout 
        << "Write " << reg_id.second 
        << " register sucess: " << size.value()
        << " bytes"
        << std::endl;
    }
    else {
      std::cout << "Error reading register" << std::endl;
    }
  }
}

void enableDriverMockSensor(adcs::ADS114S0XB &adc) {
  using namespace adcs;
  using ADS114S0XB::ADS114S0XBRegister::SENSOR_MOCK_MODE;
  adc.writeRegister(SENSOR_MOCK_MODE, "1");
}

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
