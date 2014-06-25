#include "Context.hpp"

#include <iostream>
#include "HelperFunctions.hpp"
#include "RuntimeMeasurement.hpp"
#include "OpenCLManager.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#endif
#endif

namespace oul
{

void CL_CALLBACK memoryDestructorCallback(cl_mem memobj, void* user_data)
{
	std::string* data_pointer = static_cast<std::string*>(user_data);
	std::string data = *data_pointer;
	Reporter r;
	r.report("Memory destructor callback: " + data, oul::INFO);
}

void CL_CALLBACK contextCallback(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	Reporter r;
	r.report("Context callback:\n " + std::string(errinfo), oul::ERROR);
}

Context::Context() {
	//TODO make private or implement properly
	reporter.report("[!!!WARNING!!!] Calling default oul::Context constructor, object is not correctly instantiated, make this private!", oul::WARNING);
}

Context::Context(std::vector<cl::Device> devices, unsigned long * OpenGLContext, bool enableProfiling) :
		profilingEnabled(enableProfiling),
		runtimeManager(new RuntimeMeasurementsManager())
	{
	if(profilingEnabled)
		runtimeManager->enable();
	else
		runtimeManager->disable();

    this->garbageCollector = GarbageCollectorPtr(new GarbageCollector);

    this->devices = devices;
    // TODO: make sure that all devices have the same platform
    this->platform = devices[0].getInfo<CL_DEVICE_PLATFORM>();

    // TODO: OpenGL interop properties
    // TODO: must check that a OpenGL context and display is available
    // TODO: Use current context and display, or take this as input??
    cl_context_properties * cps;
    if(OpenGLContext != NULL) {
#if defined(__APPLE__) || defined(__MACOSX)
        cps = createInteropContextProperties(
                this->platform,
                (cl_context_properties)OpenGLContext,
                NULL
        );
#else
#ifdef _WIN32
		HDC hdc = oul::getHDC();
        cps = createInteropContextProperties(
                this->platform,
                (cl_context_properties)OpenGLContext,
                (cl_context_properties)hdc
        );
#else
        std::cout << "current glX context is " << OpenGLContext << std::endl;
        Display * display = XOpenDisplay(0);
        std::cout << "current display is " << display << std::endl;
        cps = createInteropContextProperties(
                this->platform,
                (cl_context_properties)OpenGLContext,
                (cl_context_properties)display
        );
#endif
#endif
    } else {
        cps = new cl_context_properties[3];
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties)(platform)();
        cps[2] = 0;
    }
    this->context = cl::Context(devices,cps,contextCallback);
    delete[] cps;

    // Create a command queue for each device
    for(int i = 0; i < devices.size(); i++) {
        if(profilingEnabled) {
            this->queues.push_back(cl::CommandQueue(context, devices[i], CL_QUEUE_PROFILING_ENABLE));
        } else {
            this->queues.push_back(cl::CommandQueue(context, devices[i]));
        }
    }
}

int Context::createProgramFromSource(std::string filename, std::string buildOptions) {
    std::string sourceCode = readFile(filename);

    cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));
    cl::Program program = buildSources(source, buildOptions);
    programs.push_back(program);
    return programs.size()-1;
}

/**
 * Compile several source files together
 */
int Context::createProgramFromSource(std::vector<std::string> filenames, std::string buildOptions) {
    // Do this in a weird way, because the the logical way does not work.
    std::string sourceCode = readFile(filenames[0]);
    cl::Program::Sources sources(filenames.size(), std::make_pair(sourceCode.c_str(), sourceCode.length()));
    for(int i = 1; i < filenames.size(); i++) {
        std::string sourceCode2 = readFile(filenames[i]);
        sources[i] = std::make_pair(sourceCode2.c_str(), sourceCode2.length());
    }

    cl::Program program = buildSources(sources, buildOptions);
    programs.push_back(program);
    return programs.size()-1;
}

int Context::createProgramFromString(std::string code, std::string buildOptions) {
    cl::Program::Sources source(1, std::make_pair(code.c_str(), code.length()));

    cl::Program program = buildSources(source, buildOptions);
    programs.push_back(program);
    return programs.size()-1;
}

cl::Program Context::getProgram(unsigned int i) {
    return programs[i];
}

cl::CommandQueue Context::getQueue(unsigned int i) {
    return queues[i];
}


cl::Device Context::getDevice(unsigned int i) {
    return devices[i];
}

cl::Device getDevice(cl::CommandQueue queue){
	return queue.getInfo<CL_QUEUE_DEVICE>();
}

cl::Context Context::getContext() {
    return context;
}

cl::Platform Context::getPlatform() {
    return platform;
}

GarbageCollector* Context::getGarbageCollector() {
	GarbageCollector* retval = garbageCollector.get();
    return retval;
}

GarbageCollectorPtr Context::getGarbageCollectorPtr(){
	return garbageCollector;
}

cl::Program Context::buildSources(cl::Program::Sources source, std::string buildOptions) {
    // Make program of the source code in the context
    cl::Program program = cl::Program(context, source);

    // Build program for the context devices
    try{
        program.build(devices, buildOptions.c_str());
        programs.push_back(program);
    } catch(cl::Error &error) {
        if(error.err() == CL_BUILD_PROGRAM_FAILURE) {
            for(unsigned int i=0; i<devices.size(); i++){
            	reporter.report("Build log, device "+oul::number(i)+ "\n"+ program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]), oul::ERROR);
            }
        }
        reporter.report(getCLErrorString(error.err()), oul::ERROR);

        throw error;
    }
    return program;
}

RuntimeMeasurementsManagerPtr Context::getRunTimeMeasurementManager(){
	return runtimeManager;
}


int Context::createProgramFromBinary(std::string filename, std::string buildOptions) {
    //TODO todo
	return 0;
}

int Context::createProgramFromSourceWithName(
        std::string programName,
        std::string filename,
        std::string buildOptions) {
    programNames[programName] = createProgramFromSource(filename,buildOptions);
    return programNames[programName];
}

int Context::createProgramFromSourceWithName(
        std::string programName,
        std::vector<std::string> filenames,
        std::string buildOptions) {
    programNames[programName] = createProgramFromSource(filenames,buildOptions);
    return programNames[programName];
}

int Context::createProgramFromStringWithName(
        std::string programName,
        std::string code,
        std::string buildOptions) {
    programNames[programName] = createProgramFromString(code,buildOptions);
    return programNames[programName];
}

int Context::createProgramFromBinaryWithName(
        std::string programName,
        std::string filename,
        std::string buildOptions) {
    programNames[programName] = createProgramFromBinary(filename,buildOptions);
    return programNames[programName];
}

cl::Program Context::getProgram(std::string name) {
    if(programNames.count(name) == 0) {
        std::string msg ="Could not find OpenCL program with the name" + name;
        throw Exception(msg.c_str(), __LINE__, __FILE__);
    }

    return programs[programNames[name]];
}

bool Context::hasProgram(std::string name) {
    return programNames.count(name) > 0;
}

cl::Kernel Context::createKernel(cl::Program program, std::string kernel_name)
{
	cl::Kernel kernel;

	try
	{
		kernel = cl::Kernel(program, kernel_name.c_str(), NULL);
		reporter.report("Created kernel with name "+std::string(kernel_name), oul::INFO);
	}
	catch(cl::Error &error)
	{
		reporter.report("Could not create kernel. Reason:"+std::string(error.what()), oul::ERROR);
		reporter.report(getCLErrorString(error.err()), oul::ERROR);
		throw;
	}

	return kernel;
}

void Context::executeKernel(cl::CommandQueue queue, cl::Kernel kernel, size_t global_work_size, size_t local_work_size)
{
	reporter.report("Executing kernel", oul::INFO);
	try
	{
		queue.enqueueNDRangeKernel(kernel, 0, global_work_size, local_work_size, NULL, NULL);
		queue.finish();
	} catch (cl::Error &error)
	{
		reporter.report("Could not execute kernel(s). Reason: "+std::string(error.what()), oul::ERROR);
		reporter.report(getCLErrorString(error.err()), oul::ERROR);
		throw;
	}
}

cl::Buffer Context::createBuffer(cl::Context context, cl_mem_flags flags, size_t size, void * host_data, std::string bufferName)
{
	cl::Buffer dev_mem;
	try
	{
		if (host_data != NULL)
			flags |= CL_MEM_COPY_HOST_PTR;
		dev_mem = cl::Buffer(context, flags, size, host_data, NULL);
		dev_mem.setDestructorCallback(memoryDestructorCallback, static_cast<void*>(new std::string(bufferName)));
	} catch (cl::Error &error)
	{
		reporter.report("Could not create a OpenCL buffer queue. Reason: "+std::string(error.what()), oul::ERROR);
		reporter.report(getCLErrorString(error.err()), oul::ERROR);
		throw;
	}
	return dev_mem;
}

void Context::readBuffer(cl::CommandQueue queue, cl::Buffer outputBuffer, size_t outputVolumeSize, void *outputData)
{
	try
	{
		queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, outputVolumeSize, outputData, 0, 0);
	} catch (cl::Error &error)
	{
		reporter.report("Could not read output volume buffer from OpenCL. Reason: "+std::string(error.what()), oul::ERROR);
		reporter.report(getCLErrorString(error.err()), oul::ERROR);
		throw;
	}
}

} //namespace oul

