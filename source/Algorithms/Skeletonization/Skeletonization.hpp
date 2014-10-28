#ifndef SKELETONIZATION_HPP
#define SKELETONIZATION_HPP

#include "ProcessObject.hpp"
#include "Image.hpp"

namespace fast {

class Skeletonization : public ProcessObject {
    FAST_OBJECT(Skeletonization)
    public:
        void setInput(ImageData::pointer input);
        ImageData::pointer getOutput();
    private:
        Skeletonization();
        void execute();

        ImageData::pointer mInput;
        ImageData::pointer mOutput;
        OpenCLDevice::pointer mDevice;

};

} // end namespace fast

#endif
