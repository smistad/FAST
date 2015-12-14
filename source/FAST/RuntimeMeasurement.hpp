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
    double getLast() const;
    double getSlidingAverage() const;
	double getSum() const;
	double getAverage() const;
	double getStdDeviation() const;
	std::string print() const;
	virtual ~RuntimeMeasurement() {};

private:
	RuntimeMeasurement();

    double last;
    double slidingAvg;
    double slidingAlpha;
	double sum;
	unsigned int samples;
	std::string name;
};

typedef boost::shared_ptr<class RuntimeMeasurement> RuntimeMeasurementPtr;

}
;
// end namespace

#endif /* TIMING_HPP_ */
