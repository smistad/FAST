#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/Access/TensorAccess.hpp>
#include <FAST/Data/Access/Access.hpp>
#include <FAST/Data/TensorShape.hpp>

namespace fast {

class OpenCLBufferAccess;

class FAST_EXPORT Tensor : public SpatialDataObject {
    FAST_OBJECT(Tensor)
    public:
        /**
         * Create a tensor using the provided data and shape
         * @param data
         * @param shape
         */
        virtual void create(std::unique_ptr<float[]> data, TensorShape shape);
        /**
         * Create an unitialized tensor with the provided shape
         * @param shape
         */
        virtual void create(TensorShape shape);
		/**
		 * Create a 1D tensor with the provided data. Its shape will be equal to its length
		 * @param data
		 */
		virtual void create(std::initializer_list<float> data);
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

        virtual BoundingBox getTransformedBoundingBox() const override;
        virtual BoundingBox getBoundingBox() const override;
		virtual ~Tensor();

    protected:
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
        std::unordered_map<SharedPointer<OpenCLDevice>, cl::Buffer*> mCLBuffers;
        std::unordered_map<SharedPointer<OpenCLDevice>, bool> mCLBuffersIsUpToDate;
        TensorShape m_shape;
        bool mHostDataIsUpToDate;

        VectorXf m_spacing;

        friend TensorAccess;
        friend OpenCLBufferAccess;
};

}