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
        std::vector<OpenCLDevice::Ptr> getAllDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::Ptr> getAllGPUDevices(bool enableVisualization = false);
        std::vector<OpenCLDevice::Ptr> getAllCPUDevices(bool enableVisualization = false);
        OpenCLDevice::Ptr getOneGPUDevice(bool enableVisualization = false);
        OpenCLDevice::Ptr getOneCPUDevice(bool enableVisualization = false);
        Host::Ptr getHostDevice();
        void setDefaultDevice(ExecutionDevice::Ptr device);
        void setDefaultComputationDevice(ExecutionDevice::Ptr device);
        void setDefaultVisualizationDevice(ExecutionDevice::Ptr device);
        ExecutionDevice::Ptr getDefaultComputationDevice();
        ExecutionDevice::Ptr getDefaultVisualizationDevice();
    private:
        DeviceManager() {};
        DeviceManager(DeviceManager const&); // Don't implement
        void operator=(DeviceManager const&); // Don't implement
        ExecutionDevice::Ptr mDefaultComputationDevice;
        ExecutionDevice::Ptr mDefaultVisualizationDevice;
};

}



#endif /* DEVICEMANAGER_HPP_ */
