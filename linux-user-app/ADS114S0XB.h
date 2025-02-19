#pragma once

#include <iostream>
#include <fstream>
#include <iio.h>
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <unordered_map>
#include <optional>

class ADS114S0XB {
public:
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
        {ADS114S0XBRegister::VBIAS, "VBIAS"}
    };

    ADS114S0XB() {
        _ctx = iio_create_default_context();
        if (!_ctx) {
            throw std::runtime_error("Failed to create IIO context.");
        }

        _dev = iio_context_find_device(_ctx, IIO_DEVICE_NAME.c_str());
        if (!_dev) {
            throw std::runtime_error("Failed to find IIO device: " + std::string(IIO_DEVICE_NAME));
        }

        _trigger = iio_context_find_device(_ctx, _sysfs_trigger_instance.c_str());
        if (!_trigger) {
            throw std::runtime_error("Failed to find IIO trigger.");
        }
    }

    ~ADS114S0XB() {
        iio_context_destroy(_ctx);
    }
    static constexpr std::string _in_voltage_x_en{};
    void setChannel(int channel) {
        disableBuffer();
        setAttribute(
            _sysfs_scan_voltage + std::to_string(channel) + _sysfs_enable_id, 
            _sysfs_flag_on);
        enableBuffer();
        std::cout << "Switched to channel: " << channel << std::endl;
    }
    
    void enableBuffer() {
        setAttribute(_sysfs_buffer_enable, _sysfs_flag_on);
    }

    void disableBuffer() {
        setAttribute(_sysfs_buffer_enable, _sysfs_flag_off);
    }

    void setRegister(const std::string &reg, int value) {
        setAttribute(reg, std::to_string(value));
    }

    void triggerConversion() {
        std::ofstream trigger_file(_sysfs_trigger);
        if (!trigger_file) {
            throw std::runtime_error("Failed to open trigger_now file.");
        }

        trigger_file << _sysfs_flag_on;
        trigger_file.close();
    }

    std::vector<uint8_t> readBuffer(size_t size = BUFFER_SIZE) {
        std::ifstream adc_data_file(_sysfs_buffer_interface, std::ios::binary);
        if (!adc_data_file) {
            throw std::runtime_error("Failed to open ADC data file: " + IIO_DEVICE_NAME);
        }
    
        std::vector<uint8_t> data(size);
        adc_data_file.read(reinterpret_cast<char*>(data.data()), size);
    
        return data;
    }

    bool writeRegister(ADS114S0XBRegister reg, const std::string &value) {
        if (!_dev)
            return false;

        return
            iio_device_attr_write(
                _dev, 
                registerMap.at(reg).c_str(),
                value.c_str()) >= 0;
    }

    std::optional<std::string> readRegister(ADS114S0XBRegister reg) {
        if (!_dev)
            return std::nullopt;
        char buf[128];
        auto ret =
            iio_device_attr_read(
                _dev, registerMap.at(reg).c_str(),
                buf,
                sizeof(buf));
        if ( ret < 0) {
            return std::nullopt;
        }
        return std::string(buf);
    }

private:
    struct iio_context *_ctx = nullptr;
    struct iio_device *_dev = nullptr;
    struct iio_device *_trigger = nullptr;
    const std::string IIO_DEVICE_NAME{"iio:device0"};
    static const size_t BUFFER_SIZE{2}; 
    const std::string _sysfs_buffer_enable{"buffer/enable"};
    const std::string _sysfs_buffer_interface{"/dev/" + IIO_DEVICE_NAME};
    const std::string _sysfs_trigger_instance{"trigger0"};
    const std::string _sysfs_scan_voltage{"scan_elements/in_voltage"};
    const std::string _sysfs_enable_id{"_en"};
    const std::string _sysfs_trigger{"/sys/bus/iio/devices/" + _sysfs_trigger_instance + "/trigger_now"};
    const std::string _sysfs_flag_on{"1"};
    const std::string _sysfs_flag_off{"0"};

    void setAttribute(const std::string &attr, const std::string &value) {
        if (iio_device_attr_write(_dev, attr.c_str(), value.c_str()) < 0) {
            throw std::runtime_error("Failed to write attribute: " + attr);
        }
    }
};
