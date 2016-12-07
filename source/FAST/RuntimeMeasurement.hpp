#ifndef TIMING_HPP_
#define TIMING_HPP_

#include <string>
#include <stdio.h>
#include <boost/shared_ptr.hpp>

namespace fast {
/**
 * A class for a runtime measurement
 */
class RuntimeMeasurement {

public:
	RuntimeMeasurement(std::string name);
	void addSample(double runtime);
	double getSum() const;
	double getAverage() const;
    unsigned int getSamples() const;
	double getMax() const;
	double getMin() const;
	double getStdDeviation() const;
	std::string print() const;
	virtual ~RuntimeMeasurement() {};

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

typedef boost::shared_ptr<class RuntimeMeasurement> RuntimeMeasurementPtr;

}
;
// end namespace

#endif /* TIMING_HPP_ */
