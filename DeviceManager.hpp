#ifndef DEVICEMANAGER_HPP_
#define DEVICEMANAGER_HPP_

#include "ExecutionDevice.hpp"
#include <vector>

namespace fast {

/**
 * Singleton class for retrieving and setting default execution devices
 */
class DeviceManager {
    public:
        static DeviceManager& getInstance();
        std::vector<OpenCLDevice::pointer> getAllDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::pointer> getAllGPUDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::pointer> getAllCPUDevices(bool enableVisualization = false);
        OpenCLDevice::pointer getOneOpenCLDevice(bool enableVisualization = false);
        OpenCLDevice::pointer getOneGPUDevice(bool enableVisualization = false);
        OpenCLDevice::pointer getOneCPUDevice(bool enableVisualization = false);
        Host::pointer getHostDevice();
        void setDefaultDevice(ExecutionDevice::pointer device);
        void setDefaultComputationDevice(ExecutionDevice::pointer device);
        void setDefaultVisualizationDevice(ExecutionDevice::pointer device);
        ExecutionDevice::pointer getDefaultComputationDevice();
        ExecutionDevice::pointer getDefaultVisualizationDevice();
    private:
        DeviceManager();
        DeviceManager(DeviceManager const&); // Don't implement
        void operator=(DeviceManager const&); // Don't implement
        ExecutionDevice::pointer mDefaultComputationDevice;
        ExecutionDevice::pointer mDefaultVisualizationDevice;
};

}



#endif /* DEVICEMANAGER_HPP_ */
