#pragma once

#include <string>
#include <memory>
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
	RuntimeMeasurement(std::string name);
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
	std::string mName;
};

}; // end namespace
