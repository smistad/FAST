#ifndef UTILITY_HPP_
#define UTILITY_HPP_
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/DataTypes.hpp"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// This file contains a set of utility functions

// Undefine windows crap
#undef min
#undef max

namespace fast {

FAST_EXPORT double log2(double n);
FAST_EXPORT double round(double n);
FAST_EXPORT double round(double n, int decimals);

/**
 * Does simply x^2 = x*x
 * @tparam T
 * @param x a numeric value
 * @return x*x
 */
template <class T>
T square(T x) {
    return x*x;
}

template <typename ...Args>
std::string format(std::string format, Args && ... args) {
    auto size = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);
    std::string output(size + 1, '\0');
    std::sprintf(&output[0], format.c_str(), std::forward<Args>(args)...);
    return output;
}

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

FAST_EXPORT unsigned int getPowerOfTwoSize(unsigned int size);
FAST_EXPORT void* allocateDataArray(unsigned int voxels, DataType type, unsigned int nrOfComponents);
template <class T>
float getSumFromOpenCLImageResult(void* voidData, unsigned int size, unsigned int nrOfComponents) {
    T* data = (T*)voidData;
    float sum = 0.0f;
    for(unsigned int i = 0; i < size*nrOfComponents; i += nrOfComponents) {
        sum += data[i];
    }
    return sum;
}

FAST_EXPORT void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* min, float* max);
FAST_EXPORT void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image3D image, DataType type, float* min, float* max);
FAST_EXPORT void getMaxAndMinFromOpenCLBuffer(OpenCLDevice::pointer device, cl::Buffer buffer, unsigned int size, DataType type, float* min, float* max);
FAST_EXPORT void getIntensitySumFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* sum);

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

FAST_EXPORT cl::size_t<3> createRegion(unsigned int x, unsigned int y, unsigned int z);
FAST_EXPORT cl::size_t<3> createRegion(Vector3ui size);
FAST_EXPORT cl::size_t<3> createOrigoRegion();

FAST_EXPORT std::string getCLErrorString(cl_int err);

/**
 * Function for splitting a string
 * @param input string
 * @param delimiter string
 * @return vector of strings
 */
FAST_EXPORT std::vector<std::string> split(const std::string& input, const std::string& delimiter = " ");

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

/*
 * Replace all occurences of from to to in str
 */
FAST_EXPORT std::string replace(std::string str, std::string find, std::string replacement);

template <class T>
static inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

FAST_EXPORT void loadPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar);

/*
 * Creates a directory at the given path.
 * Throws exception if it fails
 */
FAST_EXPORT void createDirectory(std::string path);

/*
 * Creates all directories in the given path.
 * Throws exception if it fails
 */
FAST_EXPORT void createDirectories(std::string path);


/**
 * Returns a string of the current date
 * @param format see http://en.cppreference.com/w/cpp/chrono/c/strftime
 * @return
 */
FAST_EXPORT std::string currentDateTime(std::string format = "%Y-%m-%d-%H%M%S");

} // end namespace fast

#endif /* UTILITY_HPP_ */
