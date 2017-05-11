#ifndef DEVICEMANAGER_HPP_
#define DEVICEMANAGER_HPP_

#include "FAST/ExecutionDevice.hpp"
#include <vector>
#include "FAST/DeviceCriteria.hpp"

namespace fast {

typedef std::pair<cl::Platform, std::vector<cl::Device> > PlatformDevices;

/**
 * Singleton class for retrieving and setting default execution devices
 */
class FAST_EXPORT  DeviceManager : public Object {
    public:
        static DeviceManager* getInstance();
        static bool hasInstance();
		static void deleteInstance();
        OpenCLDevice::pointer getDevice(DeviceCriteria criteria);
        bool deviceSatisfiesCriteria(OpenCLDevice::pointer, const DeviceCriteria& criteria);
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
        std::vector<PlatformDevices> getDevices(const DeviceCriteria &criteria);
        std::vector<OpenCLDevice::pointer> getDevices(DeviceCriteria criteria, bool enableVisualization);
        std::vector<cl::Platform> getPlatforms(
                DevicePlatform platformCriteria);

        bool deviceSatisfiesCriteria(const DeviceCriteria& criteria, const cl::Device &device);
        bool deviceHasOpenGLInteropCapability(const cl::Device &device);
        bool devicePlatformMismatch(
                const cl::Device &device,
                const cl::Platform &platform);

        std::vector<cl::Device> getDevicesForBestPlatform(
                const DeviceCriteria& deviceCriteria,
               std::vector<PlatformDevices> &platformDevices);
    	static bool isGLInteropEnabled();
    	void initialize();
    private:
        unsigned long * mGLContext;
    	static DeviceManager* mInstance;
        DeviceManager();
        DeviceManager(DeviceManager const&); // Don't implement
        void operator=(DeviceManager const&); // Don't implement
        ExecutionDevice::pointer mDefaultComputationDevice;
        ExecutionDevice::pointer mDefaultVisualizationDevice;
        void sortDevicesAccordingToPreference(
                int numberOfPlatforms,
                int maxNumberOfDevices,
                std::vector<PlatformDevices> platformDevices,
                DevicePreference preference,
                std::vector<cl::Device> * sortedPlatformDevices,
                int * platformScores);
        DevicePlatform getDevicePlatform(std::string platformVendor);
        std::string getDevicePlatform(DevicePlatform devicePlatform);

        std::vector<cl::Platform> platforms;
        static bool mDisableGLInterop;
};

}



#endif /* DEVICEMANAGER_HPP_ */
