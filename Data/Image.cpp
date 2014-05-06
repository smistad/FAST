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

void Image::transferCLImageFromHost(OpenCLDevice::pointer device) {
    // TODO components support needed here
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

void Image::transferCLImageToHost(OpenCLDevice::pointer device) {
    // TODO components support needed here
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
        mCLImagesAccess[device] = true;
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
    // Check for write access
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        setAllDataToOutOfDate();
        mCLBuffersIsUpToDate[device] = true;
    }
    mCLBuffersAccess[device] = true;
    updateOpenCLBufferData(device);

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

    if (mCLBuffersIsUpToDate.count(device) == 0) {
        // Data is not on device, create it
        unsigned int bufferSize = getBufferSize();
        cl::Buffer * newBuffer = new cl::Buffer(device->getContext(),
        CL_MEM_READ_WRITE, bufferSize);

        mCLBuffers[device] = newBuffer;
        mCLBuffersIsUpToDate[device] = true;
        mCLBuffersAccess[device] = true;
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
        switch(mType) {
        case TYPE_FLOAT:
            mHostData = new float[size];
            break;
        case TYPE_UINT8:
            mHostData = new uchar[size];
            break;
        case TYPE_INT8:
            mHostData = new char[size];
            break;
        case TYPE_UINT16:
            mHostData = new ushort[size];
            break;
        case TYPE_INT16:
            mHostData = new short[size];
            break;
        }
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

ImageAccess2D Image::getImageAccess(accessType type) {
    // TODO: this method is currently just a fixed hack

    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        // Set modified to true since it wants write access
        setAllDataToOutOfDate();
        mHostDataIsUpToDate = true;
        mHostDataIsBeingAccessed = true;
    }
    updateHostData();

    return ImageAccess2D(mHostData, &mHostDataIsBeingAccessed);
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
        unsigned int size = width*height*depth*nrOfComponents;
        switch(type) {
        case TYPE_FLOAT:
            mHostData = new float[size];
            break;
        case TYPE_UINT8:
            mHostData = new uchar[size];
            break;
        case TYPE_INT8:
            mHostData = new char[size];
            break;
        case TYPE_UINT16:
            mHostData = new ushort[size];
            break;
        case TYPE_INT16:
            mHostData = new short[size];
            break;
        }
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
        memcpy(mHostData, data, getSizeOfDataType(type, nrOfComponents)*width*height*depth);
        mHostHasData = true;
        mHostDataIsUpToDate = true;
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        cl::Image3D* clImage = new cl::Image3D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            getOpenCLImageFormat(type, nrOfComponents),
            width, height, depth,
            0, 0,
            (void *)data
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
        switch(type) {
        case TYPE_FLOAT:
            mHostData = new float[width*height*nrOfComponents];
            break;
        case TYPE_UINT8:
            mHostData = new uchar[width*height*nrOfComponents];
            break;
        case TYPE_INT8:
            mHostData = new char[width*height*nrOfComponents];
            break;
        case TYPE_UINT16:
            mHostData = new ushort[width*height*nrOfComponents];
            break;
        case TYPE_INT16:
            mHostData = new short[width*height*nrOfComponents];
            break;
        }
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
        case TYPE_FLOAT:
            return padData<float>((float*)data, size);
            break;
        case TYPE_UINT8:
            return padData<unsigned char>((unsigned char*)data, size);
            break;
        case TYPE_INT8:
            return padData<char>((char*)data, size);
            break;
        case TYPE_UINT16:
            return padData<unsigned short>((unsigned short*)data, size);
            break;
        case TYPE_INT16:
            return padData<short>((short*)data, size);
            break;
        }
    }
    return data;
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
        memcpy(mHostData, data, getSizeOfDataType(type, nrOfComponents) * width * height);
        mHostHasData = true;
        mHostDataIsUpToDate = true;
    } else {
        OpenCLDevice::pointer clDevice = boost::dynamic_pointer_cast<OpenCLDevice>(device);
        cl::Image2D* clImage = new cl::Image2D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            getOpenCLImageFormat(type, nrOfComponents),
            width, height,
            0,
            adaptDataToImage((void *)data, width*height, type, nrOfComponents)
        );
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
}

bool Image::isInitialized() {
    return mCLImages.size() > 0 || mHostHasData;
}

void Image::free(ExecutionDevice::pointer device) {
    // Delete data on a specific device
    if(device->isHost()) {
        switch(mType) {
        case TYPE_FLOAT:
            delete[] (float *)mHostData;
            break;
        case TYPE_UINT8:
            delete[] (uchar *)mHostData;
            break;
        case TYPE_INT8:
            delete[] (char *)mHostData;
            break;
        case TYPE_UINT16:
            delete[] (ushort *)mHostData;
            break;
        case TYPE_INT16:
            delete[] (short *)mHostData;
            break;
        }
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
