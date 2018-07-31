#include "TensorAccess.hpp"
#include <FAST/Data/Tensor.hpp>

namespace fast {

TensorAccess::TensorAccess(float *data, std::vector<int> shape, SharedPointer<Tensor> tensor) {
    m_data = data;
    m_shape = shape;
    m_tensor = tensor;
}

std::vector<int> TensorAccess::getShape() const {
    return m_shape;
}

TensorAccess::~TensorAccess() {
    release();
}

void TensorAccess::release() {
    m_tensor->accessFinished();
}

}