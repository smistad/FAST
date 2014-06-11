#include "SurfaceExtraction.hpp"
#include "DeviceManager.hpp"

namespace fast {

void SurfaceExtraction::setThreshold(float threshold) {
    mThreshold = threshold;
    mIsModified = true;
}

void SurfaceExtraction::setInput(ImageData::pointer input) {
    mInput = input;
    setParent(mInput);
    mIsModified = true;
}

Surface::pointer SurfaceExtraction::getOutput() {
    return mOutput;
}

void SurfaceExtraction::setDevice(OpenCLDevice::pointer device) {
    mDevice = device;
}

void SurfaceExtraction::execute() {
}

SurfaceExtraction::SurfaceExtraction() {
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultComputationDevice());
    mThreshold = 0.0f;
    mHPSize = 0;
}


} // end namespace fast

