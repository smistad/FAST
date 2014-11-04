#include "DataTypes.hpp"
using namespace fast;

cl::ImageFormat fast::getOpenCLImageFormat(OpenCLDevice::pointer device, cl_mem_object_type imageType, DataType type, unsigned int components) {
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
    	// Use 1 channel image if it is supported
    	if(device->isImageFormatSupported(CL_R, channelType, imageType)) {
            channelOrder = CL_R;
    	} else {
            channelOrder = CL_RGBA;
    	}
        break;
    case 2:
    	// Use 2 channel image if it is supported
    	if(device->isImageFormatSupported(CL_RG, channelType, imageType)) {
            channelOrder = CL_RG;
    	} else {
            channelOrder = CL_RGBA;
    	}
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

float fast::getDefaultIntensityLevel(DataType type) {
    float level;
    switch(type) {
    case TYPE_FLOAT:
        level = 0.5;
        break;
    case TYPE_UINT8:
        level = 128;
        break;
    case TYPE_INT8:
        level = 0;
        break;
    case TYPE_UINT16:
        level = 128;
        break;
    case TYPE_INT16:
        level = 0;
        break;
    }
    return level;
}

float fast::getDefaultIntensityWindow(DataType type) {
    float window;
    switch(type) {
    case TYPE_FLOAT:
        window = 1.0;
        break;
    case TYPE_UINT8:
        window = 255;
        break;
    case TYPE_INT8:
        window = 255;
        break;
    case TYPE_UINT16:
        window = 255;
        break;
    case TYPE_INT16:
        window = 255;
        break;
    }
    return window;
}

void fast::deleteArray(void * data, DataType type) {
    switch(type) {
        case TYPE_FLOAT:
            delete[] (float*)data;
            break;
        case TYPE_UINT8:
            delete[] (uchar*)data;
            break;
        case TYPE_INT8:
            delete[] (char*)data;
            break;
        case TYPE_UINT16:
            delete[] (ushort*)data;
            break;
        case TYPE_INT16:
            delete[] (short*)data;
            break;
    }
}
