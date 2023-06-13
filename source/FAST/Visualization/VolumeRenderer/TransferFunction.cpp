#include "TransferFunction.hpp"

namespace fast {

void TransferFunction::checkData() const {
    if(m_data.size() % 5 != 0)
        throw Exception("Number of float data points given to TransferFunction must be dividable by 5."
                        "Each point in transfer function must be 5 tuple: (intensity, red, green, blue, alpha)");

    for(int i = 5; i < m_data.size(); i += 5) {
        if(m_data[i-5] > m_data[i]) {
            throw Exception("The points given to transfer function must be sorted based on intensity value");
        }
    }
}

TransferFunction::TransferFunction(std::initializer_list<float> data) {
    for(auto item : data) {
        m_data.push_back(item);
    }
    checkData();
}

int TransferFunction::addPoint(float intensity, Vector4f color) {
    m_data.push_back(intensity);
    for(int i = 0; i < 4; ++i)
        m_data.push_back(color[i]);
    checkData();
    return (int)m_data.size()/5;
}

void TransferFunction::removePoint(int i) {
    m_data.erase(m_data.begin() + i*5, m_data.begin() + (i+1)*5);
    checkData();
}

void TransferFunction::clear() {
    m_data.clear();
}

int TransferFunction::getSize() const {
    return (int)m_data.size()/5;
}

std::tuple<float, Vector4f> TransferFunction::getPoint(int i) const {
    float intensity = m_data[i*5];
    Vector4f color;
    for(int i = 1; i < 5; ++i)
        color[i] = m_data[i*5+i];

    return std::make_tuple(intensity, color);
}

cl::Buffer TransferFunction::getAsOpenCLBuffer(OpenCLDevice::pointer device) const {
    if(m_data.empty())
        throw Exception("Trying to get OpenCL buffer of empty TransferFunction");
    checkData();
    return cl::Buffer(
        device->getContext(),
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        m_data.size()*sizeof(float),
        (void*)m_data.data()
    );
}

TransferFunction::TransferFunction(std::vector<float> values) {
    for(auto item : values) {
        m_data.push_back(item);
    }
    checkData();
}

}