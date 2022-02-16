#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/DataTypes.hpp>
#include <FAST/ExecutionDevice.hpp>
#include <FAST/Data/Access/ImageAccess.hpp>
#include <FAST/Data/Access/OpenCLImageAccess.hpp>
#include <FAST/Data/Access/OpenCLBufferAccess.hpp>
#include <FAST/Data/Access/OpenGLTextureAccess.hpp>
#include <FAST/DeviceManager.hpp>
#include <unordered_map>

namespace fast {

#ifndef SWIG
using pixel_deleter_t = std::function<void(void *)>;
using unique_pixel_ptr = std::unique_ptr<void, pixel_deleter_t>;
template<typename T>
auto pixel_deleter(void const * data) -> void
{
    T const * p = static_cast<T const*>(data);
    //std::cout << "[" << (uint64_t)p <<  "] is being deleted." << std::endl;
    delete[] p;
}

template<typename T>
auto make_unique_pixel(T * ptr) -> unique_pixel_ptr {
    return unique_pixel_ptr(ptr, &pixel_deleter<T>);
}
unique_pixel_ptr allocatePixelArray(std::size_t size, DataType type);
#endif

/**
 * @brief Image data
 *
 * Class for image data, represents an image which can be stored in multiple different locations such as OpenCL Image,
 * Buffer, OpenGL texture and as a C++ pointer on the host.
 *
 * @ingroup data
 */
class FAST_EXPORT Image : public SpatialDataObject {
    FAST_DATA_OBJECT(Image)
    public:
        /**
         * @brief Create an image with same size and type as another image.
         *
         * Setup an image object with the same size, data type and pixel spacing as the given image.
         * Does not allocate any memory.
         *
         * @param image to copy size and pixel spacing from
         */
        static std::shared_ptr<Image> createFromImage(Image::pointer image) {
            auto ptr = std::shared_ptr<Image>(new Image(std::move(image)));
            ptr->setPtr(ptr);
            return ptr;
        }
        static std::shared_ptr<Image> createSegmentationFromImage(Image::pointer image) {
            auto ptr = std::shared_ptr<Image>(new Image(image->getSize(), TYPE_UINT8, 1));
            ptr->setSpacing(image->getSpacing());
            SceneGraph::setParentNode(ptr, image);
            ptr->setPtr(ptr);
            return ptr;
        }
        Image(Image::pointer image);
#ifndef SWIG
        /**
         * Setup a 2D/3D image object, but does not allocate any memory
         *
         * @param size
         * @param type
         * @param nrOfChannels
         */
        FAST_CONSTRUCTOR(Image, VectorXui, size,, DataType, type,, uint, nrOfChannels,);
#endif
        /**
         * Setup a 2D image object, but does not allocate any memory
         *
         * @param width
         * @param height
         * @param type
         * @param nrOfChannels
         */
        FAST_CONSTRUCTOR(Image, uint, width,, uint, height,, DataType, type,, uint, nrOfChannels,);
        /**
         * Setup a 3D image object, but does not allocate any memory.
         *
         * @param width
         * @param height
         * @param depth
         * @param type
         * @param nrOfChannels
         */
        FAST_CONSTRUCTOR(Image, uint, width,, uint, height,, uint, depth,, DataType, type,, uint, nrOfChannels,);
#ifndef SWIG
        /**
         * Copies 2D/3D data to given device
         *
         * @param size
         * @param type
         * @param nrOfChannels
         * @param device
         * @param data
         */
        FAST_CONSTRUCTOR(Image, VectorXui, size,, DataType, type,, uint, nrOfChannels,, ExecutionDevice::pointer, device,,
                    const void *const, data,);
#endif
        /**
         * Copies 2D data to given device
         *
         * @param width
         * @param height
         * @param type
         * @param nrOfChannels
         * @param device
         * @param data
         */
        FAST_CONSTRUCTOR(Image, uint, width,, uint, height,, DataType, type,, uint, nrOfChannels,, ExecutionDevice::pointer, device,, const void * const, data,);
        /**
         * Copies 3D data to given device
         *
         * @param width
         * @param height
         * @param depth
         * @param type
         * @param nrOfChannels
         * @param device
         * @param data
         */
        FAST_CONSTRUCTOR(Image, uint, width,, uint, height,, uint, depth,, DataType, type,, uint, nrOfChannels,, ExecutionDevice::pointer, device,, const void * const, data,);
#ifndef SWIG
        /**
         * Copies 2D/3D data to default device
         *
         * @param size
         * @param type
         * @param nrOfChannels
         * @param data
         */
        FAST_CONSTRUCTOR(Image, VectorXui, size,, DataType, type,, uint, nrOfChannels,, const void* const, data,);
#endif
        /**
         * Copies 2D data to default device
         *
         * @param width
         * @param height
         * @param type
         * @param nrOfChannels
         * @param data
         */
         FAST_CONSTRUCTOR(Image, uint, width,, uint, height,, DataType, type,, uint, nrOfChannels,, const void* const, data,);
        /**
         * Copies 3D data to default device
         *
         * @param width
         * @param height
         * @param depth
         * @param type
         * @param nrOfChannels
         * @param data
         */
        FAST_CONSTRUCTOR(Image, uint, width,, uint, height,, uint, depth,, DataType, type,, uint, nrOfChannels,, const void* const, data,);

        /**
         * Moves the 2D data pointer to the given device
         *
         * @param width
         * @param height
         * @param type
         * @param nrOfChannels
         * @param device
         * @param data
         */
        template <class T>
        static Image::pointer create(uint width, uint height, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> data);
        /**
         * Moves the 3D pointer to the given device
         *
         * @param width
         * @param height
         * @param depth
         * @param type
         * @param nrOfChannels
         * @param device
         * @param data
         */
        template <class T>
        static Image::pointer create(uint width, uint height, uint depth, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> data);

        /**
         * Moves the 2D/3D pointer to the given device
         *
         * @tparam T
         * @param type
         * @param nrOfChannels
         * @param device
         * @param ptr
         */
        template <class T>
        static Image::pointer create(VectorXui, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr);

        /**
         * Moves the 2D data pointer to the default device
         *
         * @param width
         * @param height
         * @param type
         * @param nrOfChannels
         * @param data
         */
        template <class T>
        static Image::pointer create(uint width, uint height, DataType type, uint nrOfChannels, std::unique_ptr<T> ptr);
        /**
         * Moves the 3D data pointer to the default device
         *
         * @param width
         * @param height
         * @param depth
         * @param type
         * @param nrOfChannels
         * @param data
         */
        template <class T>
        static Image::pointer create(uint width, uint height, uint depth, DataType type, uint nrOfChannels, std::unique_ptr<T> ptr);

        /**
         * Moves the 2D/3D data pointer to the default device
         *
         * @tparam T
         * @param type
         * @param nrOfChannels
         * @param ptr
         */
        template <class T>
        static Image::pointer create(VectorXui, DataType type, uint nrOfChannels, std::unique_ptr<T> ptr);

        OpenCLImageAccess::pointer getOpenCLImageAccess(accessType type, OpenCLDevice::pointer);
        OpenCLBufferAccess::pointer getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess::pointer getImageAccess(accessType type);
        OpenGLTextureAccess::pointer getOpenGLTextureAccess(accessType type, OpenCLDevice::pointer, bool compress = false);

        ~Image();

        int getWidth() const;
        int getHeight() const;
        int getDepth() const;
        /**
         * @return the number of pixels/voxels width*height*depth
         */
        int getNrOfVoxels() const;
        Vector3ui getSize() const;
        uchar getDimensions() const;
        DataType getDataType() const;
        int getNrOfChannels() const;
        Vector3f getSpacing() const;
        void setSpacing(Vector3f spacing);
        void setSpacing(float x, float y, float z);

        float calculateMaximumIntensity();
        float calculateMinimumIntensity();
        float calculateAverageIntensity();
        float calculateSumIntensity();

        /**
         * Copy image and put contents to specific device
         */
        Image::pointer copy(ExecutionDevice::pointer device);

        /**
         * Create a new image which is a cropped version of this image
         */
        Image::pointer crop(VectorXi offset, VectorXi size, bool allowOutOfBoundsCropping = false, int croppingValue = 0);

        /**
         * Fill entire image with a value
         * @param value
         */
        void fill(float value);

        /**
         * Checks wheter this image is of segmentation type (UINT8 and 1 channel)
         * @return
         */
        bool isSegmentationType() const;

        // Override
        DataBoundingBox getTransformedBoundingBox() const override;
        DataBoundingBox getBoundingBox() const override;

        void free(ExecutionDevice::pointer device) override;
        void freeAll() override;
    protected:
        Image();
        template <class T>
        Image(uint width, uint height, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr);
        template <class T>
        Image(uint width, uint height, uint depth, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr);
        template <class T>
        Image(VectorXui size, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr);

        /**
         * Give data to appropriate device
         *
         * @param device
         * @param data
         */
        void setData(ExecutionDevice::pointer device, void* data);
        /**
         * Copy data to appropriate device
         *
         * @param device
         * @param data
         */
        void copyData(ExecutionDevice::pointer device, const void* const data);

        void findDeviceWithUptodateData(ExecutionDevice::pointer& device, bool& isOpenCLImage);

        // OpenCL Images
        std::unordered_map<OpenCLDevice::pointer, cl::Image*> mCLImages;
        std::unordered_map<OpenCLDevice::pointer, bool> mCLImagesIsUpToDate;

        // OpenCL Buffers
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mCLBuffers;
        std::unordered_map<OpenCLDevice::pointer, bool> mCLBuffersIsUpToDate;

        // Host data
        unique_pixel_ptr mHostData;
        bool mHostHasData;
        bool mHostDataIsUpToDate;

        // OpenGL data
        uint m_GLtextureID = 0;
        bool m_GLtextureUpToDate = false;

        void setAllDataToOutOfDate();
        bool isInitialized() const;

        void updateOpenCLImageData(OpenCLDevice::pointer device);
        void transferCLImageFromHost(OpenCLDevice::pointer device);
        void transferCLImageToHost(OpenCLDevice::pointer device);

        void updateOpenCLBufferData(OpenCLDevice::pointer device);
        void transferCLBufferFromHost(OpenCLDevice::pointer device);
        void transferCLBufferToHost(OpenCLDevice::pointer device);

        void updateHostData();

        bool hasAnyData();

        uint getBufferSize() const;

        uint mWidth, mHeight, mDepth;
        uchar mDimensions;
        DataType mType;
        uint mChannels;
        bool mIsInitialized;

        Vector3f mSpacing;

        float mMaximumIntensity, mMinimumIntensity, mSumIntensity;
        unsigned long mMaxMinTimestamp, mSumIntensityTimestamp;
        bool mMaxMinInitialized, mSumInitialized;
        void calculateMaxAndMinIntensity();

        // Declare as friends so they can get access to the accessFinished methods
        friend class ImageAccess;
        friend class OpenCLBufferAccess;
        friend class OpenCLImageAccess;

        void init(VectorXui, DataType type, uint nrOfChannels);
        void init(uint width, uint height, DataType type, uint nrOfChannels);
        void init(uint width, uint height, uint depth, DataType type, uint nrOfChannels);

};

template <class T>
Image::pointer Image::create(uint width, uint height, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr) {
    auto resPtr = std::shared_ptr<Image>(new Image(width, height, type, nrOfChannels, device, ptr));
    resPtr->setPtr(resPtr);
    return resPtr;
}

template <class T>
Image::pointer Image::create(uint width, uint height, uint depth, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr) {
    auto resPtr = std::shared_ptr<Image>(new Image(width, height, depth, type, nrOfChannels, device, std::move(ptr)));
    resPtr->setPtr(resPtr);
    return resPtr;
}

template <class T>
Image::pointer Image::create(uint width, uint height, DataType type, uint nrOfChannels, std::unique_ptr<T> ptr) {
    auto resPtr = std::shared_ptr<Image>(new Image(width, height, type, nrOfChannels, DeviceManager::getInstance()->getDefaultDevice(), std::move(ptr)));
    resPtr->setPtr(resPtr);
    return resPtr;
}

template <class T>
Image::pointer Image::create(uint width, uint height, uint depth, DataType type, uint nrOfChannels, std::unique_ptr<T> ptr) {
    auto resPtr = std::shared_ptr<Image>(new Image(width, height, depth, type, nrOfChannels, DeviceManager::getInstance()->getDefaultDevice(), std::move(ptr)));
    resPtr->setPtr(resPtr);
    return resPtr;
}

template <class T>
Image::pointer Image::create(VectorXui size, DataType type, uint nrOfChannels, std::unique_ptr<T> ptr) {
    auto resPtr = std::shared_ptr<Image>(new Image(size, type, nrOfChannels, DeviceManager::getInstance()->getDefaultDevice(), std::move(ptr)));
    resPtr->setPtr(resPtr);
    return resPtr;
}

template <class T>
Image::pointer Image::create(VectorXui size, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr) {
    auto resPtr = std::shared_ptr<Image>(new Image(size, type, nrOfChannels, device, std::move(ptr)));
    resPtr->setPtr(resPtr);
    return resPtr;
}

template <class T>
Image::Image(uint width, uint height, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr) : Image() {
    init(width, height, type, nrOfChannels);
    setData(device, ptr.release()); 
}
template <class T>
Image::Image(uint width, uint height, uint depth, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr) : Image() {
    init(width, height, depth, type, nrOfChannels);
    setData(device, ptr.release()); 
}
template <class T>
Image::Image(VectorXui size, DataType type, uint nrOfChannels, ExecutionDevice::pointer device, std::unique_ptr<T> ptr) : Image() {
    if(size.size() == 2) {
        init(size.x(), size.y(), type, nrOfChannels);
    } else {
        init(size.x(), size.y(), size.z(), type, nrOfChannels);
    }
    setData(device, ptr.release()); 
}

} // end namespace fast

