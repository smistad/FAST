#include "TensorShape.hpp"
#include <algorithm>

namespace fast {

TensorShape::TensorShape(std::vector<int> dimensions) : m_data(dimensions) {
}

TensorShape::TensorShape(std::initializer_list<int> dimensions) : m_data(dimensions) {
}

TensorShape::TensorShape(VectorXi dimensions) {
    for(int i = 0; i < dimensions.size(); ++i)
        m_data.push_back(dimensions[i]);
}

bool TensorShape::empty() const {
    return m_data.empty();
}

int TensorShape::getTotalSize() const {
    int product = 1;
    for(auto i : m_data) {
        if(i >= 0)
            product *= i;
    }
    return product;
}

int TensorShape::getDimensions() const {
    return static_cast<int>(m_data.size());
}

int TensorShape::getKnownDimensions() const {
    return static_cast<int>(std::count_if(m_data.begin(), m_data.end(), [](int i){ return i >= 0;}));
}

int TensorShape::getUnknownDimensions() const {
    return static_cast<int>(std::count_if(m_data.begin(), m_data.end(), [](int i){ return i < 0;}));
}

std::vector<int> TensorShape::getAll() const {
    return m_data;
}

void TensorShape::setDimension(int i, int value) {
    m_data.at(i) = value;
}

void TensorShape::addDimension(int value) {
    m_data.push_back(value);
}

void TensorShape::insertDimension(int position, int value) {
	m_data.insert(m_data.begin() + position, value);
}

TensorShape::TensorShape(const TensorShape &other) {
    m_data = other.getAll();
}

TensorShape &TensorShape::operator=(const TensorShape &other) {
    m_data = other.getAll();
    return *this;
}

int& TensorShape::operator[](int i) {
    return m_data.at(i);
}

const int& TensorShape::operator[](int i) const {
    return m_data.at(i);
}

std::string TensorShape::toString() const {
    std::string str;
    for(int i : m_data)
        str += std::to_string(i) + " ";
    return str;
}

void TensorShape::deleteDimension(int index) {
    m_data.erase(m_data.begin() + index);
}

void TensorShape::deleteDimensions(int startIndex, int endIndex) {
    m_data.erase(m_data.begin() + startIndex, m_data.begin() + endIndex);
}

}