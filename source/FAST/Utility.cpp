#include "FAST/Utility.hpp"
#include "HelperFunctions.hpp"
#include <cmath>

namespace fast {

double log2(double n) {
    return log( n ) / log( 2.0 );
}

double round(double n) {
	return (n - floor(n) > 0.5) ? ceil(n) : floor(n);
}

int pow(int a, int b) {
    return (int)std::pow((double)a, (double)b);
}

void* allocateDataArray(unsigned int voxels, DataType type, unsigned int nrOfComponents) {
    unsigned int size = voxels*nrOfComponents;
    void * data;
    switch(type) {
        fastSwitchTypeMacro(data = new FAST_TYPE[size])
    }

    return data;
}

template <class T>
inline void getMaxAndMinFromOpenCLImageResult(void* voidData, unsigned int size, unsigned int nrOfComponents, float* min, float* max) {
    T* data = (T*)voidData;
    *min = data[0];
    *max = data[1];
    for(unsigned int i = nrOfComponents; i < size*nrOfComponents; i += nrOfComponents) {
        //std::cout << "min: " << data[i] << std::endl;
        //std::cout << "max: " << data[i+1] << std::endl;
        if(data[i] < *min) {
            *min = data[i];
        }
        if(data[i+1] > *max) {
            *max = data[i+1];
        }
    }
}

unsigned int getPowerOfTwoSize(unsigned int size) {
    int i = 1;
    while(pow(2, i) < size)
        i++;

    return (unsigned int)pow(2,i);
}


void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* min, float* max) {
    // Get power of two size
    unsigned int powerOfTwoSize = getPowerOfTwoSize(std::max(image.getImageInfo<CL_IMAGE_WIDTH>(), image.getImageInfo<CL_IMAGE_HEIGHT>()));

    // Create image levels
    unsigned int size = powerOfTwoSize;
    size /= 2;
    std::vector<cl::Image2D> levels;
    while(size >= 4) {
        cl::Image2D level = cl::Image2D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, type, 2), size, size);
        levels.push_back(level);
        size /= 2;
    }

    // Compile OpenCL code
    std::string buildOptions = "";
    switch(type) {
    case TYPE_FLOAT:
        buildOptions = "-DTYPE_FLOAT";
        break;
    case TYPE_UINT8:
        buildOptions = "-DTYPE_UINT8";
        break;
    case TYPE_INT8:
        buildOptions = "-DTYPE_INT8";
        break;
    case TYPE_UINT16:
        buildOptions = "-DTYPE_UINT16";
        break;
    case TYPE_INT16:
        buildOptions = "-DTYPE_INT16";
        break;
    }
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/ImageMinMax.cl", buildOptions);
    cl::Program program = device->getProgram(programNr);
    cl::CommandQueue queue = device->getCommandQueue();

    // Fill first level
    size = powerOfTwoSize/2;
    cl::Kernel firstLevel(program, "createFirstMinMaxImage2DLevel");
    firstLevel.setArg(0, image);
    firstLevel.setArg(1, levels[0]);

    queue.enqueueNDRangeKernel(
            firstLevel,
            cl::NullRange,
            cl::NDRange(size,size),
            cl::NullRange
    );

    // Fill all other levels
    cl::Kernel createLevel(program, "createMinMaxImage2DLevel");
    int i = 0;
    size /= 2;
    while(size >= 4) {
        createLevel.setArg(0, levels[i]);
        createLevel.setArg(1, levels[i+1]);
        queue.enqueueNDRangeKernel(
                createLevel,
                cl::NullRange,
                cl::NDRange(size,size),
                cl::NullRange
        );
        i++;
        size /= 2;
    }

    // Get result from the last level
    unsigned int nrOfElements = 4*4;
    unsigned int nrOfComponents = getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, type, 2).image_channel_order == CL_RGBA ? 4 : 2;
    void* result = allocateDataArray(nrOfElements,type,nrOfComponents);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,oul::createOrigoRegion(),oul::createRegion(4,4,1),0,0,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, nrOfComponents, min, max);
        break;
    }

}

void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image3D image, DataType type, float* min, float* max) {



   // Get power of two size
    unsigned int powerOfTwoSize = getPowerOfTwoSize(std::max(image.getImageInfo<CL_IMAGE_DEPTH>(), std::max(
            image.getImageInfo<CL_IMAGE_WIDTH>(),
            image.getImageInfo<CL_IMAGE_HEIGHT>())));

    // Create image levels
    unsigned int size = powerOfTwoSize;
    size /= 2;
    std::vector<cl::Image3D> levels;
    while(size >= 4) {
        cl::Image3D level = cl::Image3D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE3D, type, 2), size, size, size);
        levels.push_back(level);
        size /= 2;
    }

    // Compile OpenCL code
    std::string buildOptions = "";
    switch(type) {
    case TYPE_FLOAT:
        buildOptions = "-DTYPE_FLOAT";
        break;
    case TYPE_UINT8:
        buildOptions = "-DTYPE_UINT8";
        break;
    case TYPE_INT8:
        buildOptions = "-DTYPE_INT8";
        break;
    case TYPE_UINT16:
        buildOptions = "-DTYPE_UINT16";
        break;
    case TYPE_INT16:
        buildOptions = "-DTYPE_INT16";
        break;
    }
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/ImageMinMax.cl", buildOptions);
    cl::Program program = device->getProgram(programNr);
    cl::CommandQueue queue = device->getCommandQueue();

    // Fill first level
    size = powerOfTwoSize/2;
    cl::Kernel firstLevel(program, "createFirstMinMaxImage3DLevel");
    firstLevel.setArg(0, image);
    firstLevel.setArg(1, levels[0]);

    queue.enqueueNDRangeKernel(
            firstLevel,
            cl::NullRange,
            cl::NDRange(size,size,size),
            cl::NullRange
    );

    // Fill all other levels
    cl::Kernel createLevel(program, "createMinMaxImage3DLevel");
    int i = 0;
    size /= 2;
    while(size >= 4) {
        createLevel.setArg(0, levels[i]);
        createLevel.setArg(1, levels[i+1]);
        queue.enqueueNDRangeKernel(
                createLevel,
                cl::NullRange,
                cl::NDRange(size,size,size),
                cl::NullRange
        );
        i++;
        size /= 2;
    }

    // Get result from the last level
    unsigned int nrOfElements = 4*4*4;
    unsigned int nrOfComponents = getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE3D, type, 2).image_channel_order == CL_RGBA ? 4 : 2;
    void* result = allocateDataArray(nrOfElements,type,nrOfComponents);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,oul::createOrigoRegion(),oul::createRegion(4,4,4),0,0,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, nrOfComponents, min, max);
        break;
    }

}

void getMaxAndMinFromOpenCLBuffer(OpenCLDevice::pointer device, cl::Buffer buffer, unsigned int size, DataType type, float* min, float* max) {
    // Compile OpenCL code
    std::string buildOptions = "";
    switch(type) {
    case TYPE_FLOAT:
        buildOptions = "-DTYPE_FLOAT";
        break;
    case TYPE_UINT8:
        buildOptions = "-DTYPE_UINT8";
        break;
    case TYPE_INT8:
        buildOptions = "-DTYPE_INT8";
        break;
    case TYPE_UINT16:
        buildOptions = "-DTYPE_UINT16";
        break;
    case TYPE_INT16:
        buildOptions = "-DTYPE_INT16";
        break;
    }
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/ImageMinMax.cl", buildOptions);
    cl::Program program = device->getProgram(programNr);
    cl::CommandQueue queue = device->getCommandQueue();

    // Nr of work groups must be set so that work-group size does not exceed max work-group size (256 on AMD)
    int length = size;
    cl::Kernel reduce(program, "reduce");

    cl::Buffer current = buffer;
    cl::Buffer clResult;
    int workGroupSize = 256;
    int workGroups = 256;
    int X = ceil((float)length / (workGroups*workGroupSize));

    std::cout << "number of work groups is: " << workGroups << std::endl;
    std::cout << "X is: " << X << std::endl;
    clResult = cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, getSizeOfDataType(type,1)*workGroups*2);
    reduce.setArg(0, current);
    reduce.setArg(1, workGroupSize * getSizeOfDataType(type,1), NULL);
    reduce.setArg(2, workGroupSize * getSizeOfDataType(type,1), NULL);
    reduce.setArg(3, size);
    reduce.setArg(4, X);
    reduce.setArg(5, clResult);

    queue.enqueueNDRangeKernel(
            reduce,
            cl::NullRange,
            cl::NDRange(workGroups*workGroupSize),
            cl::NDRange(workGroupSize)
    );

    length = workGroups;

    void* result = allocateDataArray(length, type, 2);
    unsigned int nrOfElements = length;
    queue.enqueueReadBuffer(clResult,CL_TRUE,0,getSizeOfDataType(type,1)*workGroups*2,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, 2, min, max);
        break;
    }
}

} // end namespace fast
