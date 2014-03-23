#include "ExecutionDevice.hpp"
using namespace fast;

cl::CommandQueue OpenCLDevice::getCommandQueue() {
    return getQueue(0);
}
