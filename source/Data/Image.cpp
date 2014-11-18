#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "Exception.hpp"
#include "Utility.hpp"
#include "SceneGraph.hpp"

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
        updateModifiedTimestamp();
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
        updateModifiedTimestamp();
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
        updateModifiedTimestamp();
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
        updateModifiedTimestamp();
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

    SceneGraph &graph = SceneGraph::getInstance();
    graph.removeDataNode(mPtr); // remove old node if it exists
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mBoundingBox = BoundingBox(Float3(width, height, depth));
    SceneGraphNode::pointer node = graph.addDataNodeToNewRoot(mPtr);
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

    SceneGraph &graph = SceneGraph::getInstance();
    graph.removeDataNode(mPtr); // remove old node if it exists
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mBoundingBox = BoundingBox(Float3(width, height, depth));
    SceneGraphNode::pointer node = graph.addDataNodeToNewRoot(mPtr);
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

    SceneGraph &graph = SceneGraph::getInstance();
    graph.removeDataNode(mPtr); // remove old node if it exists
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mBoundingBox = BoundingBox(Float3(width, height, 0));
    SceneGraphNode::pointer node = graph.addDataNodeToNewRoot(mPtr);
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

    SceneGraph &graph = SceneGraph::getInstance();
    graph.removeDataNode(mPtr); // remove old node if it exists
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = 1;
    mBoundingBox = BoundingBox(Float3(width, height, 0));
    SceneGraphNode::pointer node = graph.addDataNodeToNewRoot(mPtr);
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
    updateSceneGraphTransformation();
}

void fast::Image::setOffset(Float<3> offset) {
    mOffset = offset;
    updateSceneGraphTransformation();
}

void fast::Image::setCenterOfRotation(Float<3> rotation) {
    mCenterOfRotation = rotation;
}

void fast::Image::setTransformMatrix(Float<9> transformMatrix) {
    mTransformMatrix = transformMatrix;
    updateSceneGraphTransformation();
}

void Image::updateSceneGraphTransformation() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");

    // Create linear transformation matrix
    LinearTransformation transform;
    MatrixXf transformation = MatrixXf::Identity(3,4);
    transformation(0,0) = mTransformMatrix[0]*mSpacing[0];
    transformation(0,1) = mTransformMatrix[3]*mSpacing[1];
    transformation(0,2) = mTransformMatrix[6]*mSpacing[2];
    transformation(0,3) = mOffset[0];
    transformation(1,0) = mTransformMatrix[1]*mSpacing[0];
    transformation(1,1) = mTransformMatrix[4]*mSpacing[1];
    transformation(1,2) = mTransformMatrix[7]*mSpacing[2];
    transformation(1,3) = mOffset[1];
    transformation(2,0) = mTransformMatrix[2]*mSpacing[0];
    transformation(2,1) = mTransformMatrix[5]*mSpacing[1];
    transformation(2,2) = mTransformMatrix[8]*mSpacing[2];
    transformation(2,3) = mOffset[2];
    Eigen::Transform<float,3,Eigen::Affine> asd;
    asd.affine() = transformation;
    transform.setTransform(asd);

    SceneGraphNode::pointer node = SceneGraph::getInstance().getDataNode(mPtr);
    node->setTransformation(transform);
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
inline void getMaxAndMinFromOpenCLImageResult(void* voidData, unsigned int size, unsigned int nrOfComponents, float* min, float* max) {
    T* data = (T*)voidData;
    *min = data[0];
    *max = data[1];
    for(unsigned int i = nrOfComponents; i < size*nrOfComponents; i += nrOfComponents) {
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
        cl::Image2D level = cl::Image2D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, type, 2), size, size);
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
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Data/ImageMinMax.cl", buildOptions);
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
    unsigned int nrOfComponents = getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, type, 2).image_channel_order == CL_RGBA ? 4 : 2;
    void* result = allocateDataArray(nrOfElements,type,nrOfComponents);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,oul::createOrigoRegion(),oul::createRegion(4,4,1),0,0,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, nrOfComponents, min, max);
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
        cl::Image3D level = cl::Image3D(device->getContext(), CL_MEM_READ_WRITE, getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE3D, type, 2), size, size, size);
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
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Data/ImageMinMax.cl", buildOptions);
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
    unsigned int nrOfComponents = getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE3D, type, 2).image_channel_order == CL_RGBA ? 4 : 2;
    void* result = allocateDataArray(nrOfElements,type,nrOfComponents);
    queue.enqueueReadImage(levels[levels.size()-1],CL_TRUE,oul::createOrigoRegion(),oul::createRegion(4,4,4),0,0,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, nrOfComponents, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, nrOfComponents, min, max);
        break;
    }

}

inline void getMaxAndMinFromOpenCLBuffer(OpenCLDevice::pointer device, cl::Buffer buffer, unsigned int size, DataType type, float* min, float* max) {
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
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Data/ImageMinMax.cl", buildOptions);
    cl::Program program = device->getProgram(programNr);
    cl::CommandQueue queue = device->getCommandQueue();

    // Nr of work groups must be set so that work-group size does not exceed max work-group size (256 on AMD)
    int length = size;
    cl::Kernel reduce(program, "reduce");

    cl::Buffer current = buffer;
    cl::Buffer clResult;
    int workGroupSize = 256;
    int workGroups = 256;
    int X = ceil((float)length / (workGroups*workGroupSize));

    std::cout << "number of work groups is: " << workGroups << std::endl;
    std::cout << "X is: " << X << std::endl;
    clResult = cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, getSizeOfDataType(type,1)*workGroups*2);
    reduce.setArg(0, current);
    reduce.setArg(1, workGroupSize * getSizeOfDataType(type,1), NULL);
    reduce.setArg(2, workGroupSize * getSizeOfDataType(type,1), NULL);
    reduce.setArg(3, size);
    reduce.setArg(4, X);
    reduce.setArg(5, clResult);

    queue.enqueueNDRangeKernel(
            reduce,
            cl::NullRange,
            cl::NDRange(workGroups*workGroupSize),
            cl::NDRange(workGroupSize)
    );

    length = workGroups;

    void* result = allocateDataArray(length, type, 2);
    unsigned int nrOfElements = length;
    queue.enqueueReadBuffer(clResult,CL_TRUE,0,getSizeOfDataType(type,1)*workGroups*2,result);
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromOpenCLImageResult<float>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromOpenCLImageResult<char>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromOpenCLImageResult<uchar>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromOpenCLImageResult<short>(result, nrOfElements, 2, min, max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromOpenCLImageResult<ushort>(result, nrOfElements, 2, min, max);
        break;
    }
}

void Image::calculateMaxAndMinIntensity() {
    // Calculate max and min if image has changed or it is the first time
    if(!mMaxMinInitialized || mMaxMinTimestamp != getTimestamp()) {

        unsigned int nrOfElements = mWidth*mHeight*mDepth*mComponents;
        if(mHostHasData && mHostDataIsUpToDate) {
            // Host data is up to date, calculate min and max on host
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
                            getMaxAndMinFromOpenCLBuffer(device, *buffer, nrOfElements, mType, &mMinimumIntensity, &mMaximumIntensity);
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
    setOffset(image->getOffset());
    setCenterOfRotation(image->getCenterOfRotation());
    setTransformMatrix(image->getTransformMatrix());
    updateModifiedTimestamp();
}

} // end namespace fast;


