#include "DeviceManager.hpp"
#include "OpenCLManager.hpp"
using namespace fast;

DeviceManager& DeviceManager::getInstance() {
    static DeviceManager instance;
    return instance;
}

std::vector<OpenCLDevice::Ptr> getDevices(oul::DeviceCriteria criteria) {
    oul::OpenCLManager * manager = oul::OpenCLManager::getInstance();
    std::vector<oul::PlatformDevices> platformDevices = manager->getDevices(criteria);

    std::vector<OpenCLDevice::Ptr> executionDevices;
    for(int i = 0; i < platformDevices.size(); i++) {
    for(int j = 0; j < platformDevices[i].second.size(); j++) {
        std::vector<cl::Device> deviceVector;
        deviceVector.push_back(platformDevices[i].second[j]);
        OpenCLDevice * device = new OpenCLDevice(deviceVector);
        executionDevices.push_back(OpenCLDevice::Ptr(device));
    }}

    return executionDevices;
}

std::vector<OpenCLDevice::Ptr> DeviceManager::getAllDevices(
        bool enableVisualization) {

    oul::DeviceCriteria criteria;
    return getDevices(criteria);
}

std::vector<OpenCLDevice::Ptr> DeviceManager::getAllGPUDevices(
        bool enableVisualization) {
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    return getDevices(criteria);
}

std::vector<OpenCLDevice::Ptr> DeviceManager::getAllCPUDevices(
        bool enableVisualization) {
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_CPU);
    return getDevices(criteria);
}

OpenCLDevice::Ptr DeviceManager::getOneGPUDevice(
        bool enableVisualization) {

    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    criteria.setDeviceCountCriteria(1);
    std::vector<OpenCLDevice::Ptr> devices = getDevices(criteria);
    return devices[0];
}

OpenCLDevice::Ptr DeviceManager::getOneCPUDevice(
        bool enableVisualization) {
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_CPU);
    criteria.setDeviceCountCriteria(1);
    std::vector<OpenCLDevice::Ptr> devices = getDevices(criteria);
    return devices[0];
}

Host::Ptr DeviceManager::getHostDevice() {
    return Host::New();
}

void DeviceManager::setDefaultDevice(ExecutionDevice::Ptr device) {
    mDefaultComputationDevice = device;
    mDefaultVisualizationDevice = device;
}

void DeviceManager::setDefaultComputationDevice(
        ExecutionDevice::Ptr device) {
    mDefaultComputationDevice = device;
}

void DeviceManager::setDefaultVisualizationDevice(
        ExecutionDevice::Ptr device) {
    mDefaultVisualizationDevice = device;
}

ExecutionDevice::Ptr DeviceManager::getDefaultComputationDevice() {
    return mDefaultComputationDevice;
}

ExecutionDevice::Ptr DeviceManager::getDefaultVisualizationDevice() {
    return mDefaultVisualizationDevice;
}
