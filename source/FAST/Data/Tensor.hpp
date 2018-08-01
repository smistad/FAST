#pragma once

#include <FAST/Data/DataObject.hpp>
#include <FAST/Data/Access/TensorAccess.hpp>
#include <FAST/Data/Access/Access.hpp>
#include <FAST/Data/TensorShape.hpp>

namespace fast {

class FAST_EXPORT Tensor : public DataObject {
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
        virtual TensorShape getShape() const;
        virtual TensorAccess::pointer getAccess(accessType type);
        // TODO add OpenCL buffer access
        virtual void freeAll() override;
        virtual void free(ExecutionDevice::pointer device) override;

    protected:
        Tensor() = default;

        std::unique_ptr<float[]> m_data;
        TensorShape m_shape;

        friend TensorAccess;
};

}