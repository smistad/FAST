#include "Tensor.hpp"
#include <FAST/Utility.hpp>

namespace fast {

void Tensor::create(std::unique_ptr<float[]> data, std::vector<int> shape) {
    if(shape.size() == 0)
        throw Exception("Shape can't be empty");
    m_data = std::move(data);
    m_shape = shape;
}

void Tensor::create(std::vector<int> shape) {
    if(shape.size() == 0)
        throw Exception("Shape can't be empty");
    int size = std::accumulate(shape.begin(), shape.end(), 0);
    m_data = make_uninitialized_unique<float[]>(size);
}

std::vector<int> Tensor::getShape() const {
    return m_shape;
}

TensorAccess::pointer Tensor::getAccess(accessType type) {
    // TODO process type
    return std::make_unique<TensorAccess>(m_data.get(), m_shape, std::static_pointer_cast<Tensor>(mPtr.lock()));
}

void Tensor::free(ExecutionDevice::pointer device) {

}

void Tensor::freeAll() {

}


}