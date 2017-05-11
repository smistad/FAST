#include "DataComparison.hpp"
#include "FAST/Utility.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <ctime>

namespace fast {

void* allocateRandomData(unsigned int nrOfVoxels, DataType type) {
    srand(time(NULL));
    switch(type) {
    case TYPE_FLOAT:
    {
        float* data = new float[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = (float)rand() / RAND_MAX;
        return (void*)data;
    }
        break;
    case TYPE_INT8:
    {
        char* data = new char[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255 - 128;
        return (void*)data;
    }
        break;
    case TYPE_UINT8:
    {
        uchar* data = new uchar[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255;
        return (void*)data;
    }
        break;
    case TYPE_INT16:
    {
        short* data = new short[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255 - 128;
        return (void*)data;
    }
        break;
    case TYPE_UINT16:
    {
        ushort* data = new ushort[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255;
        return (void*)data;
    }
        break;
    }
    return NULL;
}

bool compareDataArrays(void* data1, void* data2, unsigned int nrOfVoxels, DataType type) {
    bool success = true;
    switch(type) {
        fastSwitchTypeMacro(
        FAST_TYPE* data1c = (FAST_TYPE*)data1;
        FAST_TYPE* data2c = (FAST_TYPE*)data2;
        for(unsigned int i = 0; i < nrOfVoxels; i++) {
            if(data1c[i] != data2c[i]) {
            	/*
                reportInfo() << i << Reporter::end();
                reportInfo() << data1c[i] << Reporter::end();
                reportInfo() << data2c[i] << Reporter::end();
                */
                success = false;
                break;
            }
        }
        );
    }
    return success;
}

bool compareBufferWithDataArray(cl::Buffer buffer, OpenCLDevice::pointer device, void* data, unsigned int nrOfVoxels, DataType type) {
    // First, transfer data from buffer
    unsigned int elementSize;
    void* bufferData;
    switch(type) {
        fastSwitchTypeMacro(
            elementSize = sizeof(FAST_TYPE);
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadBuffer(buffer, CL_TRUE, 0, nrOfVoxels*elementSize, bufferData);

    return compareDataArrays(bufferData, data, nrOfVoxels, type);
}

template <class T>
void * removePadding(T * data, unsigned int size, unsigned int nrOfComponents) {
     T * newData = new T[size*nrOfComponents];
    for(unsigned int i = 0; i < size; i++) {
    	if(nrOfComponents == 1) {
            newData[i] = data[i*4];
    	} else if(nrOfComponents == 2) {
            newData[i*2] = data[i*4];
            newData[i*2+1] = data[i*4+1];
    	} else {
            newData[i*3] = data[i*4];
            newData[i*3+1] = data[i*4+1];
            newData[i*3+2] = data[i*4+2];
    	}
    }
    return (void *)newData;
}

bool compareImage2DWithDataArray(
        cl::Image2D image,
        OpenCLDevice::pointer device,
        void* data,
        unsigned int width,
        unsigned int height,
        unsigned int nrOfComponents,
        DataType type) {
    // First, transfer data from image
    void* bufferData;
    unsigned int nrOfVoxels = width*height;
    _cl_image_format format = image.getImageInfo<CL_IMAGE_FORMAT>();
    if(format.image_channel_order == CL_RGBA) {
        nrOfVoxels *= 4;
    } else {
        nrOfVoxels *= nrOfComponents;
    }
    switch(type) {
        fastSwitchTypeMacro(
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadImage(image, CL_TRUE, createOrigoRegion(), createRegion(width, height, 1), 0, 0, bufferData);

    if(format.image_channel_order == CL_RGBA && nrOfComponents != 4) {
        // Since OpenCL images doesn't support 3 channels, 4 channel image is used and we must remove the padding
        switch(type) {
            fastSwitchTypeMacro(
                void * newbufferData = removePadding((FAST_TYPE*)bufferData, width*height, nrOfComponents);
                deleteArray(bufferData, type);
                bufferData = newbufferData;
            );
        }
    }

    return compareDataArrays(bufferData, data, width*height*nrOfComponents, type);
}

bool compareImage3DWithDataArray(
        cl::Image3D image,
        OpenCLDevice::pointer device,
        void* data,
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        unsigned int nrOfComponents,
        DataType type) {
    // First, transfer data from image
    void* bufferData;
    unsigned int nrOfVoxels = width*height*depth;
    _cl_image_format format = image.getImageInfo<CL_IMAGE_FORMAT>();
    if(format.image_channel_order == CL_RGBA) {
        nrOfVoxels *= 4;
    } else {
        nrOfVoxels *= nrOfComponents;
    }
    switch(type) {
        fastSwitchTypeMacro(
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadImage(image, CL_TRUE, createOrigoRegion(), createRegion(width, height, depth), 0, 0, bufferData);

    if(format.image_channel_order == CL_RGBA && nrOfComponents != 4) {
        // Since OpenCL images doesn't support 3 channels, 4 channel image is used and we must remove the padding
        switch(type) {
            fastSwitchTypeMacro(
                void * newbufferData = removePadding((FAST_TYPE*)bufferData, width*height*depth, nrOfComponents);
                deleteArray(bufferData, type);
                bufferData = newbufferData;
            );
        }
    }

    return compareDataArrays(bufferData, data, width*height*depth*nrOfComponents, type);
}

} // end namespace fast
