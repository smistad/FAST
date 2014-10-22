#include "DeviceManager.hpp"
#include "OpenCLManager.hpp"
#include "Exception.hpp"
#include "SimpleWindow.hpp"
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
    if(enableVisualization) {
        // TODO: this can be simplified to just use the QGLContext class instead
        // Create GL context
#if defined(__APPLE__) || defined(__MACOSX)
	std::cout << "trying to create a MAC os X GL context" << std::endl;
    // Make sure only one QApplication is created
    SimpleWindow::initializeQtApp();

    QGLWidget* widget = new QGLWidget;

    // Create GL context
    QGLContext* context = new QGLContext(QGLFormat::defaultFormat(), widget); // by including widget here the context becomes valid
    context->create();
    if(!context->isValid()) {
        throw Exception("QGL context is invalid!");
    }

    context->makeCurrent();
CGLContextObj appleContext = CGLGetCurrentContext();
    std::cout << "Initial GL context: " << CGLGetCurrentContext() << std::endl;
    std::cout << "Initial GL share group: " << CGLGetShareGroup(CGLGetCurrentContext()) << std::endl;

SimpleWindow::mGLContext = context;


        glContext = (unsigned long *)appleContext;
//CGLSetCurrentContext(appleGLContext);
std::cout << "the device manager created the GL context " << glContext << std::endl;
#else
#if _WIN32
        // TODO implement windows OpenGL stuff
        // http://msdn.microsoft.com/en-us/library/windows/desktop/dd374379%28v=vs.85%29.aspx
        //HDC    hdc = oul::getHDC();
		//std::cout << "HDC in DeviceManager is: " << hdc << std::endl;
        HGLRC  hglrc; 

		SimpleWindow::initializeQtApp();

		// Need a drawable for this to work
		QGLWidget* widget = new QGLWidget;
		widget->show();

		widget->hide(); // TODO should probably delete widget as well
		std::cout << "created a drawable" << std::endl;
		HDC    hdc = wglGetCurrentDC();
		std::cout << "HDC in DeviceManager is: " << hdc << std::endl;
         
        // create a rendering context  
        hglrc = wglCreateContext (hdc); 
		std::cout << "GL context in DeviceManager is: " << hglrc << std::endl;
        glContext = (unsigned long *)hglrc;
         
        // make it the calling thread's current rendering context 
        bool success = wglMakeCurrent (hdc, hglrc);
		if(!success)
			throw Exception("Failed to set initial windows GL context");
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
        std::cout << "created GLX context " << glContext << std::endl;
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
        OpenCLDevice * device = new OpenCLDevice(deviceVector, glContext);
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
