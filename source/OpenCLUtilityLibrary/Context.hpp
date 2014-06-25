#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include "CL/OpenCL.hpp"
#include <vector>
#include <map>
#include "Exceptions.hpp"
#include "GarbageCollector.hpp"
#include "Reporter.hpp"
#include "RuntimeMeasurementManager.hpp"

namespace oul {

/**
 * This class holds an OpenCL context, with all of its queues and devices.
 * Its main purpose is to be a class that can't be sent between different 
 * functions and objects that does OpenCL processing.
 */
class Context {

public:
	Context();
	Context(std::vector<cl::Device> devices, unsigned long * OpenGLContext, bool enableProfiling = false);

	int createProgramFromSource(std::string filename, std::string buildOptions = "");
	int createProgramFromSource(std::vector<std::string> filenames, std::string buildOptions = "");
	int createProgramFromString(std::string code, std::string buildOptions = "");
	int createProgramFromBinary(std::string filename, std::string buildOptions = "");
	int createProgramFromSourceWithName(std::string programName, std::string filename, std::string buildOptions = "");
	int createProgramFromSourceWithName(std::string programName, std::vector<std::string> filenames, std::string buildOptions = "");
	int createProgramFromStringWithName(std::string programName, std::string code, std::string buildOptions = "");
	int createProgramFromBinaryWithName(std::string programName, std::string filename, std::string buildOptions = "");
	cl::Program getProgram(unsigned int i);
	cl::Program getProgram(std::string name);
	bool hasProgram(std::string name);

	cl::Kernel createKernel(cl::Program program, std::string kernel_name); //can throw cl::Error
	void executeKernel(cl::CommandQueue queue, cl::Kernel kernel, size_t global_work_size, size_t local_work_size); //can throw cl::Error

	cl::Buffer createBuffer(cl::Context context, cl_mem_flags flags, size_t size, void * host_data, std::string bufferName); //can throw cl::Error
	void readBuffer(cl::CommandQueue queue, cl::Buffer outputBuffer, size_t outputVolumeSize, void *outputData); //can throw cl::Error

	cl::CommandQueue getQueue(unsigned int i);
	cl::Device getDevice(unsigned int i);
	cl::Device getDevice(cl::CommandQueue queue);
	cl::Context getContext();
	cl::Platform getPlatform();

	GarbageCollector * getGarbageCollector(); //DEPRECATED, DON'T USE
	GarbageCollectorPtr getGarbageCollectorPtr();

	RuntimeMeasurementsManagerPtr getRunTimeMeasurementManager();

private:
	cl::Program buildSources(cl::Program::Sources source, std::string buildOptions);

	Reporter reporter;
	cl::Context context;
	std::vector<cl::CommandQueue> queues;
	std::map<std::string, int> programNames;
	std::vector<cl::Program> programs;
	std::vector<cl::Device> devices;
	cl::Platform platform;
	GarbageCollectorPtr garbageCollector;

	bool profilingEnabled;
	RuntimeMeasurementsManagerPtr runtimeManager;
};

typedef boost::shared_ptr<class Context> ContextPtr;

};

#endif /* CONTEXT_HPP_ */
