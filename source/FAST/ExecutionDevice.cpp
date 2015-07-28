#include "FAST/ExecutionDevice.hpp"
using namespace fast;

cl::CommandQueue OpenCLDevice::getCommandQueue() {
    return getQueue(0);
}

cl::Device OpenCLDevice::getDevice() {
    return Context::getDevice(0);
}

bool OpenCLDevice::isWritingTo3DTexturesSupported() {
    return Context::getDevice(0).getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
}

OpenCLDevice::~OpenCLDevice() {
     //std::cout << "DESTROYING opencl device object..." << std::endl;
     // Make sure that all queues are finished
     getQueue(0).finish();
}
