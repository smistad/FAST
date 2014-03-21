#ifndef EXECUTIONDEVICE_HPP_
#define EXECUTIONDEVICE_HPP_

#include <boost/shared_ptr.hpp>
#include "Context.hpp"

namespace fast {

class ExecutionDevice {
    public:
        typedef boost::shared_ptr<ExecutionDevice> Ptr;
        bool isHost() {return mIsHost;};
    protected:
        bool mIsHost;

};

class Host : public ExecutionDevice {
    public:
        typedef boost::shared_ptr<Host> Ptr;
        static Host::Ptr New() {
            Host * ptr = new Host();
            Host::Ptr smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }
    private:
        Host() {mIsHost = true;};
        Host::Ptr mPtr;
        void setPtr(Host::Ptr ptr) {mPtr = ptr;};
};

class OpenCLDevice : public ExecutionDevice, public oul::Context {
    public:
        typedef boost::shared_ptr<OpenCLDevice> Ptr;
        static OpenCLDevice::Ptr New() {
            OpenCLDevice * ptr = new OpenCLDevice();
            OpenCLDevice::Ptr smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }
        cl::CommandQueue getCommandQueue();
        cl::Device getDevice();

        OpenCLDevice(std::vector<cl::Device> devices) : oul::Context(devices, NULL, false)
        {mIsHost = false;};
    private:
        OpenCLDevice() {mIsHost = false;};
        OpenCLDevice::Ptr mPtr;
        void setPtr(OpenCLDevice::Ptr ptr) {mPtr = ptr;};

};

} // end namespace fast

#endif /* EXECUTIONDEVICE_HPP_ */
