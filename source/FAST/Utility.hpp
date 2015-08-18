#ifndef UTILITY_HPP_
#define UTILITY_HPP_
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/DataTypes.hpp"

// This file contains a set of utility functions

// Undefine windows crap
#undef min
#undef max

namespace fast {

double log2(double n);
double round(double n);
int pow(int a, int b);

template<class T>
T min(T a, T b) {
    return a < b ? a : b;
}

template<class T>
T max(T a, T b) {
    return a > b ? a : b;
}

template<class T>
T sign(T value) {
    if(value > 0) {
        return 1;
    } else if (value < 0) {
        return -1;
    } else {
        return 0;
    }
}

unsigned int getPowerOfTwoSize(unsigned int size);
void* allocateDataArray(unsigned int voxels, DataType type, unsigned int nrOfComponents);
template <class T>
float getSumFromOpenCLImageResult(void* voidData, unsigned int size, unsigned int nrOfComponents) {
    T* data = (T*)voidData;
    float sum = 0.0f;
    for(unsigned int i = 0; i < size*nrOfComponents; i += nrOfComponents) {
        sum += data[i];
    }
    return sum;
}

void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* min, float* max);
void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image3D image, DataType type, float* min, float* max);
void getMaxAndMinFromOpenCLBuffer(OpenCLDevice::pointer device, cl::Buffer buffer, unsigned int size, DataType type, float* min, float* max);
void getIntensitySumFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* sum);

template <class T>
void getMaxAndMinFromData(void* voidData, unsigned int nrOfElements, float* min, float* max) {
    T* data = (T*)voidData;

    *min = std::numeric_limits<float>::max();
    *max = std::numeric_limits<float>::min();
    for(unsigned int i = 0; i < nrOfElements; i++) {
        if((float)data[i] < *min) {
            *min = (float)data[i];
        }
        if((float)data[i] > *max) {
            *max = (float)data[i];
        }
    }
}

template <class T>
float getSumFromData(void* voidData, unsigned int nrOfElements) {
    T* data = (T*)voidData;

    float sum = 0.0f;
    for(unsigned int i = 0; i < nrOfElements; i++) {
        sum += (float)data[i];
    }
    return sum;
}

} // end namespace fast


#endif /* UTILITY_HPP_ */
