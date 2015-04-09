#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "DynamicData.hpp"
#include "DataTypes.hpp"
#include "SmartPointers.hpp"
#include "OpenCLManager.hpp"
#include "ExecutionDevice.hpp"
#include "OpenCLImageAccess2D.hpp"
#include "OpenCLImageAccess3D.hpp"
#include "OpenCLBufferAccess.hpp"
#include "ImageAccess.hpp"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
namespace fast {

class Image : public DataObject {
    FAST_OBJECT(Image)
    public:
        void createFromImage(Image::pointer image, ExecutionDevice::pointer device);
        void create2DImage(unsigned int width, unsigned int height, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device);
        void create2DImage(unsigned int width, unsigned int height, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create3DImage(unsigned int width, unsigned int height, unsigned int depth, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device);
        void create3DImage(unsigned int width, unsigned int height, unsigned int depth, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);

        OpenCLImageAccess2D::pointer getOpenCLImageAccess2D(accessType type, OpenCLDevice::pointer);
        OpenCLImageAccess3D::pointer getOpenCLImageAccess3D(accessType type, OpenCLDevice::pointer);
        OpenCLBufferAccess::pointer getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess::pointer getImageAccess(accessType type);

        ~Image() { freeAll(); };

        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned int getDepth() const;
        unsigned char getDimensions() const;
        DataType getDataType() const;
        unsigned int getNrOfComponents() const;

        Vector3f getSpacing() const;
        Vector3f getOffset() const;
        Vector3f getCenterOfRotation() const;
        Matrix3f getTransformMatrix() const;
        void setSpacing(Vector3f spacing);
        void setOffset(Vector3f offset);
        void setCenterOfRotation(Vector3f rotation);
        void setTransformMatrix(Matrix3f transformMatrix);

        float calculateMaximumIntensity();
        float calculateMinimumIntensity();

        // Copy image and put contents to specific device
        Image::pointer copy(ExecutionDevice::pointer device);
    private:
        Image();

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

        void updateSceneGraphTransformation() const;

        unsigned int getBufferSize() const;

        unsigned int mWidth, mHeight, mDepth;
        unsigned char mDimensions;
        DataType mType;
        unsigned int mComponents;
        bool mImageIsBeingWrittenTo;

        Vector3f mSpacing, mOffset, mCenterOfRotation;
        Matrix3f mTransformMatrix;

        float mMaximumIntensity, mMinimumIntensity;
        unsigned long mMaxMinTimestamp;
        bool mMaxMinInitialized;
        void calculateMaxAndMinIntensity();
};

typedef DynamicData<Image> DynamicImage;

} // end namespace fast

#endif
