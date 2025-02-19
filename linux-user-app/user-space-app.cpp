#include "ADS114S0XB.h"

void readAdcData (adcs::ADS114S0XB &adc, int channel, int count) {
  using namespace adcs;

  // Set ADC channel to 0 and trigger a conversion
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
        std::cout << "Read register success: " << reg.value() << std::endl;
    }
    else {
      std::cout << "Error reading register" << std::endl;
    }
  }
}
int main() {
  using namespace adcs;
  ADS114S0XB adc;
  adc.initialize();
  auto channel = 3;
  readAdcData(adc, channel, 20);
  readRegisters(adc);
 
  return EXIT_SUCCESS;
}
