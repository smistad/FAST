#include "Image2D.hpp"
#include "HelperFunctions.hpp"
#include "Exception.hpp"
using namespace fast;

bool Image2D::isDataModified() {
    if(!mHostDataIsUpToDate)
        return true;

    boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
        if(it->second == false)
            return true;
    }

    return false;
}

bool Image2D::isAnyDataBeingAccessed() {
    if(mHostDataIsBeingAccessed)
        return true;

    boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(it = mCLImagesAccess.begin(); it != mCLImagesAccess.end(); it++) {
        if(it->second)
            return true;
    }

    return false;
}

void Image2D::transferCLImageFromHost(OpenCLDevice::pointer device) {
     device->getCommandQueue().enqueueWriteImage(
            *mCLImages[device],
            CL_TRUE,
            oul::createOrigoRegion(),
            oul::createRegion(mWidth,mHeight,1),
            0,0,
            mHostData
    );
}

void Image2D::transferCLImageToHost(OpenCLDevice::pointer device) {
    device->getCommandQueue().enqueueReadImage(
                *mCLImages[device],
                CL_TRUE,
                oul::createOrigoRegion(),
                oul::createRegion(mWidth,mHeight,1),
                0,0,
                mHostData
            );
}

void Image2D::updateOpenCLImageData(OpenCLDevice::pointer device) {

    // If data exist on device and is up to date do nothing
    if(mCLImagesIsUpToDate.count(device) > 0 && mCLImagesIsUpToDate[device] == true)
        return;

    if(mCLImagesIsUpToDate.count(device) == 0) {
        // Data is not on device, create it
        // TODO type support
        cl::Image2D * newImage = new cl::Image2D(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_R, CL_FLOAT),
                mWidth, mHeight
        );

        mCLImages[device] = newImage;
        mCLImagesIsUpToDate[device] = true;
        mCLImagesAccess[device] = true;
        mCLDevices.push_back(device);
    }

    // Find which data is up to date
    bool updated = false;
    if(mHostDataIsUpToDate) {
        // Transfer host data to this device
        transferCLImageFromHost(device);
        updated = true;
    } else {
        boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
        for(it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
            if(it->second == true) {
                // TODO: Transfer from this device(it->first) to device
                transferCLImageToHost(it->first);
                transferCLImageFromHost(device);
                mHostDataIsUpToDate = true;
                updated = true;
                break;
            }
        }
    }

    if(!updated)
        throw Exception("Data was not updated because no data was marked as up to date");
}

void Image2D::updateHostData() {
    // It is the host data that has been modified, no need to update
    if(mHostDataIsUpToDate)
        return;

    if(!mHostHasData) {
        // Data is not initialized, do that first
        // TODO type support here
        mHostData = new float[mWidth*mHeight];
    }

    if(mCLImages.size() > 0) {
        // Find which data is up to date
        bool updated = false;
        boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
        for(it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
            if(it->second == true) {
                // TODO: transfer from this device to host
                transferCLImageToHost(it->first);

                updated = true;
                break;
            }
        }

        if(!updated)
            throw Exception("Data was not updated because no data was marked as up to date");
    }
}

void Image2D::createImage(cl::Image2D *clImage, OpenCLDevice::pointer device) {
    if(isInitialized())
        throw Exception("Can not use createImage on an object that is already initialized");

    mCLImages[device] = clImage;
    mCLImagesIsUpToDate[device] = true;
    mCLImagesAccess[device] = false;
    mCLDevices.push_back(device);
    mWidth = clImage->getImageInfo<CL_IMAGE_WIDTH>();
    mHeight = clImage->getImageInfo<CL_IMAGE_HEIGHT>();
}

void Image2D::setAllDataToOutOfDate() {
    mHostDataIsUpToDate = false;
    boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
        it->second = false;
    }
}

OpenCLImageAccess2D Image2D::getOpenCLImageAccess(
        accessType type,
        OpenCLDevice::pointer device) {
    // Check for write access
    if(type == ACCESS_READ_WRITE) {
        if(isAnyDataBeingAccessed()) {
            throw Exception("Trying to get write access to an object that is already being accessed");
        }
        setAllDataToOutOfDate();
        mCLImagesIsUpToDate[device] = true;
    }
    mCLImagesAccess[device] = true;
    updateOpenCLImageData(device);

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLImageAccess2D(mCLImages[device], &mCLImagesAccess[device]);
}

Image2D::~Image2D() {
    // Delete all images
    boost::unordered_map<OpenCLDevice::pointer, cl::Image2D*>::iterator it;
    for(it = mCLImages.begin(); it != mCLImages.end(); it++) {
        delete it->second;
    }

    // TODO: type support needed here
    delete[] (float*)mHostData;
}

Image2D::Image2D() {
    mHostData = NULL;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
    mHostDataIsBeingAccessed = false;
}

ImageAccess2D Image2D::getImageAccess(accessType type) {
    // TODO: this method is currently just a fixed hack

    if(type == ACCESS_READ_WRITE) {
        if(isAnyDataBeingAccessed()) {
            throw Exception("Trying to get write access to an object that is already being accessed");
        }
        // Set modified to true since it wants write access
        setAllDataToOutOfDate();
        mHostDataIsUpToDate = true;
        mHostDataIsBeingAccessed = true;
    }
    updateHostData();

    return ImageAccess2D(mHostData, &mHostDataIsBeingAccessed);
}

void Image2D::createImage(unsigned int width, unsigned int height) {
    if(isInitialized())
        throw Exception("Can not use createImage on an object that is already initialized.");
    mWidth = width;
    mHeight = height;
}

void Image2D::createImage(
        unsigned int width,
        unsigned int height,
        const float* data) {
    if(isInitialized())
        throw Exception("Can not use createImage on an object that is already initialized.");

    mWidth = width;
    mHeight = height;
    memcpy(mHostData,data,sizeof(float)*width*height);
    mHostDataIsUpToDate = true;
}

bool Image2D::isInitialized() {
    return mCLImages.size() > 0 || mHostHasData;
}
