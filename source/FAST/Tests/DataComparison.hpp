#ifndef DATACOMPARISON_HPP_
#define DATACOMPARISON_HPP_

#include "FAST/Data/DataTypes.hpp"
#include "FAST/ExecutionDevice.hpp"

namespace fast {


void* allocateRandomData(unsigned int nrOfVoxels, DataType type);

bool compareDataArrays(void* data1, void* data2, unsigned int nrOfVoxels, DataType type);

bool compareBufferWithDataArray(cl::Buffer buffer, OpenCLDevice::pointer device, void* data, unsigned int nrOfVoxels, DataType type);


bool compareImage2DWithDataArray(
        cl::Image2D image,
        OpenCLDevice::pointer device,
        void* data,
        unsigned int width,
        unsigned int height,
        unsigned int nrOfComponents,
        DataType type);


bool compareImage3DWithDataArray(
        cl::Image3D image,
        OpenCLDevice::pointer device,
        void* data,
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        unsigned int nrOfComponents,
        DataType type);


}



#endif /* DATACOMPARISON_HPP_ */
