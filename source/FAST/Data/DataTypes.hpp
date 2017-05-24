#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#define NOMINMAX // Removes windows min and max macros
#define _USE_MATH_DEFINES
#include <cmath>
#include "FAST/Exception.hpp"
#include "CL/OpenCL.hpp"
#include "FAST/ExecutionDevice.hpp"
#include <iostream>
#include <Eigen/Dense>

// These have to be outside of fast namespace or it will not compile with Qt on Windows. Why?
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

namespace fast {

using Eigen::MatrixXf;
using Eigen::Matrix2f;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::VectorXf;
using Eigen::VectorXi;
using Eigen::Vector4f;
using Eigen::Vector3f;
using Eigen::Vector2f;
using Eigen::Vector4i;
using Eigen::Vector3i;
using Eigen::Vector2i;
using Eigen::Affine3f;
typedef Eigen::Matrix<uint, Eigen::Dynamic, 1> VectorXui;
typedef Eigen::Matrix<uint, 4, 1> Vector4ui;
typedef Eigen::Matrix<uint, 3, 1> Vector3ui;
typedef Eigen::Matrix<uint, 2, 1> Vector2ui;

enum DataType {
    TYPE_FLOAT,
    TYPE_UINT8,
    TYPE_INT8,
    TYPE_UINT16,
    TYPE_INT16,
    TYPE_UNORM_INT16, // Unsigned normalized 16 bit integer. A 16 bit int interpreted as a float between 0 and 1.
    TYPE_SNORM_INT16 // Signed normalized 16 bit integer. A 16 bit int interpreted as a float between -1 and 1.
};

enum PlaneType {PLANE_X, PLANE_Y, PLANE_Z};

// Returns the C type for a DataType as a string
FAST_EXPORT std::string getCTypeAsString(DataType type);

#define fastCaseTypeMacro(fastType, cType, call) case fastType: {typedef cType FAST_TYPE; call;} break;

#define fastSwitchTypeMacro(call) \
        fastCaseTypeMacro(TYPE_FLOAT, float, call) \
        fastCaseTypeMacro(TYPE_INT8, char, call) \
        fastCaseTypeMacro(TYPE_UINT8, uchar, call) \
        fastCaseTypeMacro(TYPE_INT16, short, call) \
        fastCaseTypeMacro(TYPE_UINT16, ushort, call) \
        fastCaseTypeMacro(TYPE_SNORM_INT16, short, call) \
        fastCaseTypeMacro(TYPE_UNORM_INT16, ushort, call) \

FAST_EXPORT cl::ImageFormat getOpenCLImageFormat(OpenCLDevice::pointer, cl_mem_object_type imageType, DataType type, unsigned int components);

FAST_EXPORT size_t getSizeOfDataType(DataType type, unsigned int nrOfComponents);

FAST_EXPORT float getDefaultIntensityLevel(DataType type);
FAST_EXPORT float getDefaultIntensityWindow(DataType type);

FAST_EXPORT void deleteArray(void * data, DataType type);

} // end namespace
#endif
