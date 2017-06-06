#include "Image.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Config.hpp"

namespace fast {

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
        CL_TRUE, createOrigoRegion(), createRegion(mWidth, mHeight, mDepth), 0,
                0, tempData);
        deleteArray(tempData, mType);
    } else {
        device->getCommandQueue().enqueueWriteImage(*(cl::Image*)mCLImages[device],
        CL_TRUE, createOrigoRegion(), createRegion(mWidth, mHeight, mDepth), 0,
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
        CL_TRUE, createOrigoRegion(), createRegion(mWidth, mHeight, mDepth), 0,
                0, tempData);
        mHostData = adaptImageDataToHostData(tempData,CL_RGBA, mWidth*mHeight*mDepth,mType,mComponents);
        deleteArray(tempData, mType);
    } else {
        if(!mHostHasData) {
            // Must allocate memory for host data
            mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
			mHostHasData = true;
        }
        device->getCommandQueue().enqueueReadImage(*(cl::Image*)mCLImages[device],
        CL_TRUE, createOrigoRegion(), createRegion(mWidth, mHeight, mDepth), 0,
                0, mHostData);
    }
}

bool Image::hasAnyData() {
    return mHostHasData || mCLImages.size() > 0 || mCLBuffers.size() > 0;
}

void Image::updateOpenCLImageData(OpenCLDevice::pointer device) {

    // If data exist on device and is up to date do nothing
    if (mCLImagesIsUpToDate.count(device) > 0 && mCLImagesIsUpToDate[device]
            == true)
        return;

    bool updated = false;
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

        if(hasAnyData()) {
            mCLImagesIsUpToDate[device] = false;
        } else {
            mCLImagesIsUpToDate[device] = true;
            updated = true;
        }
        mCLImages[device] = newImage;
    }

    // Find which data is up to date
	if (!mCLImagesIsUpToDate[device]) {
		if (mHostDataIsUpToDate) {
			// Transfer host data to this device
			transferCLImageFromHost(device);
			updated = true;
		} else {
			std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
			for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
				it++) {
				if (it->second == true) {
					// Transfer from this device(it->first) to device
					// TODO should use copy image to image here, if possible
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
					// TODO should use copy buffer to image here, if possible
					transferCLBufferToHost(it->first);
					transferCLImageFromHost(device);
					mHostDataIsUpToDate = true;
					updated = true;
					break;
				}
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

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    updateOpenCLBufferData(device);
    if(type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    mCLBuffersIsUpToDate[device] = true;
    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

    // Now it is guaranteed that the data is on the device and that it is up to date
	OpenCLBufferAccess::pointer accessObject(new OpenCLBufferAccess(mCLBuffers[device],  mPtr.lock()));
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

    bool updated = false;
    if (mCLBuffers.count(device) == 0) {
        // Data is not on device, create it
        unsigned int bufferSize = getBufferSize();
        cl::Buffer * newBuffer = new cl::Buffer(device->getContext(),
        CL_MEM_READ_WRITE, bufferSize);

        if(hasAnyData()) {
            mCLBuffersIsUpToDate[device] = false;
        } else {
            mCLBuffersIsUpToDate[device] = true;
            updated = true;
        }
        mCLBuffers[device] = newBuffer;
    }


    // Find which data is up to date
    if(!mCLBuffersIsUpToDate[device]) {
        if (mHostDataIsUpToDate) {
            // Transfer host data to this device
            transferCLBufferFromHost(device);
            updated = true;
        } else {
            std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
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
	if (!mHostHasData) {
		// Must allocate memory for host data
		mHostData = allocateDataArray(mWidth*mHeight*mDepth, mType, mComponents);
		mHostHasData = true;
	}
    unsigned int bufferSize = getBufferSize();
    device->getCommandQueue().enqueueReadBuffer(*mCLBuffers[device],
        CL_TRUE, 0, bufferSize, mHostData);
}

void Image::updateHostData() {
    // It is the host data that has been modified, no need to update
    if (mHostDataIsUpToDate)
        return;

    bool updated = false;
    if (!mHostHasData) {
        // Data is not initialized, do that first
        unsigned int size = mWidth*mHeight*mComponents;
        if(mDimensions == 3)
            size *= mDepth;
        mHostData = allocateDataArray(mWidth*mHeight*mDepth,mType,mComponents);
        if(hasAnyData()) {
            mHostDataIsUpToDate = false;
        } else {
            mHostDataIsUpToDate = true;
            updated = true;
        }
        mHostHasData = true;
    }

    // Find which data is up to date
    std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
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

void Image::setAllDataToOutOfDate() {
    mHostDataIsUpToDate = false;
    std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end();
            it++) {
        it->second = false;
    }
    for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end();
            it++) {
        it->second = false;
    }
}

OpenCLImageAccess::pointer Image::getOpenCLImageAccess(
        accessType type,
        OpenCLDevice::pointer device) {

    if(!isInitialized())
        throw Exception("Image has not been initialized.");

    blockIfBeingWrittenTo();

    // Check for write access
    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
    	std::lock_guard<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    updateOpenCLImageData(device);
    if (type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    {
        std::lock_guard<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }
    mCLImagesIsUpToDate[device] = true;

    // Now it is guaranteed that the data is on the device and that it is up to date
    if(mDimensions == 2) {
        OpenCLImageAccess::pointer accessObject(new OpenCLImageAccess((cl::Image2D*)mCLImages[device], mPtr.lock()));
        return accessObject;
    } else {
        OpenCLImageAccess::pointer accessObject(new OpenCLImageAccess((cl::Image3D*)mCLImages[device], mPtr.lock()));
        return accessObject;
    }
}

Image::Image() {
    mHostData = NULL;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
    mIsDynamicData = false;
    mSpacing = Vector3f(1,1,1);
    mMaxMinInitialized = false;
    mAverageInitialized = false;
    mIsInitialized = false;
}

ImageAccess::pointer Image::getImageAccess(accessType type) {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    updateHostData();
    if(type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    mHostDataIsUpToDate = true;
    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

	ImageAccess::pointer accessObject(new ImageAccess(mHostData, mPtr.lock()));
	return std::move(accessObject);
}

void Image::create(
        VectorXui size,
        DataType type,
        unsigned int nrOfComponents) {

    if(size.rows() > 2 && size.z() > 1) {
        // 3D
        create(size.x(), size.y(), size.z(), type, nrOfComponents);
    } else {
        // 2D
        create(size.x(), size.y(), type, nrOfComponents);
    }
}

void Image::create(
        VectorXui size,
        DataType type,
        unsigned int nrOfComponents,
        ExecutionDevice::pointer device,
        const void* data) {

    if(size.rows() > 2 && size.z() > 1) {
        // 3D
        create(size.x(), size.y(), size.z(), type, nrOfComponents, device, data);
    } else {
        // 2D
        create(size.x(), size.y(), type, nrOfComponents, device, data);
    }
}

void Image::create(
        VectorXui size,
        DataType type,
        unsigned int nrOfComponents,
        const void* data) {

    if(size.rows() > 2 && size.z() > 1) {
        // 3D
        create(size.x(), size.y(), size.z(), type, nrOfComponents, DeviceManager::getInstance()->getDefaultComputationDevice(), data);
    } else {
        // 2D
        create(size.x(), size.y(), type, nrOfComponents, DeviceManager::getInstance()->getDefaultComputationDevice(), data);
    }
}


void Image::create(
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        DataType type,
        unsigned int nrOfComponents) {

    getSceneGraphNode()->reset(); // reset scene graph node
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mBoundingBox = BoundingBox(Vector3f(width, height, depth));
    mDimensions = 3;
    mType = type;
    mComponents = nrOfComponents;
    updateModifiedTimestamp();
    mIsInitialized = true;
}

void Image::create(
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
    }
    updateModifiedTimestamp();
    mIsInitialized = true;
}

void Image::create(
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        DataType type,
        unsigned int nrOfComponents,
        const void* data) {

	create(width, height, depth, type, nrOfComponents, DeviceManager::getInstance()->getDefaultComputationDevice(), data);
}

void Image::create(
        unsigned int width,
        unsigned int height,
        DataType type,
        unsigned int nrOfComponents) {

    getSceneGraphNode()->reset(); // reset scene graph node
    freeAll(); // delete any old data

    mWidth = width;
    mHeight = height;
    mBoundingBox = BoundingBox(Vector3f(width, height, 0));
    mDepth = 1;
    mDimensions = 2;
    mType = type;
    mComponents = nrOfComponents;
    updateModifiedTimestamp();
    mIsInitialized = true;
}




void Image::create(
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
    }
    updateModifiedTimestamp();
    mIsInitialized = true;
}

void Image::create(
        unsigned int width,
        unsigned int height,
        DataType type,
        unsigned int nrOfComponents,
        const void* data) {

	create(width, height, type, nrOfComponents, DeviceManager::getInstance()->getDefaultComputationDevice(), data);
}

bool Image::isInitialized() const {
    return mIsInitialized;
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
        // Delete any OpenCL buffers
        delete mCLBuffers[clDevice];
        mCLBuffers.erase(clDevice);
        mCLBuffersIsUpToDate.erase(clDevice);
    }
}

void Image::freeAll() {
    // Delete OpenCL Images
    std::unordered_map<OpenCLDevice::pointer, cl::Image*>::iterator it;
    for (it = mCLImages.begin(); it != mCLImages.end(); it++) {
        delete it->second;
    }
    mCLImages.clear();
    mCLImagesIsUpToDate.clear();

    // Delete OpenCL buffers
    std::unordered_map<OpenCLDevice::pointer, cl::Buffer*>::iterator it2;
    for (it2 = mCLBuffers.begin(); it2 != mCLBuffers.end(); it2++) {
        delete it2->second;
    }
    mCLBuffers.clear();
    mCLBuffersIsUpToDate.clear();

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

Vector3ui Image::getSize() const {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");
    return Vector3ui(mWidth, mHeight, mDepth);
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

void fast::Image::setSpacing(float x, float y, float z) {
	setSpacing(Vector3f(x, y, z));
}

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
            case TYPE_SNORM_INT16:
            case TYPE_UNORM_INT16:
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
            std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
            for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
                if(it->second == true) {
                    OpenCLDevice::pointer device = it->first;
                    if(mDimensions == 2) {
                        OpenCLImageAccess::pointer access = getOpenCLImageAccess(ACCESS_READ, device);
                        cl::Image2D* clImage = access->get2DImage();
                        getMaxAndMinFromOpenCLImage(device, *clImage, mType, &mMinimumIntensity, &mMaximumIntensity);
                    } else {
                        if(device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") == std::string::npos) {
                            // Writing to 3D images is not supported on this device
                            // Copy data to buffer instead and do the max min calculation on the buffer instead
                            OpenCLBufferAccess::pointer access = getOpenCLBufferAccess(ACCESS_READ, device);
                            cl::Buffer* buffer = access->get();
                            getMaxAndMinFromOpenCLBuffer(device, *buffer, nrOfElements, mType, &mMinimumIntensity, &mMaximumIntensity);
                        } else {
                            OpenCLImageAccess::pointer access = getOpenCLImageAccess(ACCESS_READ, device);
                            cl::Image3D* clImage = access->get3DImage();
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

float Image::calculateAverageIntensity() {
     if(!isInitialized())
        throw Exception("Image has not been initialized.");

    // Calculate max and min if image has changed or it is the first time
    if(!mAverageInitialized || mAverageIntensityTimestamp != getTimestamp()) {
        unsigned int nrOfElements = mWidth*mHeight*mDepth;
        if(mHostHasData && mHostDataIsUpToDate) {
            reportInfo() << "calculating sum on host" << Reporter::end();
            // Host data is up to date, calculate min and max on host
            ImageAccess::pointer access = getImageAccess(ACCESS_READ);
            void* data = access->get();
            switch(mType) {
            case TYPE_FLOAT:
                mAverageIntensity = getSumFromData<float>(data,nrOfElements) / nrOfElements;
                break;
            case TYPE_INT8:
                mAverageIntensity = getSumFromData<char>(data,nrOfElements) / nrOfElements;
                break;
            case TYPE_UINT8:
                mAverageIntensity = getSumFromData<uchar>(data,nrOfElements) / nrOfElements;
                break;
            case TYPE_INT16:
                mAverageIntensity = getSumFromData<short>(data,nrOfElements) / nrOfElements;
                break;
            case TYPE_UINT16:
                mAverageIntensity = getSumFromData<ushort>(data,nrOfElements) / nrOfElements;
                break;
            }
        } else {
            reportInfo() << "calculating sum with OpenCL" << Reporter::end();
            // TODO the logic here can be improved. For instance choose the best device
            // Find some OpenCL image data or buffer data that is up to date
            bool found = false;
            std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
            for (it = mCLImagesIsUpToDate.begin(); it != mCLImagesIsUpToDate.end(); it++) {
                if(it->second == true) {
                    OpenCLDevice::pointer device = it->first;
                    float sum;
                    if(mDimensions == 2) {
                        OpenCLImageAccess::pointer access = getOpenCLImageAccess(ACCESS_READ, device);
                        cl::Image2D* clImage = access->get2DImage();
                        getIntensitySumFromOpenCLImage(device, *clImage, mType, &sum);
                    } else {
                        if(!device->isWritingTo3DTexturesSupported()) {
                            // Writing to 3D images is not supported on this device
                            // Copy data to buffer instead and do the max min calculation on the buffer instead
                            OpenCLBufferAccess::pointer access = getOpenCLBufferAccess(ACCESS_READ, device);
                            cl::Buffer* buffer = access->get();
                            // TODO
                            throw Exception("Not implemented yet");
                            //getMaxAndMinFromOpenCLBuffer(device, *buffer, nrOfElements, mType, &mMinimumIntensity, &mMaximumIntensity);
                        } else {
                            OpenCLImageAccess::pointer access = getOpenCLImageAccess(ACCESS_READ, device);
                            cl::Image3D* clImage = access->get3DImage();
                            // TODO
                            throw Exception("Not implemented yet");
                            //getIntensitySumFromOpenCLImage(device, *clImage, mType, &sum);
                        }
                    }
                    mAverageIntensity = sum / nrOfElements;
                    found = true;
                }
            }

            if(!found) {
                for (it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end(); it++) {
                    if(it->second == true) {
                        OpenCLDevice::pointer device = it->first;
                        OpenCLBufferAccess::pointer access = getOpenCLBufferAccess(ACCESS_READ, device);
                        cl::Buffer* buffer = access->get();
                        // TODO
                            throw Exception("Not implemented yet");
                        //getMaxAndMinFromOpenCLBuffer(device, *buffer, nrOfElements, mType, &mMinimumIntensity, &mMaximumIntensity);
                        found = true;
                    }
                }
            }
        }

        // Update timestamp
        mAverageIntensityTimestamp = getTimestamp();
        mAverageInitialized = true;
    }

    return mAverageIntensity;
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
        Image::pointer image) {
    // Create image first
    create(image->getSize(), image->getDataType(), image->getNrOfComponents());

    // Copy metadata
    setSpacing(image->getSpacing());
    updateModifiedTimestamp();
}


Image::pointer Image::copy(ExecutionDevice::pointer device) {
    Image::pointer clone = Image::New();
    clone->createFromImage(mPtr.lock());

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
            OpenCLImageAccess::pointer readAccess = this->getOpenCLImageAccess(ACCESS_READ, clDevice);
            OpenCLImageAccess::pointer writeAccess = clone->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
            cl::Image2D* input = readAccess->get2DImage();
            cl::Image2D* output = writeAccess->get2DImage();

            clDevice->getCommandQueue().enqueueCopyImage(
                    *input,
                    *output,
                    createOrigoRegion(),
                    createOrigoRegion(),
                    createRegion(getWidth(), getHeight(), 1)
            );
        } else {
            OpenCLImageAccess::pointer readAccess = this->getOpenCLImageAccess(ACCESS_READ, clDevice);
            OpenCLImageAccess::pointer writeAccess = clone->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
            cl::Image3D* input = readAccess->get3DImage();
            cl::Image3D* output = writeAccess->get3DImage();

            clDevice->getCommandQueue().enqueueCopyImage(
                    *input,
                    *output,
                    createOrigoRegion(),
                    createOrigoRegion(),
                    createRegion(getWidth(), getHeight(), getDepth())
            );
        }
    }

    return clone;
}


void Image::findDeviceWithUptodateData(ExecutionDevice::pointer& device, bool& isOpenCLImage) {
	isOpenCLImage = false;
    // Check first if there are any OpenCL images
    for(auto iterator : mCLImagesIsUpToDate) {
        if(iterator.second) {
            device = iterator.first;
            isOpenCLImage = true;
            return;
        }
    }

    // OpenCL buffers
    for(auto iterator : mCLBuffersIsUpToDate) {
        if(iterator.second) {
            device = iterator.first;
            return;
        }
    }

    // Host data
    if(mHostDataIsUpToDate) {
        device = Host::getInstance();
    } else {
        throw Exception("Image has no up to date data. This should not be possible!");
    }
}

void Image::fill(float value) {
    if(!isInitialized())
        throw Exception("Image has not been initialized.");

	ExecutionDevice::pointer device;
    bool isOpenCLImage;
    try {
		findDeviceWithUptodateData(device, isOpenCLImage);
    } catch(...) {
    	// Has no data
    	// Create an OpenCL image
    	cl::Image* clImage;
        OpenCLDevice::pointer clDevice = DeviceManager::getInstance()->getDefaultComputationDevice();
    	if(getDimensions() == 2) {
			clImage = new cl::Image2D(
				clDevice->getContext(),
				CL_MEM_READ_WRITE,
				getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE2D, mType, mComponents),
				mWidth, mHeight
			);
    	} else {
			clImage = new cl::Image3D(
				clDevice->getContext(),
				CL_MEM_READ_WRITE,
				getOpenCLImageFormat(clDevice, CL_MEM_OBJECT_IMAGE3D, mType, mComponents),
				mWidth, mHeight, mDepth
			);
    	}
		mCLImages[clDevice] = clImage;
		mCLImagesIsUpToDate[clDevice] = true;
		device = clDevice;
		isOpenCLImage = true;
    }

    if(device->isHost()) {
        throw Exception("Not implemented yet");
    } else {
        OpenCLDevice::pointer clDevice = device;
        cl::CommandQueue queue = clDevice->getCommandQueue();
		std::string sourceFilename = Config::getKernelSourcePath() + "/ImageFill.cl";
		std::string programName = sourceFilename;
		// Only create program if it doesn't exist for this device from before
		if(!clDevice->hasProgram(programName))
			clDevice->createProgramFromSourceWithName(programName, sourceFilename);
		cl::Program program = clDevice->getProgram(programName);
        if(isOpenCLImage) {
            OpenCLImageAccess::pointer access = this->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
            cl_float4 color = {value, value, value, value};
            if(getDimensions() == 2) {
				cl::Kernel kernel(program, "fillImage2D");
				kernel.setArg(0, *access->get2DImage());
				kernel.setArg(1, value);
				queue.enqueueNDRangeKernel(
						kernel,
						cl::NullRange,
						cl::NDRange(mWidth, mHeight),
						cl::NullRange
				);
				// Ideally, we want to use the enqueueFillImage function for this, but it
				// is not working atm on NVIDIA GPUs when visualizing at the same time
            	/*
				queue.enqueueFillImage(
						*access->get2DImage(),
						color,
						createOrigoRegion(),
						createRegion(getSize())
				);
				*/
			} else {
				//throw Exception("Not implemented yet");
                reportWarning() << "Using enqueueFillImage method which may not work while visualizing on NVIDIA GPUs" << reportEnd();
				queue.enqueueFillImage(
						*access->get3DImage(),
						color,
						createOrigoRegion(),
						createRegion(getSize())
				);
			}
        } else {
            OpenCLBufferAccess::pointer access = this->getOpenCLBufferAccess(ACCESS_READ_WRITE, clDevice);
            queue.enqueueFillBuffer(
                    *access->get(),
                    value,
                    0,
                    getBufferSize()
            );
        }
    }
}

Image::pointer Image::crop(VectorXi offset, VectorXi size, bool allowOutOfBoundsCropping) {
    Image::pointer newImage = Image::New();

    bool needInitialization = false;
    VectorXi newImageSize = size;
    VectorXi copySourceOffset = offset;
    VectorXi copyDestinationOffset = VectorXi::Zero(offset.size());
    VectorXi copySize = size;
    if(!allowOutOfBoundsCropping) {
    	// Validate offset
    	if(offset.x() < 0 || offset.y() < 0 || (getDimensions() == 3 && offset.z() < 0)) {
    		throw Exception("Out of bounds cropping not allowed, but offset was below 0.");
    	}
    	// TODO validate size
    } else {
    	// Calculate offsets and sizes
    	for(int i = 0; i < offset.size(); ++i) {
    		if(offset[i] < 0) { // If offset is below zero, copy source offset should be zero
				copySourceOffset[i] = 0;
				copyDestinationOffset[i] = -offset[i];
				needInitialization = true;
				copySize[i] -= copyDestinationOffset[i];
    		}
    		if(copySourceOffset[i] + copySize[i] > getSize()[i]) { // If cut region is larger than image
    			// Reduce copy size
    			copySize[i] -= (copySourceOffset[i] + copySize[i]) - getSize()[i];
				needInitialization = true;
    		}
    	}
    }

    ExecutionDevice::pointer device;
    bool isOpenCLImage;
    findDeviceWithUptodateData(device, isOpenCLImage);
    // Handle host
    if(device->isHost()) {
        throw Exception("Not implemented yet");
    } else {
        OpenCLDevice::pointer clDevice = device;
        if(getDimensions() == 2) {
            if(offset.size() < 2 || size.size() < 2)
                throw Exception("offset and size vectors given to Image::crop must have at least 2 components");
            newImage->create(newImageSize.cast<uint>(), getDataType(), getNrOfComponents());
            if(needInitialization)
				newImage->fill(0);
            OpenCLImageAccess::pointer readAccess = this->getOpenCLImageAccess(ACCESS_READ, clDevice);
            OpenCLImageAccess::pointer writeAccess = newImage->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
            cl::Image2D* input = readAccess->get2DImage();
            cl::Image2D* output = writeAccess->get2DImage();

            /*
            std::cout << "New cropping:" << std::endl;
            std::cout << offset.transpose() << std::endl;
            std::cout << size.transpose() << std::endl;
            std::cout << getSize().transpose() << std::endl;
            std::cout << newImage->getSize().transpose() << std::endl;
            std::cout << copySourceOffset.transpose() << std::endl;
            std::cout << copyDestinationOffset.transpose() << std::endl;
            std::cout << copySize.transpose() << std::endl;
            */
            clDevice->getCommandQueue().enqueueCopyImage(
                    *input,
                    *output,
                    createRegion(copySourceOffset.x(), copySourceOffset.y(), 0),
                    createRegion(copyDestinationOffset.x(), copyDestinationOffset.y(), 0),
                    createRegion(copySize.x(), copySize.y(), 1)
            );
        } else {
            if(offset.size() < 3 || size.size() < 3)
                throw Exception("offset and size vectors given to Image::crop must have at least 3 components");
            newImage->create(newImageSize.cast<uint>(), getDataType(), getNrOfComponents());
            if(needInitialization)
				newImage->fill(0);
            OpenCLImageAccess::pointer readAccess = this->getOpenCLImageAccess(ACCESS_READ, clDevice);
            OpenCLImageAccess::pointer writeAccess = newImage->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
            cl::Image3D* input = readAccess->get3DImage();
            cl::Image3D* output = writeAccess->get3DImage();

            clDevice->getCommandQueue().enqueueCopyImage(
                    *input,
                    *output,
                    createRegion(copySourceOffset.x(), copySourceOffset.y(), 0),
                    createRegion(copyDestinationOffset.x(), copyDestinationOffset.y(), 0),
                    createRegion(copySize.x(), copySize.y(), 1)
            );
        }
    }

    // Fix placement and spacing of the new cropped image
    AffineTransformation::pointer T = AffineTransformation::New();
    newImage->setSpacing(getSpacing());
    // Multiply with spacing here to convert voxel translation to world(mm) translation
    T->getTransform().translation() = getSpacing().cwiseProduct(getDimensions() == 2 ? Vector3f(offset.x(), offset.y(), 0) : Vector3f(offset.x(), offset.y(), offset.z()));
    newImage->getSceneGraphNode()->setTransformation(T);
    SceneGraph::setParentNode(newImage, mPtr.lock());
    reportInfo() << SceneGraph::getAffineTransformationFromData(newImage)->getTransform().matrix() << Reporter::end();
    reportInfo() << SceneGraph::getAffineTransformationFromData(mPtr.lock())->getTransform().matrix() << Reporter::end();

    return newImage;
}

BoundingBox Image::getTransformedBoundingBox() const {
    AffineTransformation::pointer T = SceneGraph::getAffineTransformationFromData(DataObject::pointer(mPtr.lock()));

    // Add image spacing
    T->getTransform().scale(getSpacing());

    return getBoundingBox().getTransformedBoundingBox(T);
}

Image::~Image() {
    freeAll();
}

} // end namespace fast;


