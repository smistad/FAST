#include "FAST/DeviceManager.hpp"
#include "OpenCLManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Visualization/Window.hpp"
#include <QApplication>

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

std::vector<OpenCLDevice::pointer> getDevices(oul::DeviceCriteria criteria, bool enableVisualization) {
    unsigned long * glContext = NULL;
    QGLWidget* widget = NULL;
    if(enableVisualization) {
        // Create GL context

		// Make sure only one QApplication is created
		fast::Window::getMainGLContext()->makeCurrent();
#if defined(__APPLE__) || defined(__MACOSX)
		CGLContextObj appleContext = CGLGetCurrentContext();
		std::cout << "Initial GL context: " << CGLGetCurrentContext() << std::endl;
		std::cout << "Initial GL share group: " << CGLGetShareGroup(CGLGetCurrentContext()) << std::endl;

		glContext = (unsigned long *)appleContext;
#elif _WIN32
        glContext = (unsigned long *)wglGetCurrentContext();
		std::cout << "Initial W GL context " << glContext << std::endl;
#else
        glContext = (long unsigned int*)glXGetCurrentContext();
        std::cout << "Initial GLX context " << glContext << std::endl;
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
        OpenCLDevice * device = new OpenCLDevice(deviceVector, glContext);
        executionDevices.push_back(OpenCLDevice::pointer(device));
    }}

    // Cleanup widget, widget has to be alive when creating device
    delete widget;

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
    std::vector<OpenCLDevice::pointer> devices = getDevices(criteria,enableVisualization);
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
    return Host::getInstance();
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

OpenCLDevice::pointer DeviceManager::getDevice(
        DeviceCriteria criteria) const {
    bool interop = false;
    if(criteria.hasCapabilityCriteria(DEVICE_CAPABILITY_OPENGL_INTEROP)) {
        interop = true;
    }
    criteria.setDeviceCountCriteria(1);
    std::vector<OpenCLDevice::pointer> devices = getDevices(criteria, interop);
    if(devices.size() == 0)
        throw Exception("Found no devices which satisfies the device criteria");
    return devices[0];
}

bool DeviceManager::deviceSatisfiesCriteria(OpenCLDevice::pointer device,
        const DeviceCriteria& criteria) const {
    oul::OpenCLManager * manager = oul::OpenCLManager::getInstance();
    return manager->deviceSatisfiesCriteria(criteria, device->getDevice());
}
