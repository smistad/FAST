#pragma once

#include <string>
#include <map>
#include <FAST/OpenCL.hpp>
#include "RuntimeMeasurement.hpp"
#include <chrono>
#include <memory>


namespace fast {

/**
 * @brief Manages multiple runtime measurements
 *
 * @sa RuntimeMeasurement
 */
class FAST_EXPORT  RuntimeMeasurementsManager : public Object {
	FAST_OBJECT(RuntimeMeasurementsManager)
public:
    void setWarmupRounds(int rounds);
	int getWarmupRounds() const;
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

	RuntimeMeasurement::pointer getTiming(std::string name);

	void print(std::string name);
	void printAll();

private:
	RuntimeMeasurementsManager();
	bool enabled;
	int m_warmupRounds = 0;
	std::map<std::string, RuntimeMeasurement::pointer> timings;
	std::map<std::string, unsigned int> numberings;
	std::map<std::string, cl::Event> startEvents;
	std::map<std::string, std::chrono::system_clock::time_point> startTimes;
};

} //namespace fast
