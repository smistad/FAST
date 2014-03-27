#include "DataTypes.hpp"
using namespace fast;

cl::ImageFormat fast::getOpenCLImageFormat(DataType type, unsigned int components) {
    cl_channel_order channelOrder;
    cl_channel_type channelType;

    switch(type) {
    case TYPE_FLOAT:
        channelType = CL_FLOAT;
        break;
    case TYPE_UINT8:
        channelType = CL_UNSIGNED_INT8;
        break;
    case TYPE_INT8:
        channelType = CL_SIGNED_INT8;
        break;
    case TYPE_UINT16:
        channelType = CL_UNSIGNED_INT16;
        break;
    case TYPE_INT16:
        channelType = CL_SIGNED_INT16;
        break;
    }

    switch(components) {
    case 1:
        channelOrder = CL_R;
        break;
    case 2:
        channelOrder = CL_RG;
        break;
    case 3:
        // There is no 3 channel so a 4 channel has to be used
        channelOrder = CL_RGBA;
        break;
    case 4:
        channelOrder = CL_RGBA;
        break;
    default:
        throw Exception("Number of components has to be between 1 and 4");
        break;
    }

    return cl::ImageFormat(channelOrder, channelType);
}

size_t fast::getSizeOfDataType(DataType type, unsigned int nrOfComponents) {
    size_t bytes;
    switch(type) {
    case TYPE_FLOAT:
        bytes = sizeof(float);
        break;
    case TYPE_UINT8:
    case TYPE_INT8:
        bytes = sizeof(char);
        break;
    case TYPE_UINT16:
    case TYPE_INT16:
        bytes = sizeof(short);
        break;
    }

    return nrOfComponents*bytes;
}
