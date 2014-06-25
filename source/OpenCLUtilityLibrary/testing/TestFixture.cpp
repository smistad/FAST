#include "TestFixture.hpp"

#include <iostream>
#include "CL/OpenCL.hpp"
#include "OpenCLManager.hpp"
#include "OulConfig.hpp"
#include "HelperFunctions.hpp"

namespace oul
{

TestFixture::TestFixture(){
	opencl();
	test_kernels = std::string(TEST_DIR)+"/TestKernels.cl";
}

TestFixture::~TestFixture(){
	opencl()->shutdown();
}

DeviceCriteria TestFixture::getDefaultDeviceCriteria(){
	return DeviceCriteria();
}

DeviceCriteria TestFixture::getCPUDeviceCriteria(){
	DeviceCriteria retval = TestFixture::getDefaultDeviceCriteria();
	retval.setTypeCriteria(DEVICE_TYPE_CPU);
	return retval;
}

DeviceCriteria TestFixture::getGPUDeviceCriteria(){
	DeviceCriteria retval = TestFixture::getDefaultDeviceCriteria();
	retval.setTypeCriteria(DEVICE_TYPE_GPU);
	return retval;
}

std::string TestFixture::getTestCode(){
    return "__kernel void test(void){size_t id = get_global_id(0); id++;}";
}

std::vector<oul::PlatformDevices> TestFixture::getDevices(oul::DeviceType type){
    oul::DeviceCriteria criteria;
    criteria.setTypeCriteria(type);

    return oul::opencl()->getDevices(criteria);
}

std::vector<oul::PlatformDevices> TestFixture::getCPUDevices(){
    return getDevices(oul::DEVICE_TYPE_CPU);
}

std::vector<oul::PlatformDevices> TestFixture::getGPUDevices(){
    return getDevices(oul::DEVICE_TYPE_GPU);
}

std::vector<oul::PlatformDevices> TestFixture::getAllDevices(){
    return getDevices(oul::DEVICE_TYPE_ANY);
}

bool TestFixture::isCPUDeviceAvailable(){
    return (this->getCPUDevices().size() != 0);
}

bool TestFixture::isGPUDeviceAvailable(){
    return (this->getGPUDevices().size() != 0);
}

bool TestFixture::isAnyDeviceAvailable(){
    return (this->getAllDevices().size() != 0);
}

void TestFixture::canReadTestKernelFile(){
    oul::readFile(test_kernels);
}

void TestFixture::canRunCodeFromString(oul::ContextPtr context, std::string source, std::string kernel_name){
    context->createProgramFromString(source);
    this->canRunProgramOnQueue(context->getProgram(0), context->getQueue(0), kernel_name);
}

void TestFixture::canRunCodeFromFile(oul::ContextPtr context, std::string kernel_name){
    context->createProgramFromSource(test_kernels);
    this->canRunProgramOnQueue(context->getProgram(0), context->getQueue(0), kernel_name);
}

void TestFixture::canRunProgramOnQueue(cl::Program program, cl::CommandQueue queue, std::string kernel_name){
    cl::Kernel kernel(program, kernel_name.c_str());
    queue.enqueueTask(kernel);
    queue.finish();
}

void TestFixture::canWriteToBufferAndReadItBack(){
	oul::ContextPtr context = oul::opencl()->createContextPtr(getGPUDeviceCriteria());

	char buffer_input_data[] = "data to fill the buffer with";
	const int size = sizeof(buffer_input_data);
	char buffer_output_data[size];

	cl::Buffer buffer = context->createBuffer(context->getContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size, buffer_input_data, "canWriteToBufferAndReadItBack");

	int queueNumber = 0;
	context->getQueue(queueNumber).enqueueWriteBuffer(buffer, CL_TRUE, 0, size, buffer_input_data, 0, 0);
	context->getQueue(queueNumber).enqueueReadBuffer(buffer, CL_TRUE, 0, size, buffer_output_data, 0, 0);

	std::string input_string(buffer_input_data);
	std::string output_string(buffer_output_data);
	std::string error_msg = "Did not get back the same data as was written to the buffer";
	if(input_string != output_string)
	 throw oul::Exception(error_msg.c_str());
}


} /* namespace oul */
