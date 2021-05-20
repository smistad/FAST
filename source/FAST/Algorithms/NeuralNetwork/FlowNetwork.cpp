#include "FlowNetwork.hpp"
#include <FAST/Data/Image.hpp>
#include "TensorToImage.hpp"

namespace fast {

FlowNetwork::FlowNetwork() {
    createInputPort<Sequence>(0);
    createOutputPort<Image>(0);
}

void FlowNetwork::execute() {
    // Prepare input node: Should be two images in a sequence
    run();

    auto tensor = std::dynamic_pointer_cast<Tensor>(m_processedOutputData[0]);
    if(!tensor)
        throw Exception("Unable to cast processed output data to Tensor");

    auto tensorToImage = TensorToImage::New();
    tensorToImage->setInputData(tensor);
    addOutputData(0, tensorToImage->updateAndGetOutputData<Image>());
}

ImagesToSequence::ImagesToSequence() {
    createInputPort<Image>(0);
    createOutputPort<Sequence>(0);

    createIntegerAttribute("size", "Size", "Size of sequence", m_sequenceSize);
}

void ImagesToSequence::loadAttributes() {
    setSequenceSize(getIntegerAttribute("size"));
}

void ImagesToSequence::execute() {
    auto newImage = getInputData<Image>(0);
    if(m_queue.size() > 0)
        m_queue.pop_front();
    while(m_queue.size() < m_sequenceSize)
        m_queue.push_back(newImage);

    auto sequence = Sequence::New();
    std::vector<Image::pointer> vector{m_queue.begin(), m_queue.end()};
    sequence->create(vector);
    addOutputData(0, sequence);
}

void ImagesToSequence::setSequenceSize(int size) {
    if(size <= 0)
        throw Exception("Sequence size must be larger than 0");
    m_sequenceSize = size;
}

}
