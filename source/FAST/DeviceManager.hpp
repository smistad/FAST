#ifndef DEVICEMANAGER_HPP_
#define DEVICEMANAGER_HPP_

#include "FAST/ExecutionDevice.hpp"
#include <vector>
#include "DeviceCriteria.hpp"

namespace fast {

using oul::DeviceCriteria;
using oul::DeviceType;
using oul::DevicePlatform;
using oul::DeviceCapability;
using oul::DevicePreference;
using oul::DEVICE_TYPE_ANY;
using oul::DEVICE_TYPE_GPU;
using oul::DEVICE_TYPE_CPU;
using oul::DEVICE_PLATFORM_ANY;
using oul::DEVICE_PLATFORM_AMD;
using oul::DEVICE_PLATFORM_NVIDIA;
using oul::DEVICE_PLATFORM_INTEL;
using oul::DEVICE_PLATFORM_APPLE;
using oul::DEVICE_CAPABILITY_OPENGL_INTEROP;
using oul::DEVICE_PREFERENCE_NONE;
using oul::DEVICE_PREFERENCE_NOT_CONNECTED_TO_SCREEN;
using oul::DEVICE_PREFERENCE_COMPUTE_UNITS;
using oul::DEVICE_PREFERENCE_GLOBAL_MEMORY;

/**
 * Singleton class for retrieving and setting default execution devices
 */
class DeviceManager {
    public:
        static DeviceManager& getInstance();
        OpenCLDevice::pointer getDevice(DeviceCriteria criteria) const;
        bool deviceSatisfiesCriteria(OpenCLDevice::pointer, const DeviceCriteria& criteria) const;
        std::vector<OpenCLDevice::pointer> getAllDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::pointer> getAllGPUDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::pointer> getAllCPUDevices(bool enableVisualization = false);
        OpenCLDevice::pointer getOneOpenCLDevice(bool enableVisualization = false);
        OpenCLDevice::pointer getOneGPUDevice(bool enableVisualization = false);
        OpenCLDevice::pointer getOneCPUDevice(bool enableVisualization = false);
        static Host::pointer getHostDevice();
        void setDefaultDevice(ExecutionDevice::pointer device);
        void setDefaultComputationDevice(ExecutionDevice::pointer device);
        void setDefaultVisualizationDevice(ExecutionDevice::pointer device);
        ExecutionDevice::pointer getDefaultComputationDevice();
        ExecutionDevice::pointer getDefaultVisualizationDevice();
        void setGLContext(unsigned long * glContext) { mGLContext = glContext;};
    private:
	unsigned long * mGLContext;
        DeviceManager();
        DeviceManager(DeviceManager const&); // Don't implement
        void operator=(DeviceManager const&); // Don't implement
        ExecutionDevice::pointer mDefaultComputationDevice;
        ExecutionDevice::pointer mDefaultVisualizationDevice;
};

}



#endif /* DEVICEMANAGER_HPP_ */
