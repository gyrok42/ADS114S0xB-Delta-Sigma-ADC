#pragma once

#include <iostream>
#include <fstream>
#include <iio.h>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <unordered_map>

constexpr const char *IIO_DEVICE_NAME = "iio:device0";
constexpr size_t BUFFER_SIZE = 2; 

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
        _ctx = iio_create_default_context();
        if (!_ctx) {
            throw std::runtime_error("Failed to create IIO context.");
        }

        _dev = iio_context_find_device(_ctx, IIO_DEVICE_NAME);
        if (!_dev) {
            throw std::runtime_error("Failed to find IIO device: " + std::string(IIO_DEVICE_NAME));
        }

        _trigger = iio_context_find_device(_ctx, "trigger0");
        if (!_trigger) {
            throw std::runtime_error("Failed to find IIO trigger.");
        }
    }

    ~ADS114S0XB() {
        if (_buffer) iio_buffer_destroy(_buffer);
        iio_context_destroy(_ctx);
    }

    void setChannel(int channel) {
        disableBuffer();
        setAttribute("scan_elements/in_voltage" + std::to_string(channel) + "_en", "1");
        enableBuffer();
        std::cout << "Switched to channel: " << channel << std::endl;
    }

    void enableBuffer() {
        setAttribute("buffer/enable", "1");
        std::cout << "Buffer enabled." << std::endl;
    }

    void disableBuffer() {
        setAttribute("buffer/enable", "0");
        std::cout << "Buffer disabled." << std::endl;
    }

    void setRegister(const std::string &reg, int value) {
        setAttribute(reg, std::to_string(value));
        std::cout << "Set register " << reg << " to " << value << std::endl;
    }
#if 0
    void triggerConversion() {
        setAttribute("/sys/bus/iio/devices/trigger0/trigger_now", "1");
        std::cout << "Triggered ADC conversion." << std::endl;
    }
#endif
    void triggerConversion() {
	std::ofstream trigger_file("/sys/bus/iio/devices/trigger0/trigger_now");
	if (!trigger_file) {
	    throw std::runtime_error("Failed to open trigger_now file.");
	}
    
	trigger_file << "1";
	trigger_file.close();
	std::cout << "Triggered ADC conversion." << std::endl;
    }

    std::vector<uint8_t> readBuffer(size_t size = BUFFER_SIZE) {
        std::ifstream adc_data_file("/dev/iio:device0", std::ios::binary);
        if (!adc_data_file) {
            throw std::runtime_error("Failed to open ADC data file: /dev/iio:device0");
        }
    
        std::vector<uint8_t> data(size);
        
        adc_data_file.read(reinterpret_cast<char*>(data.data()), size);
        
        size_t bytesRead = adc_data_file.gcount(); // Get actual bytes read
        if (bytesRead != size) {
            std::cerr << "Warning: Read fewer bytes than expected (" << bytesRead << " instead of " << size << ")" << std::endl;
            data.resize(bytesRead); // Adjust size if fewer bytes were read
        }
    
        return data;
    }
    

    bool writeRegister(ADS114S0XBRegister reg, const std::string &value) {
	if (!_dev)
	  return false;
	return iio_device_attr_write(_dev, registerMap.at(reg).c_str(),
				     value.c_str()) >= 0;
      }
    
      std::string readRegister(ADS114S0XBRegister reg) {
	if (!_dev)
	  return "";
	char buf[128];
	if (iio_device_attr_read(_dev, registerMap.at(reg).c_str(), buf,
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

private:
    struct iio_context *_ctx = nullptr;
    struct iio_device *_dev = nullptr;
    struct iio_device *_trigger = nullptr;
    struct iio_buffer *_buffer = nullptr;

    void setAttribute(const std::string &attr, const std::string &value) {
        if (iio_device_attr_write(_dev, attr.c_str(), value.c_str()) < 0) {
            throw std::runtime_error("Failed to write attribute: " + attr);
        }
    }
};
