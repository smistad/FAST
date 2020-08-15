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

cl::Program OpenCLProgram::build(std::shared_ptr<OpenCLDevice> device,
        std::string buildOptions) {
    if(mSourceFilename == "")
        throw Exception("No source filename was given to OpenCLProgram. Therefore build operation is not possible.");

    // Add fast_3d_image_writes flag if it is supported
    if(device->isWritingTo3DTexturesSupported()) {
        if(buildOptions.size() > 0)
            buildOptions += " ";
        buildOptions += "-Dfast_3d_image_writes";
    }

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

bool OpenCLProgram::buildExists(std::shared_ptr<OpenCLDevice> device,
        std::string buildOptions) const {
    bool hasBuild = true;
    if(mOpenCLPrograms.count(device) == 0) {
        hasBuild = false;
    } else {
        const std::map<std::string, cl::Program> programs = mOpenCLPrograms.at(device);
        if(programs.count(buildOptions) == 0)
            hasBuild = false;
    }

    return hasBuild;
}

} // end namespace fast
