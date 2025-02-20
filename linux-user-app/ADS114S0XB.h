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

#include "IIOSysfsFilesUtil.h"

namespace adcs
{
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
        STATUS,
        SYS,
        VBIAS,
        SENSOR_MOCK_MODE,
        COUNT, //Sentinel Value
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
        {ADS114S0XBRegister::STATUS, "STATUS"},
        {ADS114S0XBRegister::SYS, "SYS"},
        {ADS114S0XBRegister::VBIAS, "VBIAS"},
        {ADS114S0XBRegister::SENSOR_MOCK_MODE, "SENSOR_MOCK_MODE"},
    };
    ADS114S0XB() {
    }
    // Prefer to use something similar to StatusOr<T> as a return
    // https://cloud.google.com/cpp/docs/reference/common/latest/classgoogle_1_1cloud_1_1StatusOr
    std::pair<int, std::string> initialize() {
        if (_ctx != nullptr) {
            return {0, "already initialized"};
        }
        _ctx = iio_create_default_context();
        if (!_ctx) {
            return {errno, "iio_create_default_context"};
        }

        _dev = iio_context_find_device(_ctx, _iioSysfs.getIIODeviceName().c_str());
        if (!_dev) {
            iio_context_destroy(_ctx);
            _ctx = nullptr;
            return {errno, "iio_create_default_context"};
        }

        _trigger = iio_context_find_device(_ctx, _iioSysfs.getTriggerInstance().c_str());
        if (!_trigger) {
            iio_context_destroy(_ctx);
            _ctx = nullptr;
            return {errno, "iio_create_default_context, trigger"};
        }
        return {0,""};
    }

    ~ADS114S0XB() {
        if (_ctx != nullptr) {
            iio_context_destroy(_ctx);
        }
    }
    static constexpr std::string _in_voltage_x_en{};
    void setChannel(int channel) {
        disableBuffer();
        setAttribute(
            _iioSysfs.getVoltageEnable(channel), 
            _iioSysfs.getFlagOn());
    }
    void resetChannel(int channel) {
        disableBuffer();
        setAttribute(
            _iioSysfs.getVoltageEnable(channel), 
            _iioSysfs.getFlagOff());

    }
    
    
    void enableBuffer() {
        setAttribute(_iioSysfs.getBufferEnable(), _iioSysfs.getFlagOn());
    }

    void disableBuffer() {
        setAttribute(_iioSysfs.getBufferEnable(), _iioSysfs.getFlagOff());
    }

    void setRegister(const std::string &reg, int value) {
        setAttribute(reg, std::to_string(value));
    }

    bool triggerConversion() {
        std::ofstream trigger_file(_iioSysfs.getTrigger());
        if (!trigger_file) {
            return false;
        }

        trigger_file << _iioSysfs.getFlagOn();
        trigger_file.close();
        return true;
    }

    std::optional<std::vector<uint8_t>> readBuffer(size_t size = BUFFER_SIZE) {
        std::ifstream adc_data_file(_iioSysfs.getBufferInterface(), std::ios::binary);
        if (!adc_data_file) {
            return std::nullopt;
        }
    
        std::vector<uint8_t> data(size);
        adc_data_file.read(reinterpret_cast<char*>(data.data()), size);
    
        return data;
    }

    std::optional<ssize_t> writeRegister(ADS114S0XBRegister reg, const std::string &value) {
        if (!_dev)
            return std::nullopt;

        auto ret = 
            iio_device_attr_write(
                _dev, 
                registerMap.at(reg).c_str(),
                value.c_str()) >= 0;

        if ( errno < 0) {
            return std::nullopt;
        }

        return ret;
    }

    std::optional<std::string> readRegister(ADS114S0XBRegister reg) {
        if (!_dev)
            return std::nullopt;
        char buf[128];
        auto ret =
            iio_device_attr_read(
                _dev, registerMap.at(reg).c_str(),
                buf,
                sizeof buf);
        if ( ret < 0) {
            return std::nullopt;
        }
        return std::string(buf);
    }

    int getLastErrno() const {
        return _last_errno;
    }

private:
    struct iio_context *_ctx = nullptr;
    struct iio_device *_dev = nullptr;
    struct iio_device *_trigger = nullptr;
    static const size_t BUFFER_SIZE{2};
    int _last_errno = 0;
    std::string _lastFunctionError;
    IIOSysfsFilesUtil _iioSysfs;

    void setAttribute(const std::string &attr, const std::string &value) {
        if (iio_device_attr_write(_dev, attr.c_str(), value.c_str()) < 0) {
            throw std::runtime_error("Failed to write attribute: " + attr);
        }
    }
};

} // namespace adcs::ads1140xb
