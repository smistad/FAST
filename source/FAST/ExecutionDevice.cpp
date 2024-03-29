#include "FAST/ExecutionDevice.hpp"
#include "FAST/RuntimeMeasurementManager.hpp"
#include "FAST/Utility.hpp"
#include <mutex>
#include <fstream>
#include "FAST/Config.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <CL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#endif
#endif

// Some include files for reading the modification date of files
#ifdef WIN32
#include <windows.h>
#undef min
#undef max
#else
#include <sys/stat.h>
#include <time.h>
#endif

namespace fast {

inline cl_context_properties* createInteropContextProperties(
        const cl::Platform &platform,
        cl_context_properties OpenGLContext,
        cl_context_properties display) {
#if defined(__APPLE__) || defined(__MACOSX)
    // Apple (untested)
    // TODO: create GL context for Apple
CGLSetCurrentContext((CGLContextObj)OpenGLContext);
CGLShareGroupObj shareGroup = CGLGetShareGroup((CGLContextObj)OpenGLContext);
if(shareGroup == NULL)
throw Exception("Not able to get sharegroup");
    cl_context_properties * cps = new cl_context_properties[3];
    cps[0] = CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
    cps[1] = (cl_context_properties)shareGroup;
    cps[2] = 0;

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

cl::CommandQueue OpenCLDevice::getCommandQueue() {
    return getQueue(0);
}

cl::Device OpenCLDevice::getDevice() {
    return OpenCLDevice::getDevice(0);
}

OpenCLPlatformVendor OpenCLDevice::getPlatformVendor() {
    std::string platformVendor = getPlatform().getInfo<CL_PLATFORM_VENDOR>();
    OpenCLPlatformVendor retval;
    if (platformVendor.find("Advanced Micro Devices") != std::string::npos || platformVendor.find("AMD") != std::string::npos) {
        retval = PLATFORM_VENDOR_AMD;
    } else if (platformVendor.find("Apple") != std::string::npos) {
        retval = PLATFORM_VENDOR_APPLE;
    } else if (platformVendor.find("Intel") != std::string::npos) {
        retval = PLATFORM_VENDOR_INTEL;
    } else if (platformVendor.find("NVIDIA") != std::string::npos) {
        retval = PLATFORM_VENDOR_NVIDIA;
    } else if(platformVendor.find("Portable Computing Language") != std::string::npos) {
        retval = PLATFORM_VENDOR_POCL;
	} else {
        retval = PLATFORM_VENDOR_UNKNOWN;
	}
    return retval;
}


bool OpenCLDevice::isWritingTo3DTexturesSupported() {
#ifdef WIN32
    if(getPlatformVendor() == PLATFORM_VENDOR_NVIDIA) {
        // 3D image writes on windows for nvidia is buggy, disable it
        //reportWarning() << "Disabling 3D image writes because unstable with latest nvidia drivers on windows" << reportEnd();
        return false;
    } else {
		return OpenCLDevice::getDevice(0).getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
    }
#else
    return OpenCLDevice::getDevice(0).getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
#endif
}

OpenCLDevice::~OpenCLDevice() {
     //reportInfo() << "DESTROYING opencl device object..." << Reporter::end();
     // Make sure that all queues are finished
     getQueue(0).finish();
}

OpenCLDevice::OpenCLDevice() {
    mIsHost = false;
}


std::mutex buildBinaryMutex; // a global mutex

bool OpenCLDevice::isImageFormatSupported(cl_channel_order order, cl_channel_type type, cl_mem_object_type imageType) {
    std::vector<cl::ImageFormat> formats;
    context.getSupportedImageFormats(CL_MEM_READ_WRITE, imageType, &formats);

    bool isSupported = false;
    for(int i = 0; i < formats.size(); i++) {
    	if(formats[i].image_channel_order == order && formats[i].image_channel_data_type == type) {
    		isSupported = true;
    		break;
    	}
    }

    return isSupported;
}


std::string readFile(std::string filename) {
    std::string retval = "";

    std::ifstream sourceFile(filename.c_str(), std::fstream::in);
    if (sourceFile.fail())
        throw Exception("Failed to open OpenCL source file: " + filename);

    std::stringstream stringStream;

    stringStream.str("");

    stringStream << sourceFile.rdbuf();

    std::string sourceCode = stringStream.str();

    retval = sourceCode;
    //retval = std::string(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));

    return retval;
}

OpenCLDevice::OpenCLDevice(std::vector<cl::Device> devices, unsigned long* OpenGLContext) {
    runtimeManager = RuntimeMeasurementsManager::New();
    mIsHost = false;
    mGLContext = OpenGLContext;
    profilingEnabled = false;
	if(profilingEnabled)
		runtimeManager->enable();
	else
		runtimeManager->disable();

    this->devices = devices;
    // TODO: make sure that all devices have the same platform
    this->platform = devices[0].getInfo<CL_DEVICE_PLATFORM>();

    // TODO: OpenGL interop properties
    // TODO: must check that a OpenGL context and display is available
    // TODO: Use current context and display, or take this as input??
    cl_context_properties * cps;
    if(OpenGLContext != NULL) {
#if defined(__APPLE__) || defined(__MACOSX)
        cps = createInteropContextProperties(
                this->platform,
                (cl_context_properties)OpenGLContext,
                NULL
        );
#else
#ifdef _WIN32
        cps = createInteropContextProperties(
                this->platform,
                (cl_context_properties)OpenGLContext,
                (cl_context_properties)wglGetCurrentDC()
        );
#else
        reportInfo() << "Created OpenCL context with glX context " << OpenGLContext << reportEnd();
        Display * display = XOpenDisplay(0);
        reportInfo() << "and X display " << display << reportEnd();
        cps = createInteropContextProperties(
                this->platform,
                (cl_context_properties)OpenGLContext,
                (cl_context_properties)display
        );
#endif
#endif
    } else {
        cps = new cl_context_properties[3];
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties)(platform)();
        cps[2] = 0;
    }
    this->context = cl::Context(devices,cps);
    delete[] cps;

    // Create a command queue for each device
    for(int i = 0; i < devices.size(); i++) {
        if(profilingEnabled) {
            this->queues.push_back(cl::CommandQueue(context, devices[i], CL_QUEUE_PROFILING_ENABLE));
        } else {
            this->queues.push_back(cl::CommandQueue(context, devices[i]));
        }
    }
}

int OpenCLDevice::createProgramFromSource(std::string filename, std::string buildOptions, bool useCaching) {
    std::lock_guard<std::mutex> lock(buildBinaryMutex);
    cl::Program program;
    if(useCaching) {
        program = buildProgramFromBinary(filename, buildOptions);
    } else {
        std::string sourceCode = readFile(filename);
        // If 3d image writes is supported, append the enable line to all source files (fix error on Intel devices)
        if(isWritingTo3DTexturesSupported())
            sourceCode = "#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable\n\n" + sourceCode;
        cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));
        program = buildSources(source, buildOptions);
    }
    programs.push_back(program);
    return programs.size()-1;
}

/**
 * Compile several source files together
 */
int OpenCLDevice::createProgramFromSource(std::vector<std::string> filenames, std::string buildOptions) {
    std::lock_guard<std::mutex> lock(buildBinaryMutex);
    // Do this in a weird way, because the the logical way does not work.
    std::string sourceCode = readFile(filenames[0]);
    if(isWritingTo3DTexturesSupported())
        sourceCode = "#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable\n\n" + sourceCode;
    cl::Program::Sources sources(filenames.size(), std::make_pair(sourceCode.c_str(), sourceCode.length()));
    for(int i = 1; i < filenames.size(); i++) {
        std::string sourceCode2 = readFile(filenames[i]);
        // If 3d image writes is supported, append the enable line to all source files (fix error on Intel devices)
        if(isWritingTo3DTexturesSupported())
            sourceCode2 = "#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable\n\n" + sourceCode2;
        sources[i] = std::make_pair(sourceCode2.c_str(), sourceCode2.length());
    }

    cl::Program program = buildSources(sources, buildOptions);
    programs.push_back(program);
    return programs.size()-1;
}

int OpenCLDevice::createProgramFromString(std::string code, std::string buildOptions) {
    std::lock_guard<std::mutex> lock(buildBinaryMutex);
    cl::Program::Sources source(1, std::make_pair(code.c_str(), code.length()));

    cl::Program program = buildSources(source, buildOptions);
    programs.push_back(program);
    return programs.size()-1;
}

cl::Program OpenCLDevice::getProgram(unsigned int i) {
    std::lock_guard<std::mutex> lock(buildBinaryMutex);
    return programs.at(i);
}

cl::CommandQueue OpenCLDevice::getQueue(unsigned int i) {
    return queues[i];
}


cl::Device OpenCLDevice::getDevice(unsigned int i) {
    return devices[i];
}

cl::Device getDevice(cl::CommandQueue queue){
	return queue.getInfo<CL_QUEUE_DEVICE>();
}

cl::Context OpenCLDevice::getContext() {
    return context;
}

cl::Platform OpenCLDevice::getPlatform() {
    return platform;
}


cl::Program OpenCLDevice::buildSources(cl::Program::Sources source, std::string buildOptions) {
    // Make program of the source code in the context
    cl::Program program = cl::Program(context, source);

    // Build program for the context devices
    try{
        program.build(devices, buildOptions.c_str());
    } catch(cl::Error &error) {
        if(error.err() == CL_BUILD_PROGRAM_FAILURE) {
            for(unsigned int i=0; i<devices.size(); i++){
            	reportError() << "Build log, device " << i << "\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]) << Reporter::end();
            }
        }
        reportError() << getCLErrorString(error.err()) << Reporter::end();

        throw error;
    }
    return program;
}

RuntimeMeasurementsManager::pointer OpenCLDevice::getRunTimeMeasurementManager(){
	return runtimeManager;
}


cl::Program OpenCLDevice::writeBinary(std::string filename, std::string buildOptions) {
    // Build program from source file and store the binary file
    std::string sourceCode = readFile(filename);
    // If 3d image writes is supported, append the enable line to all source files (fix error on Intel devices)
    if(isWritingTo3DTexturesSupported())
        sourceCode = "#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable\n\n" + sourceCode;

    cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));
    cl::Program program = buildSources(source, buildOptions);

    std::vector<std::size_t> binarySizes;
    binarySizes = program.getInfo<CL_PROGRAM_BINARY_SIZES>();

    std::vector<std::vector<uchar>> binaries;
    binaries = program.getInfo<CL_PROGRAM_BINARIES>();

    std::string deviceName = getDevice(0).getInfo<CL_DEVICE_NAME>();
    std::size_t hash = std::hash<std::string>{}(buildOptions + deviceName);
    std::string binaryPath = Config::getKernelBinaryPath();
    // TODO fix this for both install and build
    std::string kernelSourcePath = Config::getKernelSourcePath();
    std::string binaryFilename = binaryPath + filename.substr(kernelSourcePath.size()) + "_" + std::to_string(hash) + ".bin";
    std::string cacheFilename = binaryPath + filename.substr(kernelSourcePath.size()) + "_" + std::to_string(hash) + ".cache";

    // Create directories if they don't exist
    if(binaryFilename.rfind("/") != std::string::npos) {
        std::string directoryPath = binaryFilename.substr(0, binaryFilename.rfind("/"));
//        QDir dir(directoryPath.c_str());
//        if (!dir.exists()) {
//            dir.mkpath(".");
//        }
        createDirectories(directoryPath);
    }
    FILE * file = fopen(binaryFilename.c_str(), "wb");
    if(!file)
        throw Exception("Could not write kernel binary to file: " + binaryFilename);
    fwrite(binaries[0].data(), sizeof(char), (int)binarySizes[0], file);
    fclose(file);

    // Write cache file
    FILE * cacheFile = fopen(cacheFilename.c_str(), "w");
    auto modifiedDate = getModifiedDate(filename);
    std::string timeStr = modifiedDate + "\n";
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    timeStr += devices[0].getInfo<CL_DEVICE_NAME>() + "\n";
    timeStr += buildOptions;
    fwrite(timeStr.c_str(), sizeof(char), timeStr.size(), cacheFile);
    fclose(cacheFile);

    return program;
}

cl::Program OpenCLDevice::readBinary(std::string filename) {
    std::ifstream sourceFile(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    std::string sourceCode(
        std::istreambuf_iterator<char>(sourceFile),
        (std::istreambuf_iterator<char>()));
    cl::Program::Binaries binary(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    if(devices.size() > 1) {
        // Currently only support compiling for one device
        cl::Device device = devices[0];
        devices.clear();
        devices.push_back(device);
    }

    cl::Program program = cl::Program(context, devices, binary);

    // Build program for these specific devices
    program.build(devices);
    return program;
}

cl::Program OpenCLDevice::buildProgramFromBinary(std::string filename, std::string buildOptions) {
    cl::Program program;

    std::string deviceName = getDevice(0).getInfo<CL_DEVICE_NAME>();
    std::size_t hash = std::hash<std::string>{}(buildOptions + deviceName);
    std::string binaryPath = Config::getKernelBinaryPath();
    std::string kernelSourcePath = Config::getKernelSourcePath();
    std::string binaryFilename = binaryPath + filename.substr(kernelSourcePath.size()) + "_" + std::to_string(hash) + ".bin";
    std::string cacheFilename = binaryPath + filename.substr(kernelSourcePath.size()) + "_" + std::to_string(hash) + ".cache";

    // Check if a binary file exists
    std::ifstream binaryFile(binaryFilename.c_str(), std::ios_base::binary | std::ios_base::in);
    if(binaryFile.fail()) {
        program = writeBinary(filename, buildOptions);
    } else {
        // Compare modified dates of binary file and source file

        // Read cache file
        std::ifstream cacheFile(cacheFilename.c_str());
        std::string cache(
            std::istreambuf_iterator<char>(cacheFile),
            (std::istreambuf_iterator<char>()));
        auto lines = split(cache, "\n");

        bool outOfDate = true;
        bool wrongDeviceID = true;
        bool buildOptionsChanged = true;
        if(lines.size() >= 2) {
            // Get modification date of file
            auto modifiedDate = getModifiedDate(filename);
            outOfDate = modifiedDate != lines[0];
            std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
            wrongDeviceID = lines[1] != devices[0].getInfo<CL_DEVICE_NAME>();
            if(lines.size() == 3)
                buildOptionsChanged = lines[2] != buildOptions;
        }

        if(outOfDate || wrongDeviceID || buildOptionsChanged) {
            Reporter::warning() << "Kernel binary " << filename.substr(kernelSourcePath.size()) << " is out of date. Compiling..." << Reporter::end();
            program = writeBinary(filename, buildOptions);
        } else {
            //std::cout << "Binary is not out of date." << std::endl;
            program = readBinary(binaryFilename);
        }
    }

    return program;
}

int OpenCLDevice::createProgramFromSourceWithName(
        std::string programName,
        std::string filename,
        std::string buildOptions) {
    programNames[programName] = createProgramFromSource(filename,buildOptions);
    return programNames[programName];
}

int OpenCLDevice::createProgramFromSourceWithName(
        std::string programName,
        std::vector<std::string> filenames,
        std::string buildOptions) {
    programNames[programName] = createProgramFromSource(filenames,buildOptions);
    return programNames[programName];
}

int OpenCLDevice::createProgramFromStringWithName(
        std::string programName,
        std::string code,
        std::string buildOptions) {
    programNames[programName] = createProgramFromString(code,buildOptions);
    return programNames[programName];
}

cl::Program OpenCLDevice::getProgram(std::string name) {
    std::lock_guard<std::mutex> lock(buildBinaryMutex);
    if(programNames.count(name) == 0) {
        std::string msg ="Could not find OpenCL program with the name" + name;
        throw Exception(msg.c_str(), __LINE__, __FILE__);
    }

    if(programs.size() <= programNames[name])
        throw Exception("OpenCL program does not exist");

    return programs[programNames[name]];
}

bool OpenCLDevice::hasProgram(std::string name) {
    return programNames.count(name) > 0;
}

bool OpenCLDevice::isOpenGLInteropSupported() {
    return mGLContext != nullptr;
}

} // end namespace fast
