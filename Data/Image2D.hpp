#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "ImageData.hpp"
#include "OpenCLManager.hpp"
#include "ExecutionDevice.hpp"
#include "OpenCLImageAccess2D.hpp"
#include "ImageAccess2D.hpp"
namespace fast {

class Image2D: public ImageData {
    FAST_OBJECT(Image2D)
    public:
        void createImage(unsigned int width, unsigned int height);
        void setOpenCLImage(cl::Image2D* clImage, OpenCLDevice::pointer device);
        OpenCLImageAccess2D getOpenCLImageAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess2D getImageAccess(accessType type);
        ~Image2D();
    private:
        Image2D();
        // These two vectors should be equal in size and have entries
        // that correspond to eachother
        std::vector<cl::Image2D*> mCLImages;
        std::vector<OpenCLDevice::pointer> mCLDevices;
        void * mHostData;
        void execute(){};
};

} // end namespace fast

#endif
