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
        T& operator[](unsigned int index) const; // lvalue
        Vector<T,N>& operator=(const Vector<T,N>& other);
        T get(unsigned int index) const;
        int getSize() const;
        Vector(const Vector<T,N>& other); // copy constructor
        Vector();
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

} // end namespace
#endif
