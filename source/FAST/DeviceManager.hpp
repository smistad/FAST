#pragma once

#include <FAST/ExecutionDevice.hpp>
#include <vector>
#include <FAST/DeviceCriteria.hpp>

namespace fast {

typedef std::pair<cl::Platform, std::vector<cl::Device> > PlatformDevices;

/**
 * Singleton class for retrieving and setting default execution devices
 */
class FAST_EXPORT DeviceManager : public Object {
    public:
        static DeviceManager* getInstance();
        static void setDefaultPlatform(DevicePlatform platform);
		static void deleteInstance();
        OpenCLDevice::pointer getDevice(DeviceCriteria criteria);
        bool deviceSatisfiesCriteria(OpenCLDevice::pointer, const DeviceCriteria& criteria);
        std::vector<OpenCLDevice::pointer> getAllDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::pointer> getAllGPUDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::pointer> getAllCPUDevices(bool enableVisualization = false);
        OpenCLDevice::pointer getOneOpenCLDevice(bool enableVisualization = false, DevicePlatform platform = DEVICE_PLATFORM_ANY);
        OpenCLDevice::pointer getOneGPUDevice(bool enableVisualization = false);
        OpenCLDevice::pointer getOneCPUDevice(bool enableVisualization = false);
        static Host::pointer getHostDevice();
        void setDefaultDevice(ExecutionDevice::pointer device);
        ExecutionDevice::pointer getDefaultDevice();
        std::vector<PlatformDevices> getDevices(const DeviceCriteria &criteria);
        std::vector<OpenCLDevice::pointer> getDevices(DeviceCriteria criteria, bool enableVisualization);
        std::vector<cl::Platform> getPlatforms(
                DevicePlatform platformCriteria);

        bool deviceSatisfiesCriteria(const DeviceCriteria& criteria, const cl::Device &device, const cl::Platform &platform);
        bool deviceHasOpenGLInteropCapability(const cl::Device &device, const cl::Platform &platform);
        bool devicePlatformMismatch(
                const cl::Device &device,
                const cl::Platform &platform);

        std::vector<cl::Device> getDevicesForBestPlatform(
                const DeviceCriteria& deviceCriteria,
               std::vector<PlatformDevices> &platformDevices);
    private:
    	static DeviceManager* mInstance;
    	static DevicePlatform m_devicePlatform;
        DeviceManager();
        DeviceManager(DeviceManager const&); // Don't implement
        void operator=(DeviceManager const&); // Don't implement
        ExecutionDevice::pointer mDefaultComputationDevice;
        void sortDevicesAccordingToPreference(
                int numberOfPlatforms,
                int maxNumberOfDevices,
                std::vector<PlatformDevices> platformDevices,
                DevicePreference preference,
                std::vector<cl::Device> * sortedPlatformDevices,
                std::vector<int>& platformScores);
        DevicePlatform getDevicePlatform(std::string platformVendor);
        std::string getDevicePlatform(DevicePlatform devicePlatform);

        std::vector<cl::Platform> platforms;
};

}
