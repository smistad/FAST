#pragma once

#include <string>
#include <memory>
#include <deque>
#include "FAST/Object.hpp"

namespace fast {
/**
 * @brief A class for runtime measurement
 *
 * Collect multiple runtimes over time, and calculates running average, running standard deviation,
 * sum, max, min etc.
 *
 * All measurements are in milliseconds
 *
 */
class FAST_EXPORT  RuntimeMeasurement : public Object {
public:
	typedef std::shared_ptr<RuntimeMeasurement> pointer;
	RuntimeMeasurement(std::string name, int warmupRounds = 0, int maximumSamples = -1);
	void addSample(double runtime);
	double getSum() const;
	double getAverage() const;
    unsigned int getSamples() const;
	double getMax() const;
	double getMin() const;
	double getStdDeviation() const;
	std::string print() const;
	void reset();
	~RuntimeMeasurement() override = default;
private:
	RuntimeMeasurement();

	double mSum;
	unsigned int mSamples;
	double mRunningVariance;
	double mRunningMean;
	double mMin;
	double mMax;
	double mFirstSample;
	std::string mName;
	int m_warmupRounds;
	int m_maximumSamples;
    std::deque<double> m_queueRuntime;
	std::deque<double> m_queueAvg;
    std::deque<double> m_queueStd;
};

}; // end namespace
