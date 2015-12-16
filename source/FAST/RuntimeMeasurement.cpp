#include "RuntimeMeasurement.hpp"
#include <iostream>
#include <sstream>

namespace fast {

RuntimeMeasurement::RuntimeMeasurement(){}

RuntimeMeasurement::RuntimeMeasurement(std::string name) {
    last = 0.0f;
    sum = 0.0f;
    slidingAvg = 0.0f;
    slidingAlpha = 0.2f;
	samples = 0;
	this->name = name;
}

void RuntimeMeasurement::addSample(double runtime) {
	samples++;
    last = runtime;
    if (slidingAvg == 0.0f) slidingAvg = runtime;
    else slidingAvg = slidingAlpha*runtime + (1 - slidingAlpha)*slidingAvg;
	sum += runtime;
}

std::string RuntimeMeasurement::print() const {
	
	std::stringstream buffer;

	buffer << "Runtime of " << name << std::endl;
	buffer << "----------------------------------------------------" << std::endl;
	if (samples == 0) {
		buffer << "None recorded." << std::endl;
	} else if (samples == 1) {
		buffer << sum << " ms" << std::endl;
	} else {
        buffer << "Last: " << last << " ms" << std::endl;
        buffer << "Sliding average: " << slidingAvg << " ms" << std::endl;
		buffer << "Total: " << sum << " ms" << std::endl;
		buffer << "Average: " << sum / samples << " ms" << std::endl;
		buffer << "Standard deviation: " << "TODO" << " ms" << std::endl; //TODO
		buffer << "Number of samples: " << samples << std::endl;
	}
	buffer << "----------------------------------------------------" << std::endl;

	std::cout << buffer.str();
	return buffer.str();

}

double RuntimeMeasurement::getLast() const {
    return last;
}

double RuntimeMeasurement::getSlidingAverage() const {
    return slidingAvg;
}

double RuntimeMeasurement::getSum() const {
	return sum;
}

double RuntimeMeasurement::getAverage() const {
	return sum / samples;
}

double RuntimeMeasurement::getStdDeviation() const {
	// TODO: implement
    return 0.0;
}

} // end namespace fast
