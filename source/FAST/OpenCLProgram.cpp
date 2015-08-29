#include "OpenCLProgram.hpp"
#include "ExecutionDevice.hpp"

namespace fast {

void OpenCLProgram::setName(std::string name) {
    mName = name;
}

std::string OpenCLProgram::getName() const {
    return mName;
}

void OpenCLProgram::setSourceFilename(std::string filename) {
    mSourceFilename = filename;
}

std::string OpenCLProgram::getSourceFilename() const {
    return mSourceFilename;
}

cl::Program OpenCLProgram::build(SharedPointer<OpenCLDevice> device,
        std::string buildOptions) {
    if(mSourceFilename == "")
        throw Exception("No source filename was given to OpenCLProgram. Therefore build operation is not possible.");

    if(buildExists(device, buildOptions))
        return mOpenCLPrograms[device][buildOptions];

    std::string programName = mSourceFilename + buildOptions;
    // Only create program if it doesn't exist for this device from before
    if(!device->hasProgram(programName))
        device->createProgramFromSourceWithName(programName, mSourceFilename, buildOptions);
    return device->getProgram(programName);
}

OpenCLProgram::OpenCLProgram() {
    mName = "";
    mSourceFilename = "";
}

bool OpenCLProgram::buildExists(SharedPointer<OpenCLDevice> device,
        std::string buildOptions) const {
    bool hasBuild = true;
    if(mOpenCLPrograms.count(device) == 0) {
        hasBuild = false;
    } else {
        const boost::unordered_map<std::string, cl::Program> programs = mOpenCLPrograms.at(device);
        if(programs.count(buildOptions) == 0)
            hasBuild = false;
    }

    return hasBuild;
}

} // end namespace fast
