#include <chrono>
#include <iio.h>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

constexpr const char *DEVICE_NAME = "iio:device0";

enum class ADS114S0XBRegister {
  DATARATE,
  FSCAL0,
  FSCAL1,
  GPIOCON,
  GPIODAT,
  ID,
  IDACMAG,
  IDACMUX,
  INPMUX,
  OFCAL0,
  OFCAL1,
  PGA,
  REF,
  SENSOR_MOCK_MODE,
  STATUS,
  SYS,
  VBIAS
};

const std::unordered_map<ADS114S0XBRegister, std::string> registerMap = {
    {ADS114S0XBRegister::DATARATE, "DATARATE"},
    {ADS114S0XBRegister::FSCAL0, "FSCAL0"},
    {ADS114S0XBRegister::FSCAL1, "FSCAL1"},
    {ADS114S0XBRegister::GPIOCON, "GPIOCON"},
    {ADS114S0XBRegister::GPIODAT, "GPIODAT"},
    {ADS114S0XBRegister::ID, "ID"},
    {ADS114S0XBRegister::IDACMAG, "IDACMAG"},
    {ADS114S0XBRegister::IDACMUX, "IDACMUX"},
    {ADS114S0XBRegister::INPMUX, "INPMUX"},
    {ADS114S0XBRegister::OFCAL0, "OFCAL0"},
    {ADS114S0XBRegister::OFCAL1, "OFCAL1"},
    {ADS114S0XBRegister::PGA, "PGA"},
    {ADS114S0XBRegister::REF, "REF"},
    {ADS114S0XBRegister::SENSOR_MOCK_MODE, "SENSOR_MOCK_MODE"},
    {ADS114S0XBRegister::STATUS, "STATUS"},
    {ADS114S0XBRegister::SYS, "SYS"},
    {ADS114S0XBRegister::VBIAS, "VBIAS"}};

class ADS114S0XB {
public:
  ADS114S0XB() {
    ctx = iio_create_local_context();
    if (!ctx) {
      std::cerr << "Failed to create IIO context\n";
    }
    dev = iio_context_find_device(ctx, DEVICE_NAME);
    if (!dev) {
      std::cerr << "Device not found!\n";
    }
  }

  virtual ~ADS114S0XB() {
    if (ctx) {
      iio_context_destroy(ctx);
    }
  }

  bool writeRegister(ADS114S0XBRegister reg, const std::string &value) {
    if (!dev)
      return false;
    return iio_device_attr_write(dev, registerMap.at(reg).c_str(),
                                 value.c_str()) >= 0;
  }

  std::string readRegister(ADS114S0XBRegister reg) {
    if (!dev)
      return "";
    char buf[128];
    if (iio_device_attr_read(dev, registerMap.at(reg).c_str(), buf,
                             sizeof(buf)) < 0) {
      std::cerr << "Error reading " << registerMap.at(reg) << std::endl;
      return "";
    }
    return std::string(buf);
  }

  int readVoltage(int channel) {
    std::string channelStr = std::to_string(channel);
    if (!writeRegister(ADS114S0XBRegister::INPMUX, channelStr)) {
      return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::string rawValue =
        readRegister(static_cast<ADS114S0XBRegister>(channel));
    return rawValue.empty() ? -1 : std::stoi(rawValue);
  }
};

int main() {
  ADS114S0XB adc;

  std::cout << "Reading STATUS register: "
            << adc.readRegister(ADS114S0XBRegister::STATUS) << std::endl;
  adc.writeRegister(ADS114S0XBRegister::PGA, "1");

  int voltage = adc.readVoltage(3);
  if (voltage != -1) {
    std::cout << "Voltage on channel 3: " << voltage << std::endl;
  }

  return 0;
}
