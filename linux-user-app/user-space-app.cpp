#include "ADS114S0XB.h"

int main() {
  try {
    ADS114S0XB adc;
    std::cout << "Initializing ADS114S0XB user-space program using libiio..." << std::endl;
    #if 0
    std::cout << "Reading STATUS register: "
    << adc.readRegister(ADS114S0XBRegister::STATUS) << std::endl;
    adc.writeRegister(ADS114S0XBRegister::PGA, "1");

    int voltage = adc.readVoltage(3);
    if (voltage != -1) {
    std::cout << "Voltage on channel 3: " << voltage << std::endl;
    }
    #endif
    // Set ADC channel to 0 and trigger a conversion
    adc.setChannel(0);
    adc.enableBuffer();

    for (int i = 0; i < 20; i++) {
      adc.triggerConversion();

      // Read and display buffer data
      auto data = adc.readBuffer();
      std::cout << "ADC Data: ";
      for (auto byte : data) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
      }
      std::cout << std::endl;
    }

    adc.disableBuffer();
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
