#include "FlowNetwork.hpp"
#include <FAST/Data/Image.hpp>
#include "TensorToImage.hpp"

namespace fast {
FlowNetwork::FlowNetwork() {
    createInputPort<Sequence>(0);
    createOutputPort<Image>(0);
}

FlowNetwork::FlowNetwork(std::string modelFilename, std::vector<NeuralNetworkNode> inputNodes,
                         std::vector<NeuralNetworkNode> outputNodes, std::string inferenceEngine,
                         std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, inputNodes, outputNodes, inferenceEngine, customPlugins) {
    createInputPort<Sequence>(0);
    createOutputPort<Image>(0);
}

FlowNetwork::FlowNetwork(std::string modelFilename, float scaleFactor, float meanIntensity,
                         float stanardDeviationIntensity, std::vector<NeuralNetworkNode> inputNodes,
                         std::vector<NeuralNetworkNode> outputNodes, std::string inferenceEngine,
                         std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, scaleFactor, meanIntensity, stanardDeviationIntensity, inputNodes, outputNodes, inferenceEngine, customPlugins) {
    createInputPort<Sequence>(0);
    createOutputPort<Image>(0);
}

void FlowNetwork::execute() {
    // Prepare input node: Should be two images in a sequence
    runNeuralNetwork();

    auto tensor = std::dynamic_pointer_cast<Tensor>(m_processedOutputData[0]);
    if(!tensor)
        throw Exception("Unable to cast processed output data to Tensor");

    auto tensorToImage = TensorToImage::New();
    tensorToImage->setInputData(tensor);
    addOutputData(0, tensorToImage->updateAndGetOutputData<Image>());
}

ImagesToSequence::ImagesToSequence(int sequenceSize) {
    createInputPort<Image>(0);
    createOutputPort<Sequence>(0);

    setSequenceSize(sequenceSize);
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

    std::vector<Image::pointer> vector{m_queue.begin(), m_queue.end()};
    auto sequence = Sequence::create(vector);
    addOutputData(0, sequence);
}

void ImagesToSequence::setSequenceSize(int size) {
    if(size <= 0)
        throw Exception("Sequence size must be larger than 0");
    m_sequenceSize = size;
}

}
