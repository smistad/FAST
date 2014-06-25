#ifndef OPENCLMANAGER_HPP_
#define OPENCLMANAGER_HPP_

#include "CL/OpenCL.hpp"
#include <vector>
#include "Context.hpp"
#include "DeviceCriteria.hpp"
#include "Exceptions.hpp"
#include "Reporter.hpp"
#include <utility>


namespace oul {

typedef std::pair<cl::Platform, std::vector<cl::Device> > PlatformDevices;

/**
 * Singleton class which is used mainly for creating OpenCL contexts in an easy way
 * In the long run this object will contain the state of OpenCL in an application.
 * E.g. which devices are currently in use, how much memory is used etc.
 */
class OpenCLManager {
    public:
        static OpenCLManager * getInstance();
        static void shutdown();

        Context createContext(
                std::vector<cl::Device> &devices,
                unsigned long * OpenGLContext = NULL, bool enableProfiling = false);
        Context createContext(
                cl::Device device,
                unsigned long * OpenGLContext = NULL, bool enableProfiling = false);
        Context createContext(
                int argc,
                char ** argv,
                DeviceCriteria &defaultCriteria);
        Context createContext(const DeviceCriteria &criteria, unsigned long * OpenGLContext = NULL, bool enableProfiling = false);

        ContextPtr createContextPtr(const DeviceCriteria &criteria, unsigned long * OpenGLContext = NULL, bool enableProfiling = false);


        std::vector<PlatformDevices> getDevices(const DeviceCriteria &criteria);
        std::vector<cl::Platform> getPlatforms(
                oul::DevicePlatform platformCriteria);

        bool deviceHasOpenGLInteropCapability(const cl::Device &device);
        bool devicePlatformMismatch(
                const cl::Device &device,
                const cl::Platform &platform);

        std::vector<cl::Device> getDevicesForBestPlatform(
                const DeviceCriteria& deviceCriteria,
               std::vector<PlatformDevices> &platformDevices);

    private:
        OpenCLManager();

        void sortDevicesAccordingToPreference(
                int numberOfPlatforms,
                int maxNumberOfDevices,
                std::vector<PlatformDevices> platformDevices,
                DevicePreference preference,
                std::vector<cl::Device> * sortedPlatformDevices,
                int * platformScores);
        DevicePlatform getDevicePlatform(std::string platformVendor);
        std::string getDevicePlatform(DevicePlatform devicePlatform);

        std::vector<cl::Platform> platforms;
        Reporter reporter;

        static OpenCLManager * instance;
};

OpenCLManager* opencl(); //Shortcut for accessing the OpenCLManager

#ifdef _WIN32
// This is a function used to get the device context needed to create OpenGL context on windows
HDC getHDC();
#endif

}
;
// namespace oul

#endif /* OPENCLMANAGER_HPP_ */
