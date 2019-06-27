#include "ImageToBatchGenerator.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>

namespace fast {

ImageToBatchGenerator::ImageToBatchGenerator() {
    createInputPort<Image>(0);
    createOutputPort<Batch>(0);

    m_maxBatchSize = -1;
}

void ImageToBatchGenerator::execute() {
    if(m_maxBatchSize == 1)
        throw Exception("Max batch size must be given to the ImageToBatchGenerator");

    auto image = getInputData<Image>(0);
    // TODO need to detect end of stream
    std::cout << "ADDED IMAGE TO LIST IN BATCH GENERATOR" << std::endl;
    m_imageList.push_back(image);
    if(m_imageList.size() == m_maxBatchSize) {
        std::cout << "CREATING BATCH" << std::endl;
        auto batch = Batch::New();
        batch->create(m_imageList);
        addOutputData(0, batch);
        m_imageList.clear();
    }
}

void ImageToBatchGenerator::setMaxBatchSize(int size) {
    if(size <= 0)
        throw Exception("Max batch size must be larger than 0");
    m_maxBatchSize = size;
    m_imageList.reserve(size);
    mIsModified = true;
}

bool ImageToBatchGenerator::hasReachedEnd() {
    return false;
}

}