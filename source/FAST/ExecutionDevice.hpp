#ifndef EXECUTIONDEVICE_HPP_
#define EXECUTIONDEVICE_HPP_

#include "FAST/Object.hpp"
#include "FAST/SmartPointers.hpp"
#include "Context.hpp"

namespace fast {

class ExecutionDevice : public Object {
    public:
        typedef SharedPointer<ExecutionDevice> pointer;
        bool isHost() {return mIsHost;};
        virtual ~ExecutionDevice() {};
        static std::string getStaticNameOfClass() {
            return "ExecutionDevice";
        }
    protected:
        bool mIsHost;

};

class DeviceManager; // Forward declaration
class Host : public ExecutionDevice {
    public:
        typedef SharedPointer<Host> pointer;
        static Host::pointer getInstance() {
            static Host::pointer instance = Host::New();
            return instance;
        }
        static std::string getStaticNameOfClass() {
            return "Host";
        }
    // Declare factory method New private
    private:
        static Host::pointer New() {
            Host* ptr = new Host();
            Host::pointer smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }
        void setPtr(Host::pointer ptr) {
            mPtr = ptr;
        }
        Host() {
            mIsHost = true;
        }

};

class OpenCLDevice : public ExecutionDevice, public oul::Context {
    FAST_OBJECT(OpenCLDevice)
    public:
        cl::CommandQueue getCommandQueue();
        cl::Device getDevice();

        OpenCLDevice(std::vector<cl::Device> devices, unsigned long * glContext = NULL) : oul::Context(devices, glContext, false)
        {mIsHost = false;mGLContext = glContext;};
        unsigned long * getGLContext() { return mGLContext; };
        std::string getName() {
            return getDevice().getInfo<CL_DEVICE_NAME>();
        }
        ~OpenCLDevice();
    private:
        OpenCLDevice() {mIsHost = false;};
        unsigned long * mGLContext;

};

} // end namespace fast

#endif /* EXECUTIONDEVICE_HPP_ */
