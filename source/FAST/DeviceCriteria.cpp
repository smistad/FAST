#include "DeviceCriteria.hpp"

namespace fast {

const std::vector<DeviceCapability>& DeviceCriteria::getCapabilityCriteria() const {
    return capabilityCriteria;
}

DevicePlatform DeviceCriteria::getPlatformCriteria() const {
    return platformCriteria;
}

void DeviceCriteria::setDeviceCountCriteria(unsigned int count) {
    this->deviceCountMin = count;
    this->deviceCountMax = count;
}

void DeviceCriteria::setDeviceCountCriteria(unsigned int min, unsigned int max) {
    this->deviceCountMin = min;
    this->deviceCountMax = max;
}

DeviceType DeviceCriteria::getTypeCriteria() const {
    return typeCriteria;
}

DeviceCriteria::DeviceCriteria() {
    typeCriteria = DEVICE_TYPE_ANY;
    deviceCountMin = 0;
    deviceCountMax = 100;
    platformCriteria = DEVICE_PLATFORM_ANY;
    devicePreference = DEVICE_PREFERENCE_NONE;
}

void DeviceCriteria::setPlatformCriteria(DevicePlatform platform) {
    platformCriteria = platform;
}

void DeviceCriteria::setCapabilityCriteria(DeviceCapability capability) {
    capabilityCriteria.push_back(capability);
}

void DeviceCriteria::setTypeCriteria(DeviceType typeCriteria) {
    this->typeCriteria = typeCriteria;
}

unsigned int DeviceCriteria::getDeviceCountMinCriteria() const {
    return deviceCountMin;
}

void DeviceCriteria::setDevicePreference(DevicePreference preference) {
    this->devicePreference = preference;
}

DevicePreference DeviceCriteria::getDevicePreference() const {
    return devicePreference;
}

unsigned int DeviceCriteria::getDeviceCountMaxCriteria() const {
    return deviceCountMax;
}

bool DeviceCriteria::hasCapabilityCriteria(DeviceCapability capability) const {
    bool found = false;
    for(int i = 0; i < capabilityCriteria.size(); i++) {
        if(capabilityCriteria[i] == capability) {
            found = true;
            break;
        }
    }
    return found;
}

}
