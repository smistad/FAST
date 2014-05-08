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

} // end namespace

#endif
