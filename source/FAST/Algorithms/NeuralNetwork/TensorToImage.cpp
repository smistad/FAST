#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/Image.hpp>
#include "TensorToImage.hpp"

namespace fast {

TensorToImage::TensorToImage(std::vector<int> channels) {
    createInputPort<Tensor>(0);
    createOutputPort<Image>(0);
    m_channels = channels;

    createIntegerAttribute("channels", "Channels", "Channels to use from tensor", -1);
}

void TensorToImage::execute() {
    auto tensor = getInputData<Tensor>();
    const auto shape = tensor->getShape();
    auto access = tensor->getAccess(ACCESS_READ);
    const int dims = shape.getDimensions();
    const int channelsInTensor = shape[dims-1];
    const int outputWidth = shape[dims-2];
    const int outputHeight = shape[dims-3];
    int outputDepth = 1;
    if(dims == 5) {
        outputDepth = shape[dims - 4];
    }
    float* tensorData = access->getRawData();

    Image::pointer image;
    if(m_channels.empty()) {
        if(outputDepth == 1) {
            image = Image::create(outputWidth, outputHeight, TYPE_FLOAT, channelsInTensor, tensorData);
        } else {
            image = Image::create(outputWidth, outputHeight, outputDepth, TYPE_FLOAT, channelsInTensor, tensorData);
        }
    } else {
        // Select specific channels from tensor data
        auto newTensorData = make_uninitialized_unique<float[]>(outputWidth*outputHeight*outputDepth*m_channels.size());
        int counter = 0;
        if(outputDepth == 1) {
            auto tensorData = access->getData<3>();
            for(int channel : m_channels) {
                Eigen::array<int, 3> offsets = {0, 0, channel};
                Eigen::array<int, 3> extents = {outputHeight, outputWidth, 1};
                Eigen::Tensor<float, 3, Eigen::RowMajor, int> res = tensorData.slice(offsets, extents);
                std::memcpy(&newTensorData[outputWidth*outputHeight*counter], res.data(), sizeof(float)*outputWidth*outputHeight);
                ++counter;
            }
            image = Image::create(outputWidth, outputHeight, TYPE_FLOAT, m_channels.size(), std::move(newTensorData));
        } else {
            auto tensorData = access->getData<4>();
            for(int channel : m_channels) {
                Eigen::array<int, 4> offsets = {0, 0, 0, channel};
                Eigen::array<int, 4> extents = {outputDepth, outputHeight, outputWidth, 1};
                Eigen::Tensor<float, 4, Eigen::RowMajor, int> res = tensorData.slice(offsets, extents);
                std::memcpy(&newTensorData[outputWidth*outputHeight*outputDepth*counter], res.data(), sizeof(float)*outputWidth*outputHeight*outputDepth);
                ++counter;
            }
            image = Image::create(outputWidth, outputHeight, outputDepth, TYPE_FLOAT, m_channels.size(), std::move(newTensorData));
        }
    }
    image->setSpacing(tensor->getSpacing());
    SceneGraph::setParentNode(image, tensor);
    addOutputData(0, image);
}

void TensorToImage::loadAttributes() {
    auto channels = getIntegerListAttribute("channels");
    if(!channels.empty() && channels[0] >= 0) {
        setChannels(channels);
    }
}

void TensorToImage::setChannels(std::vector<int> channels) {
    m_channels = channels;
    setModified(true);
}

}