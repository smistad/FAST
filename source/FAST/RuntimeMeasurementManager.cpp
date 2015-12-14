#include "RuntimeMeasurementManager.hpp"
#include "Exception.hpp"

namespace fast {

void RuntimeMeasurementsManager::enable() {
	enabled = true;
}

void RuntimeMeasurementsManager::disable() {
	enabled = false;
}

void RuntimeMeasurementsManager::startCLTimer(std::string name, cl::CommandQueue queue) {
	if (!enabled)
		return;

	if (queue.getInfo<CL_QUEUE_PROPERTIES>() != CL_QUEUE_PROFILING_ENABLE) {
		throw Exception(
				"Failed to get profiling info. Make sure that RuntimeMeasurementManager::enable() is called before the OpenCL context is created.",
				__LINE__, __FILE__);
	}
	cl::Event startEvent;
#if !defined(CL_VERSION_1_2) || defined(CL_USE_DEPRECATED_OPENCL_1_1_APIS)
	// Use deprecated API
	queue.enqueueMarker(&startEvent);
#else
	queue.enqueueMarkerWithWaitList(NULL, &startEvent);
#endif
	queue.finish();
	startEvents.insert(std::make_pair(name, startEvent));
}

void RuntimeMeasurementsManager::stopCLTimer(std::string name, cl::CommandQueue queue) {
	if (!enabled)
		return;

	if (queue.getInfo<CL_QUEUE_PROPERTIES>() != CL_QUEUE_PROFILING_ENABLE) {
		throw Exception(
				"Failed to get profiling info. Make sure that RuntimeMeasurementManager::enable() is called before the OpenCL context is created.",
				__LINE__, __FILE__);
	}

	// check that the startEvent actually exist
	if (startEvents.count(name) == 0) {
		throw Exception("Unknown CL timer");
	}
	cl_ulong start, end;
	cl::Event endEvent;
#if !defined(CL_VERSION_1_2) || defined(CL_USE_DEPRECATED_OPENCL_1_1_APIS)
	// Use deprecated API
	queue.enqueueMarker(&endEvent);
#else
	queue.enqueueMarkerWithWaitList(NULL, &endEvent);
#endif
	queue.finish();
	cl::Event startEvent = startEvents[name];
	startEvent.getProfilingInfo<cl_ulong>(CL_PROFILING_COMMAND_START, &start);
	endEvent.getProfilingInfo<cl_ulong>(CL_PROFILING_COMMAND_START, &end);
	if (timings.count(name) == 0) {
		// No timings with this name exists, create a new one
		RuntimeMeasurementPtr runtime(new RuntimeMeasurement(name));
		runtime->addSample((end - start) * 1.0e-6);
		timings[name] =  runtime;
	} else {
		timings[name]->addSample((end - start) * 1.0e-6);
	}

	// Remove the start event
	startEvents.erase(name);
}
typedef boost::chrono::high_resolution_clock boostTimerClock_test;
//typedef boost::chrono::system_clock boostTimerClock_test;
void RuntimeMeasurementsManager::startRegularTimer(std::string name) {
	if (!enabled)
		return;

	//startTimes[name] = boost::chrono::system_clock::now();
    startTimes[name] = boostTimerClock_test::now();
}

void RuntimeMeasurementsManager::stopRegularTimer(std::string name) {
	if (!enabled)
		return;

	if(startTimes.count(name) == 0)
	    return;

	//boost::chrono::duration<double, boost::milli> time = boost::chrono::system_clock::now() - startTimes[name];
    boost::chrono::duration<double, boost::milli> time = boostTimerClock_test::now() - startTimes[name];
    if (timings.count(name) == 0) {
		// No timings with this name exists, create a new one
		RuntimeMeasurementPtr runtime(new RuntimeMeasurement(name));

		runtime->addSample(time.count());
		timings[name] =  runtime;
	} else {
		timings[name]->addSample(time.count());
	}

    startTimes.erase(name);
}

void RuntimeMeasurementsManager::startNumberedCLTimer(std::string name, cl::CommandQueue queue) {
	if (!enabled)
		return;
}

void RuntimeMeasurementsManager::stopNumberedCLTimer(std::string name, cl::CommandQueue queue) {
	if (!enabled)
		return;
}

void RuntimeMeasurementsManager::startNumberedRegularTimer(std::string name) {
	if (!enabled)
		return;
}

void RuntimeMeasurementsManager::stopNumberedRegularTimer(std::string name) {
	if (!enabled)
		return;
}

RuntimeMeasurementPtr RuntimeMeasurementsManager::getTiming(std::string name) {
    if(timings.count(name) == 0) {
        // Create a new empty timing
		RuntimeMeasurementPtr runtime(new RuntimeMeasurement(name));
		timings[name] = runtime;
    }

	return timings[name];
}

void RuntimeMeasurementsManager::print(std::string name) {
	if (!enabled)
		return;

	timings[name]->print();
}

void RuntimeMeasurementsManager::printAll() {
	if (!enabled)
		return;

	std::map<std::string, RuntimeMeasurementPtr>::iterator it;
	for (it = timings.begin(); it != timings.end(); it++) {
		it->second->print();
	}
}

RuntimeMeasurementsManager::RuntimeMeasurementsManager() {
    enabled = false;
}

bool RuntimeMeasurementsManager::isEnabled() {
	return enabled;
}

} //namespace fast
