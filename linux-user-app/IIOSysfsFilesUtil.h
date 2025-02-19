#pragma once

#include <string>
namespace adcs
{
class IIOSysfsFilesUtil {
public:
	IIOSysfsFilesUtil() : IIOSysfsFilesUtil(DEFAULT_IIO_DEVICE_NAME) {
	}
	
	IIOSysfsFilesUtil(const std::string &deviceId) : 
		IIO_DEVICE_NAME(deviceId),
		SYSFS_BUFFER_INTERFACE("/dev/" + IIO_DEVICE_NAME),
		SYSFS_TRIGGER("/sys/bus/iio/devices/" + SYSFS_TRIGGER_INSTANCE + "/trigger_now") {
	}

	const std::string& getIIODeviceName() const { return IIO_DEVICE_NAME; }
	const std::string& getBufferEnable() const { return SYSFS_BUFFER_ENABLE; }
	const std::string& getBufferInterface() const { return SYSFS_BUFFER_INTERFACE; }
	const std::string& getTriggerInstance() const { return SYSFS_TRIGGER_INSTANCE; }
	const std::string& getScanVoltage() const { return SYSFS_SCAN_VOLTAGE; }
	const std::string& getEnableId() const { return SYSFS_ENABLE_ID; }
	const std::string& getTrigger() const { return SYSFS_TRIGGER; }
	const std::string& getFlagOn() const { return SYSFS_FLAG_ON; }
	const std::string& getFlagOff() const { return SYSFS_FLAG_OFF; }
	std::string getVoltageEnable(int channel) const {
		return SYSFS_SCAN_VOLTAGE + std::to_string(channel) + SYSFS_ENABLE_ID;
	}

private:
	const std::string DEFAULT_IIO_DEVICE_NAME{"iio:device0"};
	const std::string IIO_DEVICE_NAME; 
	const std::string SYSFS_BUFFER_ENABLE{"buffer/enable"};
	const std::string SYSFS_BUFFER_INTERFACE;
	const std::string SYSFS_TRIGGER_INSTANCE{"trigger0"};
	const std::string SYSFS_SCAN_VOLTAGE{"scan_elements/in_voltage"};
	const std::string SYSFS_ENABLE_ID{"_en"};
	const std::string SYSFS_TRIGGER;
	const std::string SYSFS_FLAG_ON{"1"};
	const std::string SYSFS_FLAG_OFF{"0"};
};

} // namespace adcs
