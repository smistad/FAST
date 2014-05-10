#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "Exception.hpp"
using namespace fast;

bool Image::isDataModified() {
    if (!mHostDataIsUpToDate)
        return true;

    boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
            it++) {
        if (it->second == false)
            return true;
    }

    for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end();
            it++) {
        if (it->second == false)
            return true;
    }

    return false;
}

bool Image::isAnyDataBeingAccessed() {
    if (mHostDataIsBeingAccessed)
        return true;

    boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for (it = mCLImagesAccess.begin(); it != mCLImagesAccess.end(); it++) {
        if (it->second)
            return true;
    }

    for (it = mCLBuffersAccess.begin(); it != mCLBuffersAccess.end(); it++) {
        if (it->second)
            return true;
    }

    return false;
}

void * allocateDataArray(unsigned int voxels, DataType type, unsigned int nrOfComponents) {
    unsigned int size = voxels*nrOfComponents;
    void * data;
    switch(type) {
        fastSwitchTypeMacro(data = new FAST_TYPE[size])
    }

    return data;
}

// Pad data with 3 channels to 4 channels with 0
template <class T>
void * padData(T * data, unsigned int size) {
    T * newData = new T[size*4];
    for(unsigned int i = 0; i < size; i++) {
        newData[i*4] = data[i*3];
        newData[i*4+1] = data[i*3+1];
        newData[i*4+2] = data[i*3+2];
        newData[i*4+3] = 0;
    }
    return (void *)newData;
}

void * adaptDataToImage(void * data, unsigned int size, DataType type, unsigned int nrOfComponents) {
    // Because no OpenCL images support 3 channels,
    // the data has to be padded to 4 channels if the nr of components is 3
    if(nrOfComponents == 3) {
        switch(type) {
            fastSwitchTypeMacro(return padData<FAST_TYPE>((FAST_TYPE*)data, size))
        }
    }

    return data;
}

// Remove padding from a 3 channel data array created by padData
template <class T>
void * removePadding(T * data, unsigned int size) {
     T * newData = new T[size*3];
    for(unsigned int i = 0; i < size; i++) {
        newData[i*3] = data[i*4];
        newData[i*3+1] = data[i*4+1];
        newData[i*3+2] = data[i*4+2];
    }
    return (void *)newData;
}

void * adaptImageDataToHostData(void * data, unsigned int size, DataType type, unsigned int nrOfComponents) {
    // Because no OpenCL images support 3 channels,
    // the data has to be padded to 4 channels if the nr of components is 3.
    // This function removes that padding
    if(nrOfComponents == 3) {
        switch(type) {
            fastSwitchTypeMacro(return removePadding<FAST_TYPE>((FAST_TYPE*)data, size))
        }
    }
    return data;
}



void Image::transferCLImageFromHost(OpenCLDevice::pointer device) {
    // Special treatment for images with 3 components because an OpenCL image can only have 1, 2 or 4 channels
    if(mComponents == 3) {
        void * tempData = adaptDataToImage(mHostData, mWidth*mHeight*mDepth, mType, mComponents);
        if(mDimensions == 2) {
            device->getCommandQueue().enqueueWriteImage(*(cl::Image2D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, 1), 0,
                    0, tempData);
        } else {
            device->getCommandQueue().enqueueWriteImage(*(cl::Image3D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                    0, tempData);
        }
        deleteArray(tempData, mType);
    } else {
        if(mDimensions == 2) {
            device->getCommandQueue().enqueueWriteImage(*(cl::Image2D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, 1), 0,
                    0, mHostData);
        } else {
            device->getCommandQueue().enqueueWriteImage(*(cl::Image3D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                    0, mHostData);
        }
    }
}

void Image::transferCLImageToHost(OpenCLDevice::pointer device) {
    // Special treatment for images with 3 components because an OpenCL image can only have 1, 2 or 4 channels
    if(mComponents == 3) {
        void * tempData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents+1);
        if(mDimensions == 2) {
            device->getCommandQueue().enqueueReadImage(*(cl::Image2D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, 1), 0,
                    0, tempData);
        } else {
            device->getCommandQueue().enqueueReadImage(*(cl::Image3D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                    0, tempData);
        }
        mHostData = adaptImageDataToHostData(tempData,mWidth*mHeight*mDepth,mType,mComponents);
        deleteArray(tempData, mType);
    } else {
        if(!mHostHasData) {
            // Must allocate memory for host data
            mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
        }
        if(mDimensions == 2) {
            device->getCommandQueue().enqueueReadImage(*(cl::Image2D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, 1), 0,
                    0, mHostData);
        } else {
            device->getCommandQueue().enqueueReadImage(*(cl::Image3D*)mCLImages[device],
            CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                    0, mHostData);
        }
    }
}

void Image::updateOpenCLImageData(OpenCLDevice::pointer device) {

    // If data exist on device and is up to date do nothing
    if (mCLImagesIsUpToDate.count(device) > 0 && mCLImagesIsUpToDate[device]
            == true)
        return;

    if (mCLImagesIsUpToDate.count(device) == 0) {
        // Data is not on device, create it
        cl::Image * newImage;
        if(mDimensions == 2) {
            newImage = new cl::Image2D(device->getContext(),
            CL_MEM_READ_WRITE, getOpenCLImageFormat(mType,mComponents), mWidth, mHeight);
        } else {
            newImage = new cl::Image3D(device->getContext(),
            CL_MEM_READ_WRITE, getOpenCLImageFormat(mType,mComponents), mWidth, mHeight, mDepth);
        }

        mCLImages[device] = newImage;
        mCLImagesIsUpToDate[device] = true;
    }

    // Find which data is up to date
    bool updated = false;
    if (mHostDataIsUpToDate) {
        // Transfer host data to this device
        transferCLImageFromHost(device);
        updated = true;
    } else {
        boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
        for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
                it++) {
            if (it->second == true) {
                // Transfer from this device(it->first) to device
                transferCLImageToHost(it->first);
                transferCLImageFromHost(device);
                mHostDataIsUpToDate = true;
                updated = true;
                break;
            }
        }
        for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end();
                it++) {
            if (it->second == true) {
                // Transfer from this device(it->first) to device
                transferCLBufferToHost(it->first);
                transferCLImageFromHost(device);
                mHostDataIsUpToDate = true;
                updated = true;
                break;
            }
        }
    }

    if (!updated)
        throw Exception(
                "Data was not updated because no data was marked as up to date");
}

OpenCLBufferAccess Image::getOpenCLBufferAccess(
        accessType type,
        OpenCLDevice::pointer device) {

    updateOpenCLBufferData(device);
    // Check for write access
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        setAllDataToOutOfDate();
    }
    mCLBuffersAccess[device] = true;
    mCLBuffersIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLBufferAccess(mCLBuffers[device], &mCLBuffersAccess[device]);
}

unsigned int Image::getBufferSize() const {
    unsigned int bufferSize = mWidth*mHeight;
    if(mDimensions == 3) {
        bufferSize *= mDepth;
    }
    bufferSize *= getSizeOfDataType(mType,mComponents);

    return bufferSize;
}

void Image::updateOpenCLBufferData(OpenCLDevice::pointer device) {

    // If data exist on device and is up to date do nothing
    if (mCLBuffersIsUpToDate.count(device) > 0 && mCLBuffersIsUpToDate[device]
            == true)
        return;

    if (mCLBuffers.count(device) == 0) {
        // Data is not on device, create it
        unsigned int bufferSize = getBufferSize();
        cl::Buffer * newBuffer = new cl::Buffer(device->getContext(),
        CL_MEM_READ_WRITE, bufferSize);

        mCLBuffers[device] = newBuffer;
        mCLBuffersIsUpToDate[device] = false;
    }


    // Find which data is up to date
    bool updated = false;
    if (mHostDataIsUpToDate) {
        // Transfer host data to this device
        transferCLBufferFromHost(device);
        updated = true;
    } else {
        boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
        for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
                it++) {
            if (it->second == true) {
                // Transfer from this device(it->first) to device
                transferCLImageToHost(it->first);
                transferCLBufferFromHost(device);
                mHostDataIsUpToDate = true;
                updated = true;
                break;
            }
        }
        for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end();
                it++) {
            if (it->second == true) {
                // Transfer from this device(it->first) to device
                transferCLBufferToHost(it->first);
                transferCLBufferFromHost(device);
                mHostDataIsUpToDate = true;
                updated = true;
                break;
            }
        }
    }

    if (!updated)
        throw Exception(
                "Data was not updated because no data was marked as up to date");

    mCLBuffersIsUpToDate[device] = true;
}

void Image::transferCLBufferFromHost(OpenCLDevice::pointer device) {
    unsigned int bufferSize = getBufferSize();
    device->getCommandQueue().enqueueWriteBuffer(*mCLBuffers[device],
        CL_TRUE, 0, bufferSize, mHostData);
}

void Image::transferCLBufferToHost(OpenCLDevice::pointer device) {
    unsigned int bufferSize = getBufferSize();
    device->getCommandQueue().enqueueReadBuffer(*mCLBuffers[device],
        CL_TRUE, 0, bufferSize, mHostData);
}

void Image::updateHostData() {
    // It is the host data that has been modified, no need to update
    if (mHostDataIsUpToDate)
        return;

    if (!mHostHasData) {
        // Data is not initialized, do that first
        unsigned int size = mWidth*mHeight*mComponents;
        if(mDimensions == 3)
            size *= mDepth;
        mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
        mHostHasData = true;
    }

    if (mCLImages.size() > 0) {
        // Find which data is up to date
        bool updated = false;
        boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
        for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
                it++) {
            if (it->second == true) {
                // transfer from this device to host
                transferCLImageToHost(it->first);
                updated = true;
                break;
            }
        }
        for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end();
                it++) {
            if (it->second == true) {
                // transfer from this device to host
                transferCLBufferToHost(it->first);
                updated = true;
                break;
            }
        }

        if (!updated)
            throw Exception(
                    "Data was not updated because no data was marked as up to date");
    }
}

void Image::setAllDataToOutOfDate() {
    mHostDataIsUpToDate = false;
    boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
            it++) {
        it->second = false;
    }
    for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end();
            it++) {
        it->second = false;
    }
}

OpenCLImageAccess2D Image::getOpenCLImageAccess2D(
        accessType type,
        OpenCLDevice::pointer device) {

    if(mDimensions != 2)
        throw Exception("Trying to get OpenCL Image2D access to an Image that is not 2D");

    // Check for write access

    updateOpenCLImageData(device);
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        setAllDataToOutOfDate();
    }
    mCLImagesAccess[device] = true;
    mCLImagesIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLImageAccess2D((cl::Image2D*)mCLImages[device], &mCLImagesAccess[device]);
}

OpenCLImageAccess3D Image::getOpenCLImageAccess3D(
        accessType type,
        OpenCLDevice::pointer device) {

    if(mDimensions != 3)
        throw Exception("Trying to get OpenCL Image3D access to an Image that is not 3D");

    // Check for write access
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        setAllDataToOutOfDate();
        mCLImagesIsUpToDate[device] = true;
    }
    mCLImagesAccess[device] = true;
    updateOpenCLImageData(device);

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLImageAccess3D((cl::Image3D*)mCLImages[device], &mCLImagesAccess[device]);
}

Image::Image() {
    mHostData = NULL;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
    mHostDataIsBeingAccessed = false;
    mIsDynamicData = false;
}

ImageAccess Image::getImageAccess(accessType type) {
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
    }
    updateHostData();
    if(type == ACCESS_READ_WRITE) {
        // Set modified to true since it wants write access
        setAllDataToOutOfDate();
    }

    mHostDataIsUpToDate = true;
    mHostDataIsBeingAccessed = true;

    return ImageAccess(mHostData, &mHostDataIsBeingAccessed);
}

void Image::create3DImage(
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device) {
    if (isInitialized())
        throw Exception(
                "Can not use createImage on an object that is already initialized.");

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mDimensions = 3;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostHasData = true;
        mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        cl::Image3D* clImage = new cl::Image3D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE,
            getOpenCLImageFormat(type, nrOfComponents),
            width, height, depth
            );
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
}

void Image::create3DImage(
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device,
        const void* data) {
    if (isInitialized())
        throw Exception(
                "Can not use createImage on an object that is already initialized.");

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mDimensions = 3;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostData = allocateDataArray(width*height*depth, type, nrOfComponents);
        memcpy(mHostData, data, getSizeOfDataType(type, nrOfComponents)*width*height*depth);
        mHostHasData = true;
        mHostDataIsUpToDate = true;
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        cl::Image3D* clImage;
        // Special treatment for images with 3 components because an OpenCL image can only have 1, 2 or 4 channels
        if(mComponents == 3) {
            void * tempData = adaptDataToImage((void *)data, width*height*depth, type, nrOfComponents);
            clImage = new cl::Image3D(
                clDevice->getContext(),
                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                getOpenCLImageFormat(type, nrOfComponents),
                width, height, depth,
                0, 0,
                tempData
            );
            deleteArray(tempData, type);
        } else {
            clImage = new cl::Image3D(
                clDevice->getContext(),
                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                getOpenCLImageFormat(type, nrOfComponents),
                width, height, depth,
                0, 0,
                (void *)data
            );
        }
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
}

void Image::create2DImage(
        unsigned int width,
        unsigned int height,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device) {
    if (isInitialized())
        throw Exception(
                "Can not use createImage on an object that is already initialized.");

    mWidth = width;
    mHeight = height;
    mDepth = 1;
    mDimensions = 2;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostHasData = true;
        mHostData = allocateDataArray(mWidth*mHeight,mType,mComponents);
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        cl::Image2D* clImage = new cl::Image2D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE,
            getOpenCLImageFormat(type, nrOfComponents),
            width, height
            );
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
}




void Image::create2DImage(
        unsigned int width,
        unsigned int height,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device,
        const void* data) {
    if (isInitialized())
        throw Exception(
                "Can not use createImage on an object that is already initialized.");

    mWidth = width;
    mHeight = height;
    mDepth = 1;
    mDimensions = 2;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostData = allocateDataArray(width*height, type, nrOfComponents);
        memcpy(mHostData, data, getSizeOfDataType(type, nrOfComponents) * width * height);
        mHostHasData = true;
        mHostDataIsUpToDate = true;
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        cl::Image2D* clImage;
        // Special treatment for images with 3 components because an OpenCL image can only have 1, 2 or 4 channels
        if(mComponents == 3) {
            void * tempData = adaptDataToImage((void *)data, width*height, type, nrOfComponents);
            clImage = new cl::Image2D(
                clDevice->getContext(),
                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                getOpenCLImageFormat(type, nrOfComponents),
                width, height,
                0,
                tempData
            );
            deleteArray(tempData, type);
        } else {
            clImage = new cl::Image2D(
                clDevice->getContext(),
                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                getOpenCLImageFormat(type, nrOfComponents),
                width, height,
                0,
                (void*)data
            );
        }
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
}

bool Image::isInitialized() {
    return mCLImages.size() > 0 || mCLBuffers.size() > 0 || mHostHasData;
}

void Image::free(ExecutionDevice::pointer device) {
    // Delete data on a specific device
    if(device->isHost()) {
        deleteArray(mHostData, mType);
        mHostHasData = false;
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        // Delete any OpenCL images
        delete mCLImages[clDevice];
        mCLImages.erase(clDevice);
        mCLImagesIsUpToDate.erase(clDevice);
        mCLImagesAccess.erase(clDevice);
        // Delete any OpenCL buffers
        delete mCLBuffers[clDevice];
        mCLBuffers.erase(clDevice);
        mCLBuffersIsUpToDate.erase(clDevice);
        mCLBuffersAccess.erase(clDevice);
    }
}

void Image::freeAll() {
    // Delete OpenCL Images
    boost::unordered_map<OpenCLDevice::pointer, cl::Image*>::iterator it;
    for (it = mCLImages.begin(); it != mCLImages.end(); it++) {
        delete it->second;
    }
    mCLImages.clear();
    mCLImagesIsUpToDate.clear();
    mCLImagesAccess.clear();

    // Delete OpenCL buffers
    boost::unordered_map<OpenCLDevice::pointer, cl::Buffer*>::iterator it2;
    for (it2 = mCLBuffers.begin(); it2 != mCLBuffers.end(); it2++) {
        delete it2->second;
    }
    mCLBuffers.clear();
    mCLBuffersIsUpToDate.clear();
    mCLBuffersAccess.clear();

    // Delete host data
    if(mHostHasData) {
        this->free(Host::New());
    }
}

unsigned int Image::getWidth() const {
    return mWidth;
}

unsigned int Image::getHeight() const {
    return mHeight;
}

unsigned int Image::getDepth() const {
    return mDepth;
}

unsigned char Image::getDimensions() const {
    return mDimensions;
}

DataType Image::getDataType() const {
    return mType;
}

unsigned int Image::getNrOfComponents() const {
    return mComponents;
}
