#ifndef RUNTIMEMEASUREMENTMANAGER_HPP_
#define RUNTIMEMEASUREMENTMANAGER_HPP_

#include <string>
#include <map>
#include "CL/OpenCL.hpp"
#include "RuntimeMeasurement.hpp"
#include <boost/chrono.hpp>

namespace fast {

class RuntimeMeasurementsManager {

public:
	RuntimeMeasurementsManager();

	void enable();
	void disable();
	bool isEnabled();

	void startCLTimer(std::string name, cl::CommandQueue queue);
	void stopCLTimer(std::string name, cl::CommandQueue queue);

	void startRegularTimer(std::string name);
	void stopRegularTimer(std::string name);

	void startNumberedCLTimer(std::string name, cl::CommandQueue queue);
	void stopNumberedCLTimer(std::string name, cl::CommandQueue queue);

	void startNumberedRegularTimer(std::string name);
	void stopNumberedRegularTimer(std::string name);

	RuntimeMeasurementPtr getTiming(std::string name);

	void print(std::string name);
	void printAll();

private:
	bool enabled;
	std::map<std::string, RuntimeMeasurementPtr> timings;
	std::map<std::string, unsigned int> numberings;
	std::map<std::string, cl::Event> startEvents;
	//std::map<std::string, boost::chrono::system_clock::time_point> startTimes;
    std::map<std::string, boost::chrono::high_resolution_clock::time_point> startTimes;
};

typedef boost::shared_ptr<class RuntimeMeasurementsManager> RuntimeMeasurementsManagerPtr;

} //namespace fast

#endif /* RUNTIMEMEASUREMENTMANAGER_HPP_ */
