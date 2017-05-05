#include "FAST/Utility.hpp"
#include "FAST/Config.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <regex>
#ifdef _WIN32
#include <direct.h> // Needed for _mkdir
#else
// Needed for making directory
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

namespace fast {

double log2(double n) {
    return log( n ) / log( 2.0 );
}

double round(double n) {
	return (n - floor(n) > 0.5) ? ceil(n) : floor(n);
}

double round(double n, int decimals) {
    int factor = decimals*10;
    return round(n*factor)/factor;
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

void getIntensitySumFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* sum) {
    // Get power of two size
    unsigned int powerOfTwoSize = getPowerOfTwoSize(std::max(image.getImageInfo<CL_IMAGE_WIDTH>(), image.getImageInfo<CL_IMAGE_HEIGHT>()));

    // Create image levels
    unsigned int size = powerOfTwoSize;
    size /= 2;
    std::vector<cl::Image2D> levels;
    while(size >= 4) {
        cl::Image2D level = cl::Image2D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, TYPE_FLOAT, 1), size, size);
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
    std::string sourceFilename = Config::getKernelSourcePath() + "/ImageSum.cl";
    std::string programName = sourceFilename + buildOptions;
    // Only create program if it doesn't exist for this device from before
    if(!device->hasProgram(programName))
        device->createProgramFromSourceWithName(programName, sourceFilename, buildOptions);
    cl::Program program = device->getProgram(programName);
    cl::CommandQueue queue = device->getCommandQueue();

    // Fill first level
    size = powerOfTwoSize/2;
    cl::Kernel firstLevel(program, "createFirstSumImage2DLevel");
    firstLevel.setArg(0, image);
    firstLevel.setArg(1, levels[0]);

    queue.enqueueNDRangeKernel(
            firstLevel,
            cl::NullRange,
            cl::NDRange(size,size),
            cl::NullRange
    );

    // Fill all other levels
    cl::Kernel createLevel(program, "createSumImage2DLevel");
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
    unsigned int nrOfComponents = getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, TYPE_FLOAT, 1).image_channel_order == CL_RGBA ? 4 : 1;
    float* result = (float*)allocateDataArray(nrOfElements,TYPE_FLOAT,nrOfComponents);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,createOrigoRegion(),createRegion(4,4,1),0,0,result);
    *sum = getSumFromOpenCLImageResult<float>(result, nrOfElements, nrOfComponents);
    delete[] result;
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
    std::string sourceFilename = Config::getKernelSourcePath() + "/ImageMinMax.cl";
    std::string programName = sourceFilename + buildOptions;
    // Only create program if it doesn't exist for this device from before
    if(!device->hasProgram(programName))
        device->createProgramFromSourceWithName(programName, sourceFilename, buildOptions);
    cl::Program program = device->getProgram(programName);
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
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,createOrigoRegion(),createRegion(4,4,1),0,0,result);
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
    deleteArray(result, type);
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
    // Add fast_3d_image_writes flag if it is supported
    if(device->isWritingTo3DTexturesSupported()) {
        if(buildOptions.size() > 0)
            buildOptions += " ";
        buildOptions += "-Dfast_3d_image_writes";
    }
    std::string sourceFilename = Config::getKernelSourcePath() + "/ImageMinMax.cl";
    std::string programName = sourceFilename + buildOptions;
    // Only create program if it doesn't exist for this device from before
    if(!device->hasProgram(programName))
        device->createProgramFromSourceWithName(programName, sourceFilename, buildOptions);
    cl::Program program = device->getProgram(programName);
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
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,createOrigoRegion(),createRegion(4,4,4),0,0,result);
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
    deleteArray(result, type);

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
    std::string sourceFilename = Config::getKernelSourcePath() + "/ImageMinMax.cl";
    std::string programName = sourceFilename + buildOptions;
    // Only create program if it doesn't exist for this device from before
    if(!device->hasProgram(programName))
        device->createProgramFromSourceWithName(programName, sourceFilename, buildOptions);
    cl::Program program = device->getProgram(programName);
    cl::CommandQueue queue = device->getCommandQueue();

    // Nr of work groups must be set so that work-group size does not exceed max work-group size (256 on AMD)
    int length = size;
    cl::Kernel reduce(program, "reduce");

    cl::Buffer current = buffer;
    cl::Buffer clResult;
    int workGroupSize = 256;
    int workGroups = 256;
    int X = ceil((float)length / (workGroups*workGroupSize));

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
    deleteArray(result, type);
}

cl::size_t<3> createRegion(unsigned int x, unsigned int y, unsigned int z) {
    cl::size_t<3> region;
    region[0] = x;
    region[1] = y;
    region[2] = z;
    return region;
}
cl::size_t<3> createRegion(Vector3ui size) {
    return createRegion(size.x(), size.y(), size.z());
}

cl::size_t<3> createOrigoRegion() {
    cl::size_t<3> region;
    region[0] = 0;
    region[1] = 0;
    region[2] = 0;
    return region;
}

std::string getCLErrorString(cl_int err) {
    switch (err) {
    case CL_SUCCESS:
        return std::string("Success!");
    case CL_DEVICE_NOT_FOUND:
        return std::string("Device not found.");
    case CL_DEVICE_NOT_AVAILABLE:
        return std::string("Device not available");
    case CL_COMPILER_NOT_AVAILABLE:
        return std::string("Compiler not available");
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return std::string("Memory object allocation failure");
    case CL_OUT_OF_RESOURCES:
        return std::string("Out of resources");
    case CL_OUT_OF_HOST_MEMORY:
        return std::string("Out of host memory");
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return std::string("Profiling information not available");
    case CL_MEM_COPY_OVERLAP:
        return std::string("Memory copy overlap");
    case CL_IMAGE_FORMAT_MISMATCH:
        return std::string("Image format mismatch");
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return std::string("Image format not supported");
    case CL_BUILD_PROGRAM_FAILURE:
        return std::string("Program build failure");
    case CL_MAP_FAILURE:
        return std::string("Map failure");
    case CL_INVALID_VALUE:
        return std::string("Invalid value");
    case CL_INVALID_DEVICE_TYPE:
        return std::string("Invalid device type");
    case CL_INVALID_PLATFORM:
        return std::string("Invalid platform");
    case CL_INVALID_DEVICE:
        return std::string("Invalid device");
    case CL_INVALID_CONTEXT:
        return std::string("Invalid context");
    case CL_INVALID_QUEUE_PROPERTIES:
        return std::string("Invalid queue properties");
    case CL_INVALID_COMMAND_QUEUE:
        return std::string("Invalid command queue");
    case CL_INVALID_HOST_PTR:
        return std::string("Invalid host pointer");
    case CL_INVALID_MEM_OBJECT:
        return std::string("Invalid memory object");
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return std::string("Invalid image format descriptor");
    case CL_INVALID_IMAGE_SIZE:
        return std::string("Invalid image size");
    case CL_INVALID_SAMPLER:
        return std::string("Invalid sampler");
    case CL_INVALID_BINARY:
        return std::string("Invalid binary");
    case CL_INVALID_BUILD_OPTIONS:
        return std::string("Invalid build options");
    case CL_INVALID_PROGRAM:
        return std::string("Invalid program");
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return std::string("Invalid program executable");
    case CL_INVALID_KERNEL_NAME:
        return std::string("Invalid kernel name");
    case CL_INVALID_KERNEL_DEFINITION:
        return std::string("Invalid kernel definition");
    case CL_INVALID_KERNEL:
        return std::string("Invalid kernel");
    case CL_INVALID_ARG_INDEX:
        return std::string("Invalid argument index");
    case CL_INVALID_ARG_VALUE:
        return std::string("Invalid argument value");
    case CL_INVALID_ARG_SIZE:
        return std::string("Invalid argument size");
    case CL_INVALID_KERNEL_ARGS:
        return std::string("Invalid kernel arguments");
    case CL_INVALID_WORK_DIMENSION:
        return std::string("Invalid work dimension");
    case CL_INVALID_WORK_GROUP_SIZE:
        return std::string("Invalid work group size");
    case CL_INVALID_WORK_ITEM_SIZE:
        return std::string("Invalid work item size");
    case CL_INVALID_GLOBAL_OFFSET:
        return std::string("Invalid global offset");
    case CL_INVALID_EVENT_WAIT_LIST:
        return std::string("Invalid event wait list");
    case CL_INVALID_EVENT:
        return std::string("Invalid event");
    case CL_INVALID_OPERATION:
        return std::string("Invalid operation");
    case CL_INVALID_GL_OBJECT:
        return std::string("Invalid OpenGL object");
    case CL_INVALID_BUFFER_SIZE:
        return std::string("Invalid buffer size");
    case CL_INVALID_MIP_LEVEL:
        return std::string("Invalid mip-map level");
    default:
        return std::string("Unknown");
    }
}

std::string replace(std::string str, std::string find, std::string replacement) {
    while(true) {
        int pos = str.find(find);
        if(pos == std::string::npos)
            break;
        str.replace(pos, find.size(), replacement);
    }

    return str;
}

std::vector<std::string> split(const std::string& input, const std::string& delimiter) {
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(delimiter);
    std::sregex_token_iterator
            first{input.begin(), input.end(), re, -1},
            last;
    return {first, last};
}

void loadPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar) {
	float ymax = zNear * tan(fovy * M_PI / 360.0);
	float ymin = -ymax;
	float xmin = ymin * aspect;
	float xmax = ymax * aspect;
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void createDirectory(std::string path) {
    int error = 0;
#if defined(_WIN32)
    error = _mkdir(path.c_str()); // can be used on Windows
#else
    mode_t nMode = 0733; // UNIX style permissions
    error = mkdir(path.c_str(), nMode); // can be used on non-Windows
#endif
    if (error != 0) {
        if(error == EEXIST) {
            throw ExistException("Unable to create directory at " + path + ": Directory already exists.");
        } else if(error == ENOENT) {
            throw DoesNotExistException("Unable to create directory at " + path + ": Path was not found.");
        } else {
            throw Exception("Unable to create directory at " + path + ": Unknown error.");
        }
    }
}


void createDirectories(std::string path) {
    // Replace \ with / so that this will work on windows
    path = replace(path, "\\", "/");
    std::vector<std::string> directories = split(path, "/");
    std::vector<std::string> filteredDirectories;

    // Fix any path with /../ in path
    for(int i = 0; i < directories.size(); ++i) {
        trim(directories[i]);
        //std::cout << directories[i] << std::endl;
        if(directories[i] == "..") {
            // Pop previous
            filteredDirectories.pop_back();
        } else if(directories[i].size() > 0) {
            filteredDirectories.push_back(directories[i]);
        }
    }

    directories = filteredDirectories;
#ifdef _WIN32
    std::string currentPath = directories[0];
#else
    std::string currentPath = "/" + directories[0];
#endif
    // Create each directory needed
    for(int i = 1; i < directories.size(); ++i) {
        currentPath += "/" + directories[i];
        try {
            createDirectory(currentPath);
        } catch(ExistException &e) {
            continue;
        } catch(Exception &e) {
            continue;
        }
    }
}

std::string currentDateTime(std::string format) {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), format.c_str(), &tstruct);

    return buf;
}

} // end namespace fast
