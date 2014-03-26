#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "ImageData.hpp"
#include "OpenCLManager.hpp"
#include "ExecutionDevice.hpp"
#include "OpenCLImageAccess2D.hpp"
#include "ImageAccess2D.hpp"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
namespace fast {

class Image2D: public ImageData {
    FAST_OBJECT(Image2D)
    public:
        void createImage(unsigned int width, unsigned int height);
        void createImage(unsigned int width, unsigned int height, const float * data);
        void createImage(cl::Image2D* clImage, OpenCLDevice::pointer device);
        OpenCLImageAccess2D getOpenCLImageAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess2D getImageAccess(accessType type);
        ~Image2D();
    private:
        Image2D();
        boost::unordered_map<OpenCLDevice::pointer, cl::Image2D*> mCLImages;
        boost::unordered_map<OpenCLDevice::pointer, bool> mCLImagesIsUpToDate;
        boost::unordered_map<OpenCLDevice::pointer, bool> mCLImagesAccess;
        std::vector<OpenCLDevice::pointer> mCLDevices;
        void * mHostData;
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        bool mHostDataIsBeingAccessed;
        void execute(){};
        bool isDataModified();
        void updateOpenCLImageData(OpenCLDevice::pointer device);
        void updateHostData();
        void setAllDataToOutOfDate();
        bool isAnyDataBeingAccessed();
        void transferCLImageFromHost(OpenCLDevice::pointer device);
        void transferCLImageToHost(OpenCLDevice::pointer device);
        bool isInitialized();
};

} // end namespace fast

#endif
