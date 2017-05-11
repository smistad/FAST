#ifndef EXECUTIONDEVICE_HPP_
#define EXECUTIONDEVICE_HPP_

#include "FAST/Object.hpp"
#include "FAST/SmartPointers.hpp"
#include "RuntimeMeasurementManager.hpp"

namespace fast {

class FAST_EXPORT  ExecutionDevice : public Object {
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
class FAST_EXPORT  Host : public ExecutionDevice {
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


enum OpenCLPlatformVendor {
    PLATFORM_VENDOR_APPLE,
    PLATFORM_VENDOR_AMD,
    PLATFORM_VENDOR_INTEL,
    PLATFORM_VENDOR_NVIDIA,
    PLATFORM_VENDOR_UKNOWN
};

enum DeviceVendor {
    DEVICE_VENDOR_AMD,
    DEVICE_VENDOR_INTEL,
    DEVICE_VENDOR_NVIDIA,
    DEVICE_VENDOR_UKNOWN
};

class FAST_EXPORT  OpenCLDevice : public ExecutionDevice {
    FAST_OBJECT(OpenCLDevice)
    public:
        cl::CommandQueue getCommandQueue();
        cl::Device getDevice();

        int createProgramFromSource(std::string filename, std::string buildOptions = "", bool caching = true);
        int createProgramFromSource(std::vector<std::string> filenames, std::string buildOptions = "");
        int createProgramFromString(std::string code, std::string buildOptions = "");
        int createProgramFromSourceWithName(std::string programName, std::string filename, std::string buildOptions = "");
        int createProgramFromSourceWithName(std::string programName, std::vector<std::string> filenames, std::string buildOptions = "");
        int createProgramFromStringWithName(std::string programName, std::string code, std::string buildOptions = "");
        cl::Program getProgram(unsigned int i);
        cl::Program getProgram(std::string name);
        bool hasProgram(std::string name);

        bool isImageFormatSupported(cl_channel_order order, cl_channel_type type, cl_mem_object_type imageType);

        //OpenCLPlatformVendor getPlatformVendor();
        //DeviceVendor getDeviceVendor();

        cl::CommandQueue getQueue(unsigned int i);
        cl::Device getDevice(unsigned int i);
        cl::Device getDevice(cl::CommandQueue queue);
        cl::Context getContext();
        cl::Platform getPlatform();

        OpenCLDevice(std::vector<cl::Device> devices, unsigned long* glContext = NULL);
        unsigned long * getGLContext() { return mGLContext; };
        std::string getName() {
            return getDevice().getInfo<CL_DEVICE_NAME>();
        }
        bool isWritingTo3DTexturesSupported();
        RuntimeMeasurementsManager::pointer getRunTimeMeasurementManager();
        ~OpenCLDevice();
    private:
        OpenCLDevice();
        unsigned long * mGLContext;
        cl::Program writeBinary(std::string filename, std::string buildOptions);
        cl::Program readBinary(std::string filename);
        cl::Program buildProgramFromBinary(std::string filename, std::string buildOptions);
        cl::Program buildSources(cl::Program::Sources source, std::string buildOptions);

        cl::Context context;
        std::vector<cl::CommandQueue> queues;
        std::map<std::string, int> programNames;
        std::vector<cl::Program> programs;
        std::vector<cl::Device> devices;
        cl::Platform platform;

        bool profilingEnabled;
        RuntimeMeasurementsManager::pointer runtimeManager;

};

} // end namespace fast

#endif /* EXECUTIONDEVICE_HPP_ */
