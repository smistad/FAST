#include "HelperFunctions.hpp"
#include "Exceptions.hpp"

#include <fstream>
#include <sstream>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#endif
#endif
#include <iostream>

namespace oul {

cl_context_properties * createInteropContextProperties(
        const cl::Platform &platform,
        cl_context_properties OpenGLContext,
        cl_context_properties display) {
#if defined(__APPLE__) || defined(__MACOSX)
    // Apple (untested)
    // TODO: create GL context for Apple
std::cout << "trying to get share group of gl context" << std::endl;
CGLSetCurrentContext((CGLContextObj)OpenGLContext);
CGLShareGroupObj shareGroup = CGLGetShareGroup((CGLContextObj)OpenGLContext);
if(shareGroup == NULL)
throw Exception("Not able to get sharegroup");
    cl_context_properties * cps = new cl_context_properties[3];
    cps[0] = CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
    cps[1] = (cl_context_properties)shareGroup;
    cps[2] = 0;
std::cout << "success " << shareGroup << std::endl;

#else
#ifdef _WIN32
    // Windows
    cl_context_properties * cps = new cl_context_properties[7];
    cps[0] = CL_GL_CONTEXT_KHR;
    cps[1] = OpenGLContext;
    cps[2] = CL_WGL_HDC_KHR;
    cps[3] = display;
    cps[4] = CL_CONTEXT_PLATFORM;
    cps[5] = (cl_context_properties) (platform)(); 
	cps[6] = 0;
#else
    cl_context_properties * cps = new cl_context_properties[7];
    cps[0] = CL_GL_CONTEXT_KHR;
    cps[1] = OpenGLContext;
    cps[2] = CL_GLX_DISPLAY_KHR;
    cps[3] = display;
    cps[4] = CL_CONTEXT_PLATFORM;
    cps[5] = (cl_context_properties) (platform)(); 
	cps[6] = 0;
#endif
#endif
    return cps;
}

cl::size_t<3> createRegion(unsigned int x, unsigned int y, unsigned int z) {
    cl::size_t<3> region;
    region[0] = x;
    region[1] = y;
    region[2] = z;
    return region;
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

std::string readFile(std::string filename) {
    std::string retval = "";

    std::ifstream sourceFile(filename.c_str(), std::fstream::in);
    if (sourceFile.fail())
        throw Exception("Failed to open OpenCL source file.");

    std::stringstream stringStream;

    stringStream.str("");

    stringStream << sourceFile.rdbuf();

    std::string sourceCode = stringStream.str();

    retval = sourceCode;
    //retval = std::string(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));

    return retval;
}
} //namespace oul
