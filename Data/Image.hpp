#ifndef IMAGE2D_HPP
#define IMAGE2D_HPP

#include "ImageData.hpp"
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

class Image: public ImageData {
    FAST_OBJECT(Image)
    public:
        void create2DImage(unsigned int width, unsigned int height, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device);
        void create2DImage(unsigned int width, unsigned int height, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);
        void create3DImage(unsigned int width, unsigned int height, unsigned int depth, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device);
        void create3DImage(unsigned int width, unsigned int height, unsigned int depth, DataType type, unsigned int nrOfComponents, ExecutionDevice::pointer device, const void * data);

        OpenCLImageAccess2D getOpenCLImageAccess2D(accessType type, OpenCLDevice::pointer);
        OpenCLImageAccess3D getOpenCLImageAccess3D(accessType type, OpenCLDevice::pointer);
        OpenCLBufferAccess getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer);
        ImageAccess getImageAccess(accessType type);

        ~Image() { freeAll(); };

        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned int getDepth() const;
        unsigned char getDimensions() const;
        DataType getDataType() const;
        unsigned int getNrOfComponents() const;

        Float<3> getSpacing() const;
        Float<3> getOffset() const;
        Float<3> getCenterOfRotation() const;
        Float<9> getTransformMatrix() const;
        void setSpacing(Float<3> spacing);
        void setOffset(Float<3> offset);
        void setCenterOfRotation(Float<3> rotation);
        void setTransformMatrix(Float<9> transformMatrix);
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

        unsigned int getBufferSize() const;

        unsigned int mWidth, mHeight, mDepth;
        unsigned char mDimensions;
        DataType mType;
        unsigned int mComponents;
        bool mImageIsBeingWrittenTo;

        Float<3> mSpacing, mOffset, mCenterOfRotation;
        Float<9> mTransformMatrix;
};

} // end namespace fast

#endif
