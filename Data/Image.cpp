#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "Exception.hpp"

namespace fast {

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

    if(!isInitialized())
        throw Exception("Image has not been initialized.");

    if(mImageIsBeingWrittenTo)
        throw Exception("Requesting access to an image that is already being written to.");
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mImageIsBeingWrittenTo = true;
    }
    updateOpenCLBufferData(device);
    if (type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
    }
    mCLBuffersAccess[device] = true;
    mCLBuffersIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLBufferAccess(mCLBuffers[device], &mCLBuffersAccess[device], &mImageIsBeingWrittenTo);
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

    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    if(mImageIsBeingWrittenTo)
        throw Exception("Requesting access to an image that is already being written to.");
    if(mDimensions != 2)
        throw Exception("Trying to get OpenCL Image2D access to an Image that is not 2D");

    // Check for write access
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mImageIsBeingWrittenTo = true;
    }
    updateOpenCLImageData(device);
    if (type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
    }
    mCLImagesAccess[device] = true;
    mCLImagesIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLImageAccess2D((cl::Image2D*)mCLImages[device], &mCLImagesAccess[device], &mImageIsBeingWrittenTo);
}

OpenCLImageAccess3D Image::getOpenCLImageAccess3D(
        accessType type,
        OpenCLDevice::pointer device) {

    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    if(mImageIsBeingWrittenTo)
        throw Exception("Requesting access to an image that is already being written to.");
    if(mDimensions != 3)
        throw Exception("Trying to get OpenCL Image3D access to an Image that is not 3D");

    // Check for write access
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mImageIsBeingWrittenTo = true;
    }
    updateOpenCLImageData(device);
    if (type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
    }
    mCLImagesAccess[device] = true;
    mCLImagesIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date

    return OpenCLImageAccess3D((cl::Image3D*)mCLImages[device], &mCLImagesAccess[device], &mImageIsBeingWrittenTo);
}

Image::Image() {
    mHostData = NULL;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
    mHostDataIsBeingAccessed = false;
    mIsDynamicData = false;
    mImageIsBeingWrittenTo = false;
    mSpacing[0] = 1;
    mSpacing[1] = 1;
    mSpacing[2] = 1;
    // Identity matrix
    mTransformMatrix[0] = 1;
    mTransformMatrix[4] = 1;
    mTransformMatrix[8] = 1;

    mMaxMinInitialized = false;
}

ImageAccess Image::getImageAccess(accessType type) {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    if(mImageIsBeingWrittenTo)
        throw Exception("Requesting access to an image that is already being written to.");

    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mImageIsBeingWrittenTo = true;
    }
    updateHostData();
    if(type == ACCESS_READ_WRITE) {
        // Set modified to true since it wants write access
        setAllDataToOutOfDate();
    }

    mHostDataIsUpToDate = true;
    mHostDataIsBeingAccessed = true;

    return ImageAccess(mHostData, &mHostDataIsBeingAccessed, &mImageIsBeingWrittenTo);
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

bool Image::isInitialized() const {
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
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return mWidth;
}

unsigned int Image::getHeight() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return mHeight;
}

unsigned int Image::getDepth() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return mDepth;
}

unsigned char Image::getDimensions() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return mDimensions;
}

DataType Image::getDataType() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return mType;
}

unsigned int Image::getNrOfComponents() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return mComponents;
}

Float<3> fast::Image::getSpacing() const {
    return mSpacing;
}

Float<3> fast::Image::getOffset() const {
    return mOffset;
}

Float<3> fast::Image::getCenterOfRotation() const {
    return mCenterOfRotation;
}

Float<9> fast::Image::getTransformMatrix() const {
    return mTransformMatrix;
}

void fast::Image::setSpacing(Float<3> spacing) {
    mSpacing = spacing;
}

void fast::Image::setOffset(Float<3> offset) {
    mOffset = offset;
}

void fast::Image::setCenterOfRotation(Float<3> rotation) {
    mCenterOfRotation = rotation;
}

void fast::Image::setTransformMatrix(Float<9> transformMatrix) {
    mTransformMatrix = transformMatrix;
}

template <class T>
inline void getMaxAndMinFromData(void* voidData, unsigned int nrOfElements, float* min, float* max) {
    T* data = (T*)voidData;

    *min = std::numeric_limits<float>::max();
    *max = std::numeric_limits<float>::min();
    for(unsigned int i = 0; i < nrOfElements; i++) {
        if((float)data[i] < *min) {
            *min = (float)data[i];
        }
        if((float)data[i] > *max) {
            *max = (float)data[i];
        }
    }
}

unsigned int getPowerOfTwoSize(unsigned int size) {
    int i = 1;
    while(pow(2, i) < size)
        i++;

    return (unsigned int)pow(2,i);
}

template <class T>
inline void getMaxAndMinFromOpenCLImageResult(void* voidData, unsigned int size, float* min, float* max) {
    T* data = (T*)voidData;
    *min = data[0];
    *max = data[1];
    for(unsigned int i = 2; i < size*2; i += 2) {
        //std::cout << "min: " << data[i] << std::endl;
        //std::cout << "max: " << data[i+1] << std::endl;
        if(data[i] < *min) {
            *min = data[i];
        }
        if(data[i+1] > *max) {
            *max = data[i+1];
        }
    }
}

inline void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image2D image, DataType type, float* min, float* max) {
    // Get power of two size
    unsigned int powerOfTwoSize = getPowerOfTwoSize(std::max(image.getImageInfo<CL_IMAGE_WIDTH>(), image.getImageInfo<CL_IMAGE_HEIGHT>()));

    // Create image levels
    unsigned int size = powerOfTwoSize;
    size /= 2;
    std::vector<cl::Image2D> levels;
    while(size >= 4) {
        cl::Image2D level = cl::Image2D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(type, 2), size, size);
        levels.push_back(level);
        size /= 2;
    }

    // Compile OpenCL code
    std::string buildOptions = "";
    switch(type) {
    case TYPE_FLOAT:
        buildOptions = "-DTYPE_FLOAT";
        break;
    case TYPE_UINT8:
        buildOptions = "-DTYPE_UINT8";
        break;
    case TYPE_INT8:
        buildOptions = "-DTYPE_INT8";
        break;
    case TYPE_UINT16:
        buildOptions = "-DTYPE_UINT16";
        break;
    case TYPE_INT16:
        buildOptions = "-DTYPE_INT16";
        break;
    }
    int programNr = device->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Data/ImageMinMax.cl", buildOptions);
    cl::Program program = device->getProgram(programNr);
    cl::CommandQueue queue = device->getCommandQueue();

    // Fill first level
    size = powerOfTwoSize/2;
    cl::Kernel firstLevel(program, "createFirstMinMaxImage2DLevel");
    firstLevel.setArg(0, image);
    firstLevel.setArg(1, levels[0]);

    queue.enqueueNDRangeKernel(
            firstLevel,
            cl::NullRange,
            cl::NDRange(size,size),
            cl::NullRange
    );

    // Fill all other levels
    cl::Kernel createLevel(program, "createMinMaxImage2DLevel");
    int i = 0;
    size /= 2;
    while(size >= 4) {
        createLevel.setArg(0, levels[i]);
        createLevel.setArg(1, levels[i+1]);
        queue.enqueueNDRangeKernel(
                createLevel,
                cl::NullRange,
                cl::NDRange(size,size),
                cl::NullRange
        );
        i++;
        size /= 2;
    }

    // Get result from the last level
    unsigned int nrOfElements = 4*4;
    void* result = allocateDataArray(nrOfElements,type,2);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,oul::createOrigoRegion(),oul::createRegion(4,4,1),0,0,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, min, max);
        break;
    }

}

inline void getMaxAndMinFromOpenCLImage(OpenCLDevice::pointer device, cl::Image3D image, DataType type, float* min, float* max) {



   // Get power of two size
    unsigned int powerOfTwoSize = getPowerOfTwoSize(std::max(image.getImageInfo<CL_IMAGE_DEPTH>(), std::max(
            image.getImageInfo<CL_IMAGE_WIDTH>(),
            image.getImageInfo<CL_IMAGE_HEIGHT>())));

    // Create image levels
    unsigned int size = powerOfTwoSize;
    size /= 2;
    std::vector<cl::Image3D> levels;
    while(size >= 4) {
        cl::Image3D level = cl::Image3D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(type, 2), size, size, size);
        levels.push_back(level);
        size /= 2;
    }

    // Compile OpenCL code
    std::string buildOptions = "";
    switch(type) {
    case TYPE_FLOAT:
        buildOptions = "-DTYPE_FLOAT";
        break;
    case TYPE_UINT8:
        buildOptions = "-DTYPE_UINT8";
        break;
    case TYPE_INT8:
        buildOptions = "-DTYPE_INT8";
        break;
    case TYPE_UINT16:
        buildOptions = "-DTYPE_UINT16";
        break;
    case TYPE_INT16:
        buildOptions = "-DTYPE_INT16";
        break;
    }
    int programNr = device->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Data/ImageMinMax.cl", buildOptions);
    cl::Program program = device->getProgram(programNr);
    cl::CommandQueue queue = device->getCommandQueue();

    // Fill first level
    size = powerOfTwoSize/2;
    cl::Kernel firstLevel(program, "createFirstMinMaxImage3DLevel");
    firstLevel.setArg(0, image);
    firstLevel.setArg(1, levels[0]);

    queue.enqueueNDRangeKernel(
            firstLevel,
            cl::NullRange,
            cl::NDRange(size,size,size),
            cl::NullRange
    );

    // Fill all other levels
    cl::Kernel createLevel(program, "createMinMaxImage3DLevel");
    int i = 0;
    size /= 2;
    while(size >= 4) {
        createLevel.setArg(0, levels[i]);
        createLevel.setArg(1, levels[i+1]);
        queue.enqueueNDRangeKernel(
                createLevel,
                cl::NullRange,
                cl::NDRange(size,size,size),
                cl::NullRange
        );
        i++;
        size /= 2;
    }

    // Get result from the last level
    unsigned int nrOfElements = 4*4*4;
    void* result = allocateDataArray(nrOfElements,type,2);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,oul::createOrigoRegion(),oul::createRegion(4,4,4),0,0,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, min, max);
        break;
    }

}

inline void getMaxAndMinFromOpenCLBuffer(OpenCLDevice::pointer device, cl::Buffer buffer, float* min, float* max) {

}

void Image::calculateMaxAndMinIntensity() {
    // Calculate max and min if image has changed or it is the first time
    if(!mMaxMinInitialized || mMaxMinTimestamp != getTimestamp()) {

        if(mHostHasData && mHostDataIsUpToDate) {
            // Host data is up to date, calculate min and max on host
            unsigned int nrOfElements = mWidth*mHeight*mDepth*mComponents;
            ImageAccess access = getImageAccess(ACCESS_READ);
            void* data = access.get();
            switch(mType) {
            case TYPE_FLOAT:
                getMaxAndMinFromData<float>(data,nrOfElements,&mMinimumIntensity,&mMaximumIntensity);
                break;
            case TYPE_INT8:
                getMaxAndMinFromData<char>(data,nrOfElements,&mMinimumIntensity,&mMaximumIntensity);
                break;
            case TYPE_UINT8:
                getMaxAndMinFromData<uchar>(data,nrOfElements,&mMinimumIntensity,&mMaximumIntensity);
                break;
            case TYPE_INT16:
                getMaxAndMinFromData<short>(data,nrOfElements,&mMinimumIntensity,&mMaximumIntensity);
                break;
            case TYPE_UINT16:
                getMaxAndMinFromData<ushort>(data,nrOfElements,&mMinimumIntensity,&mMaximumIntensity);
                break;
            }
        } else {
            // TODO the logic here can be improved. For instance choose the best device
            // Find some OpenCL image data or buffer data that is up to date
            bool found = false;
            boost::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
            for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
                if(it->second == true) {
                    OpenCLDevice::pointer device = it->first;
                    if(mDimensions == 2) {
                        OpenCLImageAccess2D access = getOpenCLImageAccess2D(ACCESS_READ, device);
                        cl::Image2D* clImage = access.get();
                        getMaxAndMinFromOpenCLImage(device, *clImage, mType, &mMinimumIntensity, &mMaximumIntensity);
                    } else {
                        if(device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") == std::string::npos) {
                            // Writing to 3D images is not supported on this device
                            // Copy data to buffer instead and do the max min calculation on the buffer instead
                            OpenCLBufferAccess access = getOpenCLBufferAccess(ACCESS_READ, device);
                            cl::Buffer* buffer = access.get();
                            getMaxAndMinFromOpenCLBuffer(device, *buffer, &mMinimumIntensity, &mMaximumIntensity);
                        } else {
                            OpenCLImageAccess3D access = getOpenCLImageAccess3D(ACCESS_READ, device);
                            cl::Image3D* clImage = access.get();
                            getMaxAndMinFromOpenCLImage(device, *clImage, mType, &mMinimumIntensity, &mMaximumIntensity);
                        }
                    }
                    found = true;
                }
            }

            if(!found) {
                for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end(); it++) {
                    if(it->second == true) {
                        OpenCLDevice::pointer device = it->first;
                        OpenCLBufferAccess access = getOpenCLBufferAccess(ACCESS_READ, device);
                        cl::Buffer* buffer = access.get();
                        getMaxAndMinFromOpenCLBuffer(device, *buffer, &mMinimumIntensity, &mMaximumIntensity);
                        found = true;
                    }
                }
            }
        }

        // Update timestamp
        mMaxMinTimestamp = getTimestamp();
    }
}

float Image::calculateMaximumIntensity() {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    calculateMaxAndMinIntensity();

    return mMaximumIntensity;
}

float Image::calculateMinimumIntensity() {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    calculateMaxAndMinIntensity();

    return mMinimumIntensity;
}

} // end namespace fast;
