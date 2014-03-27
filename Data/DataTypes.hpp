#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include "Exception.hpp"
#include "OpenCL.hpp"

namespace fast {

enum DataType { TYPE_FLOAT, TYPE_UINT8, TYPE_INT8, TYPE_UINT16, TYPE_INT16 };

cl::ImageFormat createOpenCLImageFormat(DataType type, int components);

} // end namespace

#endif
