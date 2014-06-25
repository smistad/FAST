#ifndef TESTFIXTURE_HPP_
#define TESTFIXTURE_HPP_

#include "OpenCLManager.hpp"
#include "CL/OpenCL.hpp"
#include "DeviceCriteria.hpp"
#include "Context.hpp"

namespace oul
{
/**
 * \brief A collection of functionality for running tests on OpenCL
 *
 * \date Jan 24, 2014
 * \author Janne Beate Bakeng, SINTEF
 */

class TestFixture
{
public:
	TestFixture();
	~TestFixture();

	static DeviceCriteria getDefaultDeviceCriteria();
	static DeviceCriteria getCPUDeviceCriteria();
	static DeviceCriteria getGPUDeviceCriteria();

	//will not throw exceptions
	std::string getTestCode();
	oul::Context getContext();

	std::vector<oul::PlatformDevices> getDevices(oul::DeviceType type);
	std::vector<oul::PlatformDevices> getCPUDevices();
	std::vector<oul::PlatformDevices> getGPUDevices();
	std::vector<oul::PlatformDevices> getAllDevices();

	bool isCPUDeviceAvailable();
	bool isGPUDeviceAvailable();
	bool isAnyDeviceAvailable();

	//will throw exceptions if they fail
	void canReadTestKernelFile();
	void canRunCodeFromString(oul::ContextPtr context, std::string source, std::string kernel_name);
	void canRunCodeFromFile(oul::ContextPtr context, std::string kernel_name);
	void canRunProgramOnQueue(cl::Program program, cl::CommandQueue queue, std::string kernel_name);
	void canWriteToBufferAndReadItBack();

private:
	std::string test_kernels;
};

} /* namespace oul */

#endif /* TESTFIXTURE_HPP_ */
