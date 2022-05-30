#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/Access/TensorAccess.hpp>
#include <FAST/Data/Access/Access.hpp>
#include <FAST/Data/TensorShape.hpp>
#include <unordered_map>

namespace fast {

class OpenCLBufferAccess;

/**
 * @brief N-Dimensional tensor data object
 *
 * This object represents an N-dimensional tensor.
 * The data can be stored as a C++ pointer, and as an OpenCL buffer.
 * The tensor data is stored as 32-bit floats.
 *
 * @ingroup data neural-network
 */
class FAST_EXPORT Tensor : public SpatialDataObject {
    FAST_DATA_OBJECT(Tensor)
    public:
#ifndef SWIG
        /**
         * Create a tensor using the provided data and shape.
         * @param data
         * @param shape
         */
        FAST_CONSTRUCTOR(Tensor, std::unique_ptr<float[]>, data,, TensorShape, shape,)
        /**
         * Create a 1D tensor with the provided data. Its shape will be equal to its length
         * @param data
         */
        FAST_CONSTRUCTOR(Tensor, std::initializer_list<float>, data,)
#endif
        /**
         * Create a tensor using the provided data and shape. This method will COPY the data.
         * @param data
         * @param shape
         */
        FAST_CONSTRUCTOR(Tensor, const float* const, data,, TensorShape, shape,)
        /**
         * Create an unitialized tensor with the provided shape
         * @param shape
         */
        FAST_CONSTRUCTOR(Tensor, TensorShape, shape,)
		/**
		 * Add a dimension of size 1 at provided position. -1 is last position.
		 * @param position
		 */
		virtual void expandDims(int position = 0);
        virtual TensorShape getShape() const;
        virtual TensorAccess::pointer getAccess(accessType type);
        virtual std::unique_ptr<OpenCLBufferAccess> getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer);
        virtual void freeAll() override;
        virtual void free(ExecutionDevice::pointer device) override;
        virtual void setSpacing(VectorXf spacing);
        virtual VectorXf getSpacing() const;
        virtual void deleteDimension(int dimension);

        virtual DataBoundingBox getTransformedBoundingBox() const override;
        virtual DataBoundingBox getBoundingBox() const override;
		virtual ~Tensor();

    protected:
        void init(std::unique_ptr<float[]> data, TensorShape shape);
        Tensor() = default;
        virtual bool isInitialized();
        virtual void transferCLBufferFromHost(OpenCLDevice::pointer device);
        void transferCLBufferToHost(OpenCLDevice::pointer device);
        void updateOpenCLBufferData(OpenCLDevice::pointer device);
        void setAllDataToOutOfDate();
        virtual bool hasAnyData();
        void updateHostData();
        virtual float* getHostDataPointer();

        std::unique_ptr<float[]> m_data;
        std::unordered_map<std::shared_ptr<OpenCLDevice>, cl::Buffer*> mCLBuffers;
        std::unordered_map<std::shared_ptr<OpenCLDevice>, bool> mCLBuffersIsUpToDate;
        TensorShape m_shape;
        bool mHostDataIsUpToDate;

        VectorXf m_spacing;

        friend TensorAccess;
        friend OpenCLBufferAccess;
};

}