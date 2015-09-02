#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "SpatialDataObject.hpp"
#include "DynamicData.hpp"
#include "DataTypes.hpp"
#include "FAST/SmartPointers.hpp"
#include "OpenCLManager.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Access/OpenCLImageAccess.hpp"
#include "FAST/Data/Access/OpenCLBufferAccess.hpp"
#include "FAST/Data/Access/ImageAccess.hpp"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
namespace fast {

class Image : public SpatialDataObject {
    FAST_OBJECT(Image)
    public:
        void createFromImage(Image::pointer image);
        void create(VectorXui size, DataType type, unsigned int nrOfComponents);
        void create(unsigned int width, unsigned int height, DataType type, unsigned int nrOfComponents);
        void create(unsigned int width, unsigned int height, unsigned int depth, DataType type, unsigned int nrOfComponents);
        void create(VectorXui size, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create(unsigned int width, unsigned int height, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create(unsigned int width, unsigned int height, unsigned int depth, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);

        OpenCLImageAccess::pointer getOpenCLImageAccess(accessType type, OpenCLDevice::pointer);
        OpenCLBufferAccess::pointer getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess::pointer getImageAccess(accessType type);

        ~Image() { freeAll(); };

        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned int getDepth() const;
        Vector3ui getSize() const;
        unsigned char getDimensions() const;
        DataType getDataType() const;
        unsigned int getNrOfComponents() const;
        Vector3f getSpacing() const;
        void setSpacing(Vector3f spacing);

        float calculateMaximumIntensity();
        float calculateMinimumIntensity();
        float calculateAverageIntensity();

        // Copy image and put contents to specific device
        Image::pointer copy(ExecutionDevice::pointer device);

        // Create a new image which is a cropped version of this image
        Image::pointer crop(VectorXui offset, VectorXui size);

        // Override
        BoundingBox getTransformedBoundingBox() const;
    protected:
        Image();

        void findDeviceWithUptodateData(ExecutionDevice::pointer* device, bool* isOpenCLImage);

        // OpenCL Images
        boost::unordered_map<OpenCLDevice::pointer, cl::Image*> mCLImages;
        boost::unordered_map<OpenCLDevice::pointer, bool> mCLImagesIsUpToDate;
        boost::unordered_map<OpenCLDevice::pointer, bool> mCLImagesAccess;

        // OpenCL Buffers
        boost::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mCLBuffers;
        boost::unordered_map<OpenCLDevice::pointer, bool> mCLBuffersIsUpToDate;
        boost::unordered_map<OpenCLDevice::pointer, bool> mCLBuffersAccess;

        // Host data
        void * mHostData;
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        bool mHostDataIsBeingAccessed;

        bool isDataModified();
        void setAllDataToOutOfDate();
        bool isAnyDataBeingAccessed();
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

        unsigned int getBufferSize() const;

        unsigned int mWidth, mHeight, mDepth;
        unsigned char mDimensions;
        DataType mType;
        unsigned int mComponents;
        bool mImageIsBeingWrittenTo;
        bool mIsInitialized;

        Vector3f mSpacing;

        float mMaximumIntensity, mMinimumIntensity, mAverageIntensity;
        unsigned long mMaxMinTimestamp, mAverageIntensityTimestamp;
        bool mMaxMinInitialized, mAverageInitialized;
        void calculateMaxAndMinIntensity();
};

} // end namespace fast

#endif
