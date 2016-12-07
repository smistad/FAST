#include "RuntimeMeasurement.hpp"
#include <iostream>
#include <sstream>

namespace fast {

RuntimeMeasurement::RuntimeMeasurement(){
    mSum = 0.0;
	mSamples = 0;
	mRunningMean = 0.0;
    mRunningVariance = 0.0;
}

RuntimeMeasurement::RuntimeMeasurement(std::string name) {
	mSum = 0.0;
	mSamples = 0;
	mRunningMean = 0.0;
    mRunningVariance = 0.0;
	this->mName = name;
}

void RuntimeMeasurement::addSample(double runtime) {
	mSamples++;
	mSum += runtime;
    if(mSamples > 1) {
		const double delta = runtime - mRunningMean;
		mRunningMean += delta / mSamples;
		const double delta2 = runtime - mRunningMean;
		mRunningVariance += delta * delta2;
        if(runtime < mMin)
			mMin = runtime;
		if(runtime > mMax)
			mMax = runtime;
	} else {
		mRunningMean = runtime;
        mMin = runtime;
		mMax = runtime;
	}
}

std::string RuntimeMeasurement::print() const {
	
	std::stringstream buffer;

    buffer << std::endl;
	buffer << "Runtime of " << mName << std::endl;
	buffer << "----------------------------------------------------" << std::endl;
	if (mSamples == 0) {
		buffer << "None recorded." << std::endl;
	} else if (mSamples == 1) {
		buffer << mSum << " ms" << std::endl;
	} else {
		buffer << "Total: " << getSum() << " ms" << std::endl;
		buffer << "Average: " << getAverage() << " ms" << std::endl;
		buffer << "Standard deviation: " << getStdDeviation() << " ms" << std::endl;
		buffer << "Minimum: " << mMin << " ms" << std::endl;
		buffer << "Maximum: " << mMax << " ms" << std::endl;
		buffer << "Number of samples: " << mSamples << std::endl;
	}
	buffer << "----------------------------------------------------" << std::endl;

	std::cout << buffer.str();
	return buffer.str();

}

double RuntimeMeasurement::getSum() const {
	return mSum;
}

double RuntimeMeasurement::getAverage() const {
	return mRunningMean;
}

double RuntimeMeasurement::getStdDeviation() const {
    return sqrt(mRunningVariance / mSamples);
}

unsigned int RuntimeMeasurement::getSamples() const {
	return mSamples;
}

double RuntimeMeasurement::getMax() const {
	return mMax;
}

double RuntimeMeasurement::getMin() const {
	return mMin;
}

} // end namespace fast
