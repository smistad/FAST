#include "RuntimeMeasurement.hpp"
#include <iostream>

using namespace oul;

RuntimeMeasurement::RuntimeMeasurement(){}

RuntimeMeasurement::RuntimeMeasurement(std::string name) {
	sum = 0.0f;
	samples = 0;
	this->name = name;
}

void RuntimeMeasurement::addSample(double runtime) {
	samples++;
	sum += runtime;
}

void RuntimeMeasurement::print() const {
	std::cout << "Runtime of " << name << std::endl;
	std::cout << "----------------------------------------------------"
			<< std::endl;
	if (samples == 0) {
		std::cout << "None recorded." << std::endl;
	} else if (samples == 1) {
		std::cout << sum << " ms" << std::endl;
	} else {
		std::cout << "Total: " << sum << " ms" << std::endl;
		std::cout << "Average: " << sum / samples << " ms" << std::endl;
		std::cout << "Standard deviation: " << "TODO" << " ms" << std::endl; //TODO
		std::cout << "Number of samples: " << samples << std::endl;
	}
	std::cout << "----------------------------------------------------"
			<< std::endl;
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



