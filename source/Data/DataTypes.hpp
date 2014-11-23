#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include "Exception.hpp"
#include "OpenCL.hpp"
#include "ExecutionDevice.hpp"
#include <cmath>
#include <iostream>
#if defined(__APPLE__) || defined(__MACOSX)
#include <eigen3/Eigen/Dense>
#elif _WIN32
#include <Eigen/Dense>
#else
#include <eigen3/Eigen/Dense>
#endif

// These have to be outside of fast namespace or it will not compile with Qt on Windows. Why?
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

namespace fast {

using Eigen::MatrixXf;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::VectorXf;
using Eigen::Vector4f;
using Eigen::Vector3f;
using Eigen::Vector4i;
using Eigen::Vector3i;
typedef Eigen::Matrix<uint, 3, 1> Vector3ui;
typedef Eigen::Matrix<uint, 4, 1> Vector4ui;




enum DataType { TYPE_FLOAT, TYPE_UINT8, TYPE_INT8, TYPE_UINT16, TYPE_INT16 };

#define fastCaseTypeMacro(fastType, cType, call) case fastType: {typedef cType FAST_TYPE; call;} break;

#define fastSwitchTypeMacro(call) \
        fastCaseTypeMacro(TYPE_FLOAT, float, call) \
        fastCaseTypeMacro(TYPE_INT8, char, call) \
        fastCaseTypeMacro(TYPE_UINT8, uchar, call) \
        fastCaseTypeMacro(TYPE_INT16, short, call) \
        fastCaseTypeMacro(TYPE_UINT16, ushort, call) \

cl::ImageFormat getOpenCLImageFormat(OpenCLDevice::pointer, cl_mem_object_type imageType, DataType type, unsigned int components);

size_t getSizeOfDataType(DataType type, unsigned int nrOfComponents);

float getDefaultIntensityLevel(DataType type);
float getDefaultIntensityWindow(DataType type);

void deleteArray(void * data, DataType type);

// TODO fix the operator casting for vector
class Float3;

template <class T, int N>
class Vector {
    public:
        T x();
        T y();
        T z();
        T& operator[](unsigned int index) const; // lvalue
        Vector<T,N>& operator=(const Vector<T,N>& other);
        T get(unsigned int index) const;
        int getSize() const;
        Vector(const Vector<T,N>& other); // copy constructor
        Vector();
        template<class U>
        Vector<T,N> operator*(const U& scalar) const;
        template<class U>
        Vector<T,N> operator/(const U& scalar) const;
        Vector<T,N> operator+(const Vector<T,N>& vector) const;
        Vector<T,N> operator-(const Vector<T,N>& vector) const;
        T dot(const Vector<T,N>& vector) const;
        float distance(const Vector<T,N>& vector) const;
        Vector<T,N> normalize() const;
        operator Float3();
        void print() const;
        ~Vector();
    protected:
        T* data;
};

// Some useful macros for creating several vector classes
#define createDefaultVectorClassMacro(name, type)   \
template <unsigned int N>                           \
class name : public Vector<type, N> {               \
};                                                  \

#define createNumberedVectorClassMacro2(name,type)       \
class name##2 : public name<2> {                    \
    public:                                         \
        name##2() : name<2>() {data = new type[2];};\
        name##2(type x, type y) {                   \
            data = new type[2];\
            data[0] = x;\
            data[1] = y;\
        };\
};                                                  \

#define createNumberedVectorClassMacro3(name,type)       \
class name##3 : public name<3> {                    \
    public:                                         \
        name##3() : name<3>() {data = new type[3];};\
        name##3(type x, type y, type z) {                   \
            data = new type[3];\
            data[0] = x;\
            data[1] = y;\
            data[2] = z;\
        };\
};                                                  \

#define createNumberedVectorClassMacro4(name,type)       \
class name##4 : public name<4> {                    \
    public:                                         \
        name##4() : name<4>() {data = new type[4];};\
        name##4(type x, type y, type z, type w) {                   \
            data = new type[4];\
            data[0] = x;\
            data[1] = y;\
            data[2] = z;\
            data[3] = w;\
        };\
};\


#define createVectorTypesMacro(name, type)          \
    createDefaultVectorClassMacro(name, type)       \
    createNumberedVectorClassMacro2(name,type)         \
    createNumberedVectorClassMacro3(name,type)         \
    createNumberedVectorClassMacro4(name,type)         \

createVectorTypesMacro(Float, float);
createVectorTypesMacro(Double, double);
createVectorTypesMacro(Uint, uint);
createVectorTypesMacro(Int, int);
createVectorTypesMacro(Uchar, uchar);
createVectorTypesMacro(Char, char);

template<class T, int N>
Vector<T,N>::operator Float3() {
    Float3 result;
    result[0] = data[0];
    result[1] = data[1];
    result[2] = data[2];
    return result;
}

// TODO: add out of bounds checks

template<class T, int N>
inline T Vector<T,N>::x() {
    return data[0];
}

template<class T, int N>
inline T Vector<T,N>::y() {
    return data[1];
}

template<class T, int N>
inline T Vector<T,N>::z() {
    return data[2];
}

template<class T, int N>
inline T& Vector<T,N>::operator [](const unsigned int index) const {
    return data[index];
}

template<class T, int N>
inline int Vector<T,N>::getSize() const {
    return N;
}

template<class T, int N>
inline T Vector<T,N>::get(unsigned int i) const {
    return data[i];
}

template<class T, int N>
inline Vector<T,N>::Vector(const Vector<T,N>& other) {
    data = new T[N];
    for(unsigned int i = 0; i < N; i++)
        data[i] = other.get(i);
}

template<class T, int N>
inline Vector<T,N>::Vector() {
    data = new T[N];
    for(unsigned int i = 0; i < N; i++)
        data[i] = T();
}

template<class T, int N>
inline Vector<T,N>& Vector<T,N>::operator=(const Vector<T,N>& other) {
    data = new T[N];
    for(unsigned int i = 0; i < N; i++)
        data[i] = other.get(i);
    return *this;
}

template<class T, int N>
inline Vector<T,N>::~Vector() {
    delete[] data;
}

template<class T, int N>
template<class U>
Vector<T,N> Vector<T,N>::operator*(const U& scalar) const {
    Vector<T, N> result;
    for(unsigned int i = 0; i < N; i++)
        result[i] = data[i]*scalar;
    return result;
}

template<class T, int N>
template<class U>
Vector<T,N> Vector<T,N>::operator/(const U& scalar) const {
    Vector<T, N> result;
    for(unsigned int i = 0; i < N; i++)
        result[i] = data[i]/scalar;
    return result;
}

template<class T, int N>
Vector<T,N> Vector<T,N>::operator+(const Vector<T,N>& vector) const {
    Vector<T, N> result;
    for(unsigned int i = 0; i < N; i++)
        result[i] = data[i]+vector[i];
    return result;
}

template<class T, int N>
Vector<T,N> Vector<T,N>::operator-(const Vector<T,N>& vector) const {
    Vector<T, N> result;
    for(unsigned int i = 0; i < N; i++)
        result[i] = data[i]-vector[i];
    return result;
}

template<class T, int N>
T Vector<T,N>::dot(const Vector<T,N>& vector) const {
    T result = 0;
    for(unsigned int i = 0; i < N; i++)
        result += data[i]*vector[i];
    return result;
}

template<class T, int N>
Vector<T,N> Vector<T,N>::normalize() const {
    Vector<T,N> result;

    double sum = 0.0;
    for(unsigned int i = 0; i < N; i++)
        sum += data[i]*data[i];
    for(unsigned int i = 0; i < N; i++)
        result[i] = (double)data[i] / sqrt(sum);

    return result;
}

template<class T, int N>
float Vector<T,N>::distance(const Vector<T,N>& vector) const {
    float sum = 0.0;
    for(unsigned int i = 0; i < N; i++) {
        sum += (data[i]-vector[i])*(data[i]-vector[i]);
    }

    return sqrt(sum);
}

template<class T, int N>
void Vector<T, N>::print() const {
    for(unsigned int i = 0; i < N; i++) {
        std::cout << data[i] << std::endl;
    }
}


} // end namespace
#endif
