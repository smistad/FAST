#include "RuntimeMeasurement.hpp"
#include <iostream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <numeric>
#include <algorithm>

namespace fast {

RuntimeMeasurement::RuntimeMeasurement(){
    mSum = 0.0;
	mSamples = 0;
	mRunningMean = 0.0;
    mRunningVariance = 0.0;
}

RuntimeMeasurement::RuntimeMeasurement(std::string name, int warmupRounds, int maximumSamples) {
	mSum = 0.0;
	mSamples = 0;
	mRunningMean = 0.0;
    mRunningVariance = 0.0;
	this->mName = name;
	if(warmupRounds < 0)
		throw Exception("Warmup rounds must be > 0");
	m_warmupRounds = warmupRounds;
	m_maximumSamples = maximumSamples;
}

void RuntimeMeasurement::addSample(double runtime) {
	if(m_warmupRounds > 0) {
		--m_warmupRounds;
		return;
	}
	mSamples++;
	mSum += runtime;
    if(mSamples > 1) {

		if(m_maximumSamples > 0) {
		    // TODO should probably mutex protect all of this
            m_queueRuntime.push_back(runtime);
            mSum = std::accumulate(m_queueRuntime.begin(), m_queueRuntime.end(), 0.0);
            mRunningMean = mSum / m_queueRuntime.size();
            mRunningVariance = 0.0; // TODO
            const auto [min, max] = std::minmax_element(m_queueRuntime.begin(), m_queueRuntime.end());
            mMin = *min;
            mMax = *max;
		} else {
            const double delta = runtime - mRunningMean;
            mRunningMean += delta / mSamples;
            const double delta2 = runtime - mRunningMean;
            mRunningVariance += delta * delta2;
            if(runtime < mMin)
                mMin = runtime;
            if(runtime > mMax)
                mMax = runtime;
		}
        if(m_maximumSamples == mSamples) {
            m_queueRuntime.pop_front();
            mSamples--;
        }

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
    return std::sqrt(mRunningVariance / mSamples);
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

void RuntimeMeasurement::reset() {
    mSum = 0.0;
    mSamples = 0;
    mRunningMean = 0.0;
    mRunningVariance = 0.0;
}

} // end namespace fast
