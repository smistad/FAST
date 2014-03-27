#ifndef EXECUTIONDEVICE_HPP_
#define EXECUTIONDEVICE_HPP_

#include <boost/shared_ptr.hpp>
#include "Context.hpp"

namespace fast {

class ExecutionDevice {
    public:
        typedef boost::shared_ptr<ExecutionDevice> pointer;
        bool isHost() {return mIsHost;};
        virtual ~ExecutionDevice() {};
    protected:
        bool mIsHost;

};

class Host : public ExecutionDevice {
    public:
        typedef boost::shared_ptr<Host> pointer;
        static Host::pointer New() {
            Host * ptr = new Host();
            Host::pointer smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }
    private:
        Host() {mIsHost = true;};
        Host::pointer mPtr;
        void setPtr(Host::pointer ptr) {mPtr = ptr;};
};

class OpenCLDevice : public ExecutionDevice, public oul::Context {
    public:
        typedef boost::shared_ptr<OpenCLDevice> pointer;
        static OpenCLDevice::pointer New() {
            OpenCLDevice * ptr = new OpenCLDevice();
            OpenCLDevice::pointer smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }
        cl::CommandQueue getCommandQueue();
        cl::Device getDevice();

        OpenCLDevice(std::vector<cl::Device> devices) : oul::Context(devices, NULL, false)
        {mIsHost = false;};
    private:
        OpenCLDevice() {mIsHost = false;};
        OpenCLDevice::pointer mPtr;
        void setPtr(OpenCLDevice::pointer ptr) {mPtr = ptr;};

};

} // end namespace fast

#endif /* EXECUTIONDEVICE_HPP_ */
