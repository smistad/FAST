#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"

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

// Pad data with 1, 2 or 3 channels to 4 channels with 0
template <class T>
void * padData(T * data, unsigned int size, unsigned int nrOfComponents) {
    T * newData = new T[size*4]();
    for(unsigned int i = 0; i < size; i++) {
    	if(nrOfComponents == 1) {
            newData[i*4] = data[i];
    	} else if(nrOfComponents == 2) {
            newData[i*4] = data[i*2];
            newData[i*4+1] = data[i*2+1];
    	} else {
            newData[i*4] = data[i*3];
            newData[i*4+1] = data[i*3+1];
            newData[i*4+2] = data[i*3+2];
    	}
    }
    return (void *)newData;
}

void * adaptDataToImage(void * data, cl_channel_order order, unsigned int size, DataType type, unsigned int nrOfComponents) {
    // Because no OpenCL images support 3 channels,
    // the data has to be padded to 4 channels if the nr of components is 3
    // Also, not all CL platforms support CL_R and CL_RG images
    if(order == CL_RGBA && nrOfComponents != 4) {
        switch(type) {
            fastSwitchTypeMacro(return padData<FAST_TYPE>((FAST_TYPE*)data, size, nrOfComponents))
        }
    }

    return data;
}

// Remove padding from a data array created by padData
template <class T>
void * removePadding(T * data, unsigned int size, unsigned int nrOfComponents) {
     T * newData = new T[size*nrOfComponents];
    for(unsigned int i = 0; i < size; i++) {
    	if(nrOfComponents == 1) {
            newData[i] = data[i*4];
    	} else if(nrOfComponents == 2) {
            newData[i*2] = data[i*4];
            newData[i*2+1] = data[i*4+1];
    	} else {
            newData[i*3] = data[i*4];
            newData[i*3+1] = data[i*4+1];
            newData[i*3+2] = data[i*4+2];
    	}
    }
    return (void *)newData;
}

void * adaptImageDataToHostData(void * data, cl_channel_order order, unsigned int size, DataType type, unsigned int nrOfComponents) {
    // Because no OpenCL images support 3 channels,
    // the data has to be padded to 4 channels if the nr of components is 3.
    // Also, not all CL platforms support CL_R and CL_RG images
    // This function removes that padding
    if(order == CL_RGBA && nrOfComponents != 4) {
        switch(type) {
            fastSwitchTypeMacro(return removePadding<FAST_TYPE>((FAST_TYPE*)data, size, nrOfComponents))
        }
    }

    return data;
}



void Image::transferCLImageFromHost(OpenCLDevice::pointer device) {

    // Special treatment for images with 3 components because an OpenCL image can only have 1, 2 or 4 channels
	// And if the device does not support 1 or 2 channels
    cl::ImageFormat format = getOpenCLImageFormat(device, mDimensions == 2 ? CL_MEM_OBJECT_IMAGE2D : CL_MEM_OBJECT_IMAGE3D, mType, mComponents);
    if(format.image_channel_order == CL_RGBA && mComponents != 4) {
        void * tempData = adaptDataToImage(mHostData, CL_RGBA, mWidth*mHeight*mDepth, mType, mComponents);
        device->getCommandQueue().enqueueWriteImage(*(cl::Image*)mCLImages[device],
        CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                0, tempData);
        deleteArray(tempData, mType);
    } else {
        device->getCommandQueue().enqueueWriteImage(*(cl::Image*)mCLImages[device],
        CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                0, mHostData);
    }


}

void Image::transferCLImageToHost(OpenCLDevice::pointer device) {
    // Special treatment for images with 3 components because an OpenCL image can only have 1, 2 or 4 channels
	// And if the device does not support 1 or 2 channels
    cl::ImageFormat format = getOpenCLImageFormat(device, mDimensions == 2 ? CL_MEM_OBJECT_IMAGE2D : CL_MEM_OBJECT_IMAGE3D, mType, mComponents);
    if(format.image_channel_order == CL_RGBA && mComponents != 4) {
        void * tempData = allocateDataArray(mWidth*mHeight*mDepth,mType,4);
        device->getCommandQueue().enqueueReadImage(*(cl::Image*)mCLImages[device],
        CL_TRUE, oul::createOrigoRegion(), oul::createRegion(mWidth, mHeight, mDepth), 0,
                0, tempData);
        mHostData = adaptImageDataToHostData(tempData,CL_RGBA, mWidth*mHeight*mDepth,mType,mComponents);
        deleteArray(tempData, mType);
    } else {
        if(!mHostHasData) {
            // Must allocate memory for host data
            mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
        }
        device->getCommandQueue().enqueueReadImage(*(cl::Image*)mCLImages[device],
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
            CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, mType,mComponents), mWidth, mHeight);
        } else {
            newImage = new cl::Image3D(device->getContext(),
            CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE3D, mType,mComponents), mWidth, mHeight, mDepth);
        }

        mCLImages[device] = newImage;
        mCLImagesIsUpToDate[device] = false;
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

OpenCLBufferAccess::pointer Image::getOpenCLBufferAccess(
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
        updateModifiedTimestamp();
    }
    mCLBuffersAccess[device] = true;
    mCLBuffersIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date
	OpenCLBufferAccess::pointer accessObject(new OpenCLBufferAccess(mCLBuffers[device], &mCLBuffersAccess[device], &mImageIsBeingWrittenTo));
	return std::move(accessObject);
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

OpenCLImageAccess2D::pointer Image::getOpenCLImageAccess2D(
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
        updateModifiedTimestamp();
    }
    mCLImagesAccess[device] = true;
    mCLImagesIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date
	OpenCLImageAccess2D::pointer accessObject(new OpenCLImageAccess2D((cl::Image2D*)mCLImages[device], &mCLImagesAccess[device], &mImageIsBeingWrittenTo));
	return accessObject;
}

OpenCLImageAccess3D::pointer Image::getOpenCLImageAccess3D(
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
        updateModifiedTimestamp();
    }
    mCLImagesAccess[device] = true;
    mCLImagesIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date
	OpenCLImageAccess3D::pointer accessObject(new OpenCLImageAccess3D((cl::Image3D*)mCLImages[device], &mCLImagesAccess[device], &mImageIsBeingWrittenTo));
	return accessObject;
}

Image::Image() {
    mHostData = NULL;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
    mHostDataIsBeingAccessed = false;
    mIsDynamicData = false;
    mImageIsBeingWrittenTo = false;
    mSpacing = Vector3f(1,1,1);
    mMaxMinInitialized = false;
}

ImageAccess::pointer Image::getImageAccess(accessType type) {
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
        updateModifiedTimestamp();
    }

    mHostDataIsUpToDate = true;
    mHostDataIsBeingAccessed = true;

	Image::pointer image = mPtr.lock();
	ImageAccess::pointer accessObject(new ImageAccess(mHostData, image, &mHostDataIsBeingAccessed, &mImageIsBeingWrittenTo));
	return std::move(accessObject);
}

void Image::create3DImage(
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device) {

    getSceneGraphNode()->reset(); // reset scene graph node
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mBoundingBox = BoundingBox(Vector3f(width, height, depth));
    mDimensions = 3;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostHasData = true;
        mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
    } else {
        OpenCLDevice::pointer clDevice = device;
        cl::Image3D* clImage = new cl::Image3D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE,
            getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE3D, type, nrOfComponents),
            width, height, depth
            );
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
    updateModifiedTimestamp();
}

void Image::create3DImage(
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device,
        const void* data) {

    getSceneGraphNode()->reset(); // reset scene graph node
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mBoundingBox = BoundingBox(Vector3f(width, height, depth));
    mDimensions = 3;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostData = allocateDataArray(width*height*depth, type, nrOfComponents);
        memcpy(mHostData, data, getSizeOfDataType(type, nrOfComponents)*width*height*depth);
        mHostHasData = true;
        mHostDataIsUpToDate = true;
    } else {
        OpenCLDevice::pointer clDevice = device;
        cl::Image3D* clImage;
        void * tempData = adaptDataToImage((void *)data, getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE3D, type, nrOfComponents).image_channel_order, width*height*depth, type, nrOfComponents);
        clImage = new cl::Image3D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE3D, type, nrOfComponents),
            width, height, depth,
            0, 0,
            tempData
        );
        if(data != tempData) {
            deleteArray(tempData, type);
        }
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
    updateModifiedTimestamp();
}

void Image::create2DImage(
        unsigned int width,
        unsigned int height,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device) {

    getSceneGraphNode()->reset(); // reset scene graph node
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mBoundingBox = BoundingBox(Vector3f(width, height, 0));
    mDepth = 1;
    mDimensions = 2;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostHasData = true;
        mHostData = allocateDataArray(mWidth*mHeight,mType,mComponents);
    } else {
        OpenCLDevice::pointer clDevice = device;
        cl::Image2D* clImage = new cl::Image2D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE,
            getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE2D, type, nrOfComponents),
            width, height
            );
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
    updateModifiedTimestamp();
}




void Image::create2DImage(
        unsigned int width,
        unsigned int height,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device,
        const void* data) {

    getSceneGraphNode()->reset(); // reset scene graph node
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = 1;
    mBoundingBox = BoundingBox(Vector3f(width, height, 0));
    mDimensions = 2;
    mType = type;
    mComponents = nrOfComponents;
    if(device->isHost()) {
        mHostData = allocateDataArray(width*height, type, nrOfComponents);
        memcpy(mHostData, data, getSizeOfDataType(type, nrOfComponents) * width * height);
        mHostHasData = true;
        mHostDataIsUpToDate = true;
    } else {
        OpenCLDevice::pointer clDevice = device;
        cl::Image2D* clImage;
        void * tempData = adaptDataToImage((void *)data, getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE2D, type, nrOfComponents).image_channel_order, width*height, type, nrOfComponents);
        clImage = new cl::Image2D(
            clDevice->getContext(),
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE2D, type, nrOfComponents),
            width, height,
            0,
            tempData
        );
        if(data != tempData) {
            deleteArray(tempData, type);
        }
        mCLImages[clDevice] = clImage;
        mCLImagesIsUpToDate[clDevice] = true;
        mCLImagesAccess[clDevice] = false;
    }
    updateModifiedTimestamp();
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
        OpenCLDevice::pointer clDevice = device;
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
        this->free(Host::getInstance());
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

Vector3f fast::Image::getSpacing() const {
    return mSpacing;
}

void fast::Image::setSpacing(Vector3f spacing) {
    mSpacing = spacing;
}

/*
void Image::updateSceneGraphTransformation() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");

    // Create linear transformation matrix
    AffineTransformation transformation;
    transformation.linear() = mTransformMatrix;
    transformation.translation() = mOffset;
    transformation.scale(mSpacing);

    SceneGraphNode::pointer node = getSceneGraphNode();
    node->setTransformation(transformation);
}
*/


void Image::calculateMaxAndMinIntensity() {
    // Calculate max and min if image has changed or it is the first time
    if(!mMaxMinInitialized || mMaxMinTimestamp != getTimestamp()) {

        unsigned int nrOfElements = mWidth*mHeight*mDepth*mComponents;
        if(mHostHasData && mHostDataIsUpToDate) {
            // Host data is up to date, calculate min and max on host
            ImageAccess::pointer access = getImageAccess(ACCESS_READ);
            void* data = access->get();
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
                        OpenCLImageAccess2D::pointer access = getOpenCLImageAccess2D(ACCESS_READ, device);
                        cl::Image2D* clImage = access->get();
                        getMaxAndMinFromOpenCLImage(device, *clImage, mType, &mMinimumIntensity, &mMaximumIntensity);
                    } else {
                        if(device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") == std::string::npos) {
                            // Writing to 3D images is not supported on this device
                            // Copy data to buffer instead and do the max min calculation on the buffer instead
                            OpenCLBufferAccess::pointer access = getOpenCLBufferAccess(ACCESS_READ, device);
                            cl::Buffer* buffer = access->get();
                            getMaxAndMinFromOpenCLBuffer(device, *buffer, nrOfElements, mType, &mMinimumIntensity, &mMaximumIntensity);
                        } else {
                            OpenCLImageAccess3D::pointer access = getOpenCLImageAccess3D(ACCESS_READ, device);
                            cl::Image3D* clImage = access->get();
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
                        OpenCLBufferAccess::pointer access = getOpenCLBufferAccess(ACCESS_READ, device);
                        cl::Buffer* buffer = access->get();
                        getMaxAndMinFromOpenCLBuffer(device, *buffer, nrOfElements, mType, &mMinimumIntensity, &mMaximumIntensity);
                        found = true;
                    }
                }
            }
        }

        // Update timestamp
        mMaxMinTimestamp = getTimestamp();
        mMaxMinInitialized = true;
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

void Image::createFromImage(
        Image::pointer image,
        ExecutionDevice::pointer device) {
    // Create image first
    if(image->getDimensions() == 2) {
        create2DImage(
                image->getWidth(),
                image->getHeight(),
                image->getDataType(),
                image->getNrOfComponents(),
                device
        );
    } else {
        create3DImage(
                image->getWidth(),
                image->getHeight(),
                image->getDepth(),
                image->getDataType(),
                image->getNrOfComponents(),
                device
        );
    }

    // Copy metadata
    setSpacing(image->getSpacing());
    updateModifiedTimestamp();
}


Image::pointer Image::copy(ExecutionDevice::pointer device) {
    Image::pointer clone = Image::New();
    clone->createFromImage(mPtr.lock(), device);

    // If device is host, get data from this image to host
    if(device->isHost()) {
        ImageAccess::pointer readAccess = this->getImageAccess(ACCESS_READ);
        ImageAccess::pointer writeAccess = clone->getImageAccess(ACCESS_READ_WRITE);

        void* input = readAccess->get();
        void* output = writeAccess->get();
        switch(getDataType()) {
            fastSwitchTypeMacro(memcpy(output, input, sizeof(FAST_TYPE)*getWidth()*getHeight()*getDepth()*getNrOfComponents()));
        }
    } else {
        // If device is not host
        OpenCLDevice::pointer clDevice = device;
        if(getDimensions() == 2) {
            OpenCLImageAccess2D::pointer readAccess = this->getOpenCLImageAccess2D(ACCESS_READ, clDevice);
            OpenCLImageAccess2D::pointer writeAccess = clone->getOpenCLImageAccess2D(ACCESS_READ_WRITE, clDevice);
            cl::Image2D* input = readAccess->get();
            cl::Image2D* output = writeAccess->get();

            clDevice->getCommandQueue().enqueueCopyImage(
                    *input,
                    *output,
                    oul::createOrigoRegion(),
                    oul::createOrigoRegion(),
                    oul::createRegion(getWidth(), getHeight(), 1)
            );
        } else {
            OpenCLImageAccess3D::pointer readAccess = this->getOpenCLImageAccess3D(ACCESS_READ, clDevice);
            OpenCLImageAccess3D::pointer writeAccess = clone->getOpenCLImageAccess3D(ACCESS_READ_WRITE, clDevice);
            cl::Image3D* input = readAccess->get();
            cl::Image3D* output = writeAccess->get();

            clDevice->getCommandQueue().enqueueCopyImage(
                    *input,
                    *output,
                    oul::createOrigoRegion(),
                    oul::createOrigoRegion(),
                    oul::createRegion(getWidth(), getHeight(), getDepth())
            );
        }
    }

    return clone;
}

BoundingBox Image::getTransformedBoundingBox() const {
    AffineTransformation T = SceneGraph::getAffineTransformationFromData(DataObject::pointer(mPtr.lock()));

    // Add image spacing
    T.scale(getSpacing());

    return getBoundingBox().getTransformedBoundingBox(T);
}

} // end namespace fast;


