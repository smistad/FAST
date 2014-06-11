#ifndef SURFACEEXTRACTION_HPP_
#define SURFACEEXTRACTION_HPP_

#include "ProcessObject.hpp"
#include "ImageData.hpp"
#include "Surface.hpp"

namespace fast {

class SurfaceExtraction : public ProcessObject {
    FAST_OBJECT(SurfaceExtraction)
    public:
        void setThreshold(float threshold);
        void setInput(ImageData::pointer input);
        void setDevice(OpenCLDevice::pointer device);
        Surface::pointer getOutput();
    private:
        SurfaceExtraction();
        void execute();

        ImageData::pointer mInput;
        Surface::pointer mOutput;
        float mThreshold;
        OpenCLDevice::pointer mDevice;
        unsigned int mHPSize;
};

} // end namespace fast




#endif /* SURFACEEXTRACTION_HPP_ */
