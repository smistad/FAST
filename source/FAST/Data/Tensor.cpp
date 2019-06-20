#include "Tensor.hpp"
#include <FAST/Utility.hpp>

namespace fast {

void Tensor::create(std::unique_ptr<float[]> data, TensorShape shape) {
    if(shape.empty())
        throw Exception("Shape can't be empty");
    m_data = std::move(data);
    m_shape = shape;
}

void Tensor::create(TensorShape shape) {
    if(shape.empty())
        throw Exception("Shape can't be empty");
    if(shape.getUnknownDimensions() > 0)
        throw Exception("When creating a tensor, shape must be fully defined");
    m_data = make_uninitialized_unique<float[]>(shape.getTotalSize());
}

void Tensor::create(std::initializer_list<float> data) {
	if (data.size() == 0)
		throw Exception("Shape can't be empty");

	m_data = std::make_unique<float[]>(data.size());
	int i = 0;
	for (auto item : data) {
		m_data[i] = item;
		++i;
	}
	m_shape = TensorShape({ (int)data.size() });
}

void Tensor::expandDims(int position) {
	if(position < 0) { // append to end
		m_shape.addDimension(1);
	} else {
		m_shape.insertDimension(position, 1);
	}	
}

TensorShape Tensor::getShape() const {
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