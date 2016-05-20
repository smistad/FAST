#ifndef EXECUTIONDEVICE_HPP_
#define EXECUTIONDEVICE_HPP_

#include "FAST/Object.hpp"
#include "FAST/SmartPointers.hpp"
#include "RuntimeMeasurementManager.hpp"

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

class OpenCLDevice : public ExecutionDevice {
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
        RuntimeMeasurementsManagerPtr getRunTimeMeasurementManager();
        ~OpenCLDevice();
    private:
        OpenCLDevice();
        unsigned long * mGLContext;
		cl::Program writeBinary(std::string absolute_filename, std::string buildOptions);
        cl::Program readBinary(std::string filename);
		cl::Program buildProgramFromBinary(std::string absolute_filename, std::string buildOptions);
        cl::Program buildSources(cl::Program::Sources source, std::string buildOptions);

        std::string getUniqueFilePathForWriting(std::string absolute_filename_of_kernel_file, std::string ending, std::size_t hash);
        std::string getFilePathForBinary(std::string absolute_filename_of_kernel_file, std::size_t hash);
        std::string getFilePathForCache(std::string absolute_filename_of_kernel_file, std::size_t hash);

        cl::Context context;
        std::vector<cl::CommandQueue> queues;
        std::map<std::string, int> programNames;
        std::vector<cl::Program> programs;
        std::vector<cl::Device> devices;
        cl::Platform platform;

        bool profilingEnabled;
        RuntimeMeasurementsManagerPtr runtimeManager;

};

} // end namespace fast

#endif /* EXECUTIONDEVICE_HPP_ */
