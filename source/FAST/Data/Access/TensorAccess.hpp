#pragma once

#include <eigen3/unsupported/Eigen/CXX11/Tensor>
#include <FAST/Object.hpp>
#include <FAST/SmartPointers.hpp>

namespace fast {

// Rename the Eigen float tensor for simplicity
template<int NumDimensions>
using TensorData = Eigen::TensorMap<Eigen::Tensor<float, NumDimensions, Eigen::RowMajor>>;

class Tensor;

class FAST_EXPORT TensorAccess {
    public:
        typedef std::unique_ptr<TensorAccess> pointer;
        TensorAccess(float* data, std::vector<int> shape, SharedPointer<Tensor> tensor);
        std::vector<int> getShape() const;
        ~TensorAccess();
        void release();
        template <int NumDimensions>
        TensorData<NumDimensions> getData() const;
    private:
        SharedPointer<Tensor> m_tensor;
        std::vector<int> m_shape;
        float* m_data;
};


template <int NumDimensions>
TensorData<NumDimensions> TensorAccess::getData() const {
    if(NumDimensions != m_shape.size())
        throw Exception("Dimension mismatch for Eigen tensor.");

    // Construct eigen shape
    Eigen::DSizes<long int, NumDimensions> sizes;
    for(int i = 0; i < m_shape.size(); ++i)
        sizes[i] = m_shape[i];

    // Create and return mapped eigen tensor
    return TensorData<NumDimensions>(m_data, sizes);
}


}