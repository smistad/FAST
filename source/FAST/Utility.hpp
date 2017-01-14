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

cl::size_t<3> createRegion(unsigned int x, unsigned int y, unsigned int z);
cl::size_t<3> createRegion(Vector3ui size);
cl::size_t<3> createOrigoRegion();

std::string getCLErrorString(cl_int err);

/**
 * Function for splitting a string
 * @param input string
 * @param delimiter string
 * @return vector of strings
 */
std::vector<std::string> split(const std::string& input, const std::string& delimiter = " ");

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

template <class T>
static inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

static inline void loadPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar) {
    float ymax = zNear * tan(fovy * M_PI / 360.0);
    float ymin = -ymax;
    float xmin = ymin * aspect;
    float xmax = ymax * aspect;
    glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

} // end namespace fast

// Hasher for enums
// This is not working on windows:
#ifndef _WIN32
namespace std {
    template<class E>
	class hash {
        using sfinae = typename std::enable_if<std::is_enum<E>::value, E>::type;
    public:
        size_t operator()(const E&e) const {
            return std::hash<typename std::underlying_type<E>::type>()(e);
        }
    };
};
#endif

#endif /* UTILITY_HPP_ */
