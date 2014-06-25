#include "catch.hpp"

#include "TestFixture.hpp"
#include "OpenCLManager.hpp"
#include "RuntimeMeasurementManager.hpp"

namespace test
{
TEST_CASE("Can create instance of the manager","[oul][OpenCL]"){
    CHECK(oul::opencl());
}

TEST_CASE("Can create a Context with default DeviceCriteria","[oul][OpenCL]"){
    oul::DeviceCriteria criteria = oul::TestFixture::getDefaultDeviceCriteria();
    CHECK_NOTHROW(oul::ContextPtr context = oul::opencl()->createContextPtr(criteria));
}

TEST_CASE("OpenCL platform(s) installed","[oul][OpenCL]"){
    oul::DeviceCriteria criteria = oul::TestFixture::getDefaultDeviceCriteria();

    CHECK(oul::opencl()->getPlatforms(oul::DEVICE_PLATFORM_ANY).size() != 0);
}

TEST_CASE("OpenCL device(s) available","[oul][OpenCL]"){
    oul::TestFixture fixture;
    CHECK(fixture.isAnyDeviceAvailable());
}

TEST_CASE("OpenCL CPU device(s) available","[oul][OpenCL]"){
    oul::TestFixture fixture;
    CHECK(fixture.isCPUDeviceAvailable());
}

TEST_CASE("OpenCL GPU device(s) available","[oul][OpenCL]"){
    oul::TestFixture fixture;
    CHECK(fixture.isGPUDeviceAvailable());
}

//TODO This test fails on Apple
//Apple functionality not implemented yet
TEST_CASE("At least one OpenCL device has OpenGL interop capability","[oul][OpenCL][OpenGL]"){
    oul::TestFixture fixture;
    bool foundOpenGLInteropCapableDevice  = false;

    std::vector<oul::PlatformDevices> devices = fixture.getAllDevices();
    for(unsigned int i=0; i< devices.size(); i++){
        for(int j = 0; j < devices[i].second.size(); j++) {
            foundOpenGLInteropCapableDevice = foundOpenGLInteropCapableDevice || oul::opencl()->deviceHasOpenGLInteropCapability(devices[i].second[j]);
        }
    }

    CHECK(foundOpenGLInteropCapableDevice);
}

TEST_CASE("Default construction gives expected values","[oul][OpenCL]"){
    oul::TestFixture fixture;
    oul::DeviceCriteria criteria;

    CHECK(criteria.getTypeCriteria() == oul::DEVICE_TYPE_ANY);
    CHECK(criteria.getDeviceCountMinCriteria() == 0);
    CHECK(criteria.getDeviceCountMaxCriteria() == 100);
    CHECK(criteria.getPlatformCriteria() == oul::DEVICE_PLATFORM_ANY);
    CHECK(criteria.getDevicePreference() == oul::DEVICE_PREFERENCE_NONE);
}

//TODO make a better test for the devicePlatformMismatch function...
TEST_CASE("Check for device and platform mismatch","[oul][OpenCL]"){
    oul::TestFixture fixture;
    oul::ContextPtr context = oul::opencl()->createContextPtr(oul::TestFixture::getDefaultDeviceCriteria());
    bool mismatch = oul::opencl()->devicePlatformMismatch(context->getDevice(0), context->getPlatform());

#if defined(__APPLE__) || defined(__MACOSX)
    CHECK(mismatch);
#else
    CHECK_FALSE(mismatch);
#endif
}

TEST_CASE("Can run simple kernel from string","[oul][OpenCL]"){
    oul::TestFixture fixture;
    oul::ContextPtr context = oul::opencl()->createContextPtr(oul::TestFixture::getDefaultDeviceCriteria());
    CHECK_NOTHROW(fixture.canRunCodeFromString(context, fixture.getTestCode(), "test"));
}

TEST_CASE("Can read TestKernels.cl", "[oul]"){
    oul::TestFixture fixture;
    CHECK_NOTHROW(fixture.canReadTestKernelFile());
}

TEST_CASE("Can create a program from file","[oul][OpenCL]"){
    oul::TestFixture fixture;
    oul::ContextPtr context = oul::opencl()->createContextPtr(oul::TestFixture::getDefaultDeviceCriteria());
    CHECK_NOTHROW(fixture.canRunCodeFromFile(context, "test_one"));
}

TEST_CASE("Can initialize OpenCL using GPU", "[oul][OpenCL]"){
	oul::TestFixture fixture;
	oul::DeviceCriteria criteria = oul::TestFixture::getGPUDeviceCriteria();
	CHECK_NOTHROW(oul::opencl()->createContextPtr(criteria););
}

TEST_CASE("Can create a small global OpenCL buffer using GPU context", "[oul][OpenCL]"){
	oul::TestFixture fixture;
	oul::DeviceCriteria criteria = oul::TestFixture::getGPUDeviceCriteria();
	oul::ContextPtr context = oul::opencl()->createContextPtr(criteria);

	size_t size = sizeof(cl_char);
	CHECK_NOTHROW(context->createBuffer(context->getContext(), CL_MEM_READ_WRITE, size, NULL, "global test buffer"));
}

TEST_CASE("Can create a small kernel, build a program and run it on a GPU", "[oul][OpenCL]"){
	oul::TestFixture fixture;
	oul::DeviceCriteria criteria = oul::TestFixture::getGPUDeviceCriteria();
	oul::ContextPtr context = oul::opencl()->createContextPtr(criteria);
	CHECK_NOTHROW(fixture.canRunCodeFromString(context, fixture.getTestCode(), "test"));
}

TEST_CASE("Can write to buffer and read the data back", "[oul][OpenCL]"){
	oul::TestFixture fixture;
	CHECK_NOTHROW(fixture.canWriteToBufferAndReadItBack());
}

TEST_CASE("Can run profiling on simple test code", "[oul][OpenCL][profiling]"){

	oul::TestFixture fixture;
	oul::DeviceCriteria criteria = oul::TestFixture::getGPUDeviceCriteria();
	bool enableProfiling = true;
	oul::ContextPtr context = oul::opencl()->createContextPtr(criteria, NULL, enableProfiling);
	int programID = context->createProgramFromString(fixture.getTestCode(), "");
	cl::Program program = context->getProgram(programID);
	cl::CommandQueue queue = context->getQueue(0); //getQueueForDevice(device)?

	oul::RuntimeMeasurementsManagerPtr runtime = context->getRunTimeMeasurementManager();
	runtime->enable();
	runtime->startCLTimer("time", queue);
	CHECK_NOTHROW(fixture.canRunProgramOnQueue(program, queue, "test"));
	runtime->stopCLTimer("time", queue);

	runtime->printAll();
}



}//namespace test
