#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "SpatialDataObject.hpp"
#include "DynamicData.hpp"
#include "DataTypes.hpp"
#include "FAST/SmartPointers.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Access/OpenCLImageAccess.hpp"
#include "FAST/Data/Access/OpenCLBufferAccess.hpp"
#include "FAST/Data/Access/ImageAccess.hpp"
#include <unordered_map>

namespace fast {

class FAST_EXPORT  Image : public SpatialDataObject {
    FAST_OBJECT(Image)
    public:
        void createFromImage(Image::pointer image);
        void create(VectorXui size, DataType type, uint nrOfComponents);
        void create(uint width, uint height, DataType type, uint nrOfComponents);
        void create(uint width, uint height, uint depth, DataType type, uint nrOfComponents);
        void create(VectorXui size, DataType type, uint nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create(uint width, uint height, DataType type, uint nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create(uint width, uint height, uint depth, DataType type, uint nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create(VectorXui size, DataType type, uint nrOfComponents, const void * data);
        void create(uint width, uint height, DataType type, uint nrOfComponents, const void * data);
        void create(uint width, uint height, uint depth, DataType type, uint nrOfComponents, const void * data);

        OpenCLImageAccess::pointer getOpenCLImageAccess(accessType type, OpenCLDevice::pointer);
        OpenCLBufferAccess::pointer getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess::pointer getImageAccess(accessType type);

        ~Image();

        uint getWidth() const;
        uint getHeight() const;
        uint getDepth() const;
        Vector3ui getSize() const;
        uchar getDimensions() const;
        DataType getDataType() const;
        uint getNrOfComponents() const;
        Vector3f getSpacing() const;
        void setSpacing(Vector3f spacing);
        void setSpacing(float x, float y, float z);

        float calculateMaximumIntensity();
        float calculateMinimumIntensity();
        float calculateAverageIntensity();

        // Copy image and put contents to specific device
        Image::pointer copy(ExecutionDevice::pointer device);

        // Create a new image which is a cropped version of this image
        Image::pointer crop(VectorXi offset, VectorXi size, bool allowOutOfBoundsCropping = false);

        // Fill entire image with a value
        void fill(float value);

        // Override
        BoundingBox getTransformedBoundingBox() const;

    protected:
        Image();

        void findDeviceWithUptodateData(ExecutionDevice::pointer& device, bool& isOpenCLImage);

        // OpenCL Images
        std::unordered_map<OpenCLDevice::pointer, cl::Image*> mCLImages;
        std::unordered_map<OpenCLDevice::pointer, bool> mCLImagesIsUpToDate;

        // OpenCL Buffers
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mCLBuffers;
        std::unordered_map<OpenCLDevice::pointer, bool> mCLBuffersIsUpToDate;

        // Host data
        void * mHostData;
        bool mHostHasData;
        bool mHostDataIsUpToDate;

        void setAllDataToOutOfDate();
        bool isInitialized() const;
        void free(ExecutionDevice::pointer device);
        void freeAll();

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
        uint mComponents;
        bool mIsInitialized;

        Vector3f mSpacing;

        float mMaximumIntensity, mMinimumIntensity, mAverageIntensity;
        unsigned long mMaxMinTimestamp, mAverageIntensityTimestamp;
        bool mMaxMinInitialized, mAverageInitialized;
        void calculateMaxAndMinIntensity();

        // Declare as friends so they can get access to the accessFinished methods
        friend class ImageAccess;
        friend class OpenCLBufferAccess;
        friend class OpenCLImageAccess;
};

} // end namespace fast

#endif
