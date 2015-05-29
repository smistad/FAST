#include "FAST/ExecutionDevice.hpp"
using namespace fast;

cl::CommandQueue OpenCLDevice::getCommandQueue() {
    return getQueue(0);
}

cl::Device OpenCLDevice::getDevice() {
    return Context::getDevice(0);
}

OpenCLDevice::~OpenCLDevice() {
     //std::cout << "DESTROYING opencl device object..." << std::endl;
     // Make sure that all queues are finished
     getQueue(0).finish();
}
