#include "DeviceManager.hpp"
#include "OpenCLManager.hpp"
#include "Exception.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <CL/cl_gl.h>
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#endif
#endif

using namespace fast;

DeviceManager& DeviceManager::getInstance() {
    static DeviceManager instance;
    return instance;
}

std::vector<OpenCLDevice::pointer> getDevices(oul::DeviceCriteria criteria, bool enableVisualization, unsigned long * mGLContext = NULL) {
    unsigned long * glContext = NULL;
    if(enableVisualization) {
        // Create GL context
#if defined(__APPLE__) || defined(__MACOSX)
	std::cout << "trying to create a MAC os X GL context" << std::endl;
CGLPixelFormatAttribute attribs[13] = {
    kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core, // This sets the context to 3.2
    kCGLPFAColorSize,     (CGLPixelFormatAttribute)24,
    kCGLPFAAlphaSize,     (CGLPixelFormatAttribute)8,
    kCGLPFAAccelerated,
    kCGLPFADoubleBuffer,
    kCGLPFASampleBuffers, (CGLPixelFormatAttribute)1,
    kCGLPFASamples,       (CGLPixelFormatAttribute)4,
    (CGLPixelFormatAttribute)0
};
        CGLPixelFormatObj pix;
GLint npix;
        CGLChoosePixelFormat(attribs, &pix, &npix);
        CGLContextObj appleGLContext;
std::cout << "asd" << std::endl;
        CGLCreateContext(pix, NULL, &appleGLContext);
std::cout << "asd" << std::endl;
        glContext = (unsigned long *)appleGLContext;
//CGLSetCurrentContext(appleGLContext);
std::cout << "the device manager created the GL context " << glContext << std::endl;
#else
#if _WIN32
        // TODO implement windows OpenGL stuff
        // http://msdn.microsoft.com/en-us/library/windows/desktop/dd374379%28v=vs.85%29.aspx
        HDC    hdc = oul::getHDC();
		std::cout << "HDC in DeviceManager is: " << hdc << std::endl;
        HGLRC  hglrc; 
         
        // create a rendering context  
        hglrc = wglCreateContext (hdc); 
		std::cout << "GL context in DeviceManager is: " << hglrc << std::endl;
        glContext = (unsigned long *)hglrc;
         
        // make it the calling thread's current rendering context 
        wglMakeCurrent (hdc, hglrc);
#else
        int sngBuf[] = { GLX_RGBA,
                         GLX_DOUBLEBUFFER,
                         GLX_RED_SIZE, 1,
                         GLX_GREEN_SIZE, 1,
                         GLX_BLUE_SIZE, 1,
                         GLX_DEPTH_SIZE, 12,
                         None
        };
        Display * display = XOpenDisplay(0);
        XVisualInfo* vi = glXChooseVisual(display, DefaultScreen(display), sngBuf);
        glContext = (unsigned long *)glXCreateContext(display, vi, 0, GL_TRUE);
#endif
#endif
        criteria.setCapabilityCriteria(oul::DEVICE_CAPABILITY_OPENGL_INTEROP);
    }
    oul::OpenCLManager * manager = oul::OpenCLManager::getInstance();
    std::vector<oul::PlatformDevices> platformDevices = manager->getDevices(criteria);

    std::vector<OpenCLDevice::pointer> executionDevices;
    for(unsigned int i = 0; i < platformDevices.size(); i++) {
    for(unsigned int j = 0; j < platformDevices[i].second.size(); j++) {
        std::vector<cl::Device> deviceVector;
        deviceVector.push_back(platformDevices[i].second[j]);
std::cout << "creating a device using GL context: " << CGLGetCurrentContext() << std::endl;
        OpenCLDevice * device = new OpenCLDevice(deviceVector, mGLContext);
        executionDevices.push_back(OpenCLDevice::pointer(device));
    }}

    return executionDevices;
}

std::vector<OpenCLDevice::pointer> DeviceManager::getAllDevices(
        bool enableVisualization) {

    oul::DeviceCriteria criteria;
    return getDevices(criteria,enableVisualization);
}

std::vector<OpenCLDevice::pointer> DeviceManager::getAllGPUDevices(
        bool enableVisualization) {
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    return getDevices(criteria,enableVisualization);
}

std::vector<OpenCLDevice::pointer> DeviceManager::getAllCPUDevices(
        bool enableVisualization) {
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_CPU);
    return getDevices(criteria,enableVisualization);
}

OpenCLDevice::pointer DeviceManager::getOneGPUDevice(
        bool enableVisualization) {

    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    criteria.setDeviceCountCriteria(1);
    std::vector<OpenCLDevice::pointer> devices = getDevices(criteria,enableVisualization,mGLContext);
	if(devices.size() == 0)
		throw Exception("No compatible OpenCL devices found");
    return devices[0];
}

OpenCLDevice::pointer DeviceManager::getOneOpenCLDevice(
        bool enableVisualization) {

    oul::DeviceCriteria criteria;

    // Check if a GPU is available first, if not choose any
    criteria.setTypeCriteria(oul::DEVICE_TYPE_GPU);
    criteria.setDeviceCountCriteria(1);
    std::vector<OpenCLDevice::pointer> devices = getDevices(criteria,enableVisualization);
    if(devices.size() > 0) {
        return devices[0];
    } else {
        criteria.setTypeCriteria(oul::DEVICE_TYPE_ANY);
        std::vector<OpenCLDevice::pointer> devices = getDevices(criteria,enableVisualization);
		if(devices.size() == 0)
			throw Exception("No compatible OpenCL devices found");
        return devices[0];
    }
}

OpenCLDevice::pointer DeviceManager::getOneCPUDevice(
        bool enableVisualization) {
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(oul::DEVICE_TYPE_CPU);
    criteria.setDeviceCountCriteria(1);
    std::vector<OpenCLDevice::pointer> devices = getDevices(criteria,enableVisualization);
	if(devices.size() == 0)
		throw Exception("No compatible OpenCL devices found");
    return devices[0];
}

Host::pointer DeviceManager::getHostDevice() {
    return Host::New();
}

void DeviceManager::setDefaultDevice(ExecutionDevice::pointer device) {
    mDefaultComputationDevice = device;
    mDefaultVisualizationDevice = device;
}

void DeviceManager::setDefaultComputationDevice(
        ExecutionDevice::pointer device) {
    mDefaultComputationDevice = device;
}

void DeviceManager::setDefaultVisualizationDevice(
        ExecutionDevice::pointer device) {
    mDefaultVisualizationDevice = device;
}

ExecutionDevice::pointer DeviceManager::getDefaultComputationDevice() {
    return mDefaultComputationDevice;
}

ExecutionDevice::pointer DeviceManager::getDefaultVisualizationDevice() {
    return mDefaultVisualizationDevice;
}

DeviceManager::DeviceManager() {
    // Set one random device as default device
    setDefaultDevice(getOneOpenCLDevice(true));
}
