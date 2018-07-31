#pragma once

#include <eigen3/unsupported/Eigen/CXX11/Tensor>
#include <FAST/Object.hpp>
#include <FAST/SmartPointers.hpp>

namespace fast {

class Tensor;

class FAST_EXPORT TensorAccess {
    public:
        typedef std::unique_ptr<TensorAccess> pointer;
        TensorAccess(float* data, std::vector<int> shape, SharedPointer<Tensor> tensor);
        std::vector<int> getShape() const;
        float* getData() const;
        ~TensorAccess();
        void release();
        template <int NumDimensions>
        Eigen::Tensor<float, NumDimensions, Eigen::RowMajor> getDataAsEigenTensorMap() const;
    private:
        SharedPointer<Tensor> m_tensor;
        std::vector<int> m_shape;
        float* m_data;
};


template <int NumDimensions>
Eigen::Tensor<float, NumDimensions, Eigen::RowMajor> TensorAccess::getDataAsEigenTensorMap() const {
    if(NumDimensions != m_shape.size())
        throw Exception("Dimension mismatch for Eigen tensor.");

    // Construct eigen shape
    Eigen::DSizes<long int, NumDimensions> sizes;
    for(int i = 0; i < m_shape.size(); ++i)
        sizes[i] = m_shape[i];

    // Create and return mapped eigen tensor
    return Eigen::TensorMap<Eigen::Tensor<float, NumDimensions, Eigen::RowMajor>>(m_data, sizes);
}


}