#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#define NOMINMAX // Removes windows min and max macros
#include "FAST/Exception.hpp"
#include "OpenCL.hpp"
#include "FAST/ExecutionDevice.hpp"
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
using Eigen::VectorXi;
using Eigen::Vector4f;
using Eigen::Vector3f;
using Eigen::Vector2f;
using Eigen::Vector4i;
using Eigen::Vector3i;
using Eigen::Vector2i;
typedef Eigen::Matrix<uint, 4, 1> Vector4ui;
typedef Eigen::Matrix<uint, 3, 1> Vector3ui;
typedef Eigen::Matrix<uint, 2, 1> Vector2ui;

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

} // end namespace
#endif
