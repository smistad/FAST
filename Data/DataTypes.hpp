#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include "Exception.hpp"
#include "OpenCL.hpp"

namespace fast {

typedef unsigned char uchar;
typedef unsigned short ushort;


enum DataType { TYPE_FLOAT, TYPE_UINT8, TYPE_INT8, TYPE_UINT16, TYPE_INT16 };

#define fastCaseTypeMacro(fastType, cType, call) case fastType: {typedef cType FAST_TYPE; call;} break;

#define fastSwitchTypeMacro(call) \
        fastCaseTypeMacro(TYPE_FLOAT, float, call) \
        fastCaseTypeMacro(TYPE_INT8, char, call) \
        fastCaseTypeMacro(TYPE_UINT8, uchar, call) \
        fastCaseTypeMacro(TYPE_INT16, short, call) \
        fastCaseTypeMacro(TYPE_UINT16, ushort, call) \

cl::ImageFormat getOpenCLImageFormat(DataType type, unsigned int components);

size_t getSizeOfDataType(DataType type, unsigned int nrOfComponents);

float getDefaultIntensityLevel(DataType type);
float getDefaultIntensityWindow(DataType type);

void deleteArray(void * data, DataType type);

template <class T, int N>
class Vector {
    public:
        T x();
        T y();
        T z();
        T& operator[](unsigned int index); // lvalue
        const T& operator[](unsigned int index) const; // rvalue
        int getSize() const;
        Vector(const Vector& other);
        Vector();
        ~Vector();
    protected:
        T* data;
};

template <int N>
class Float : public Vector<float, N> {
};

} // end namespace

template<class T, int N>
inline T fast::Vector<T,N>::x() {
    return data[0];
}

template<class T, int N>
inline T fast::Vector<T,N>::y() {
    return data[1];
}

template<class T, int N>
inline T fast::Vector<T,N>::z() {
    return data[2];
}

template<class T, int N>
inline T& fast::Vector<T,N>::operator [](unsigned int index) {
    return data[index];
}

template<class T, int N>
inline const T& fast::Vector<T,N>::operator [](unsigned int index) const {
    return const_cast<T&>((*this)[index]);
}

template<class T, int N>
inline int fast::Vector<T,N>::getSize() const {
    return N;
}

template<class T, int N>
inline fast::Vector<T,N>::Vector(const Vector& other) {
    data = new T[N];
    for(unsigned int i = 0; i < N; i++)
        data[i] = other[i];
}

template<class T, int N>
inline fast::Vector<T,N>::Vector() {
    data = new T[N];
    for(unsigned int i = 0; i < N; i++)
        data[i] = 0;
}

template<class T, int N>
inline fast::Vector<T,N>::~Vector() {
    delete[] data;
}

#endif
