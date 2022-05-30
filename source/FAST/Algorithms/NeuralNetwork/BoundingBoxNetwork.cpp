#include "BoundingBoxNetwork.hpp"
#include <FAST/Data/BoundingBox.hpp>
#include "TensorToBoundingBoxSet.hpp"

namespace fast {

void BoundingBoxNetwork::loadAttributes() {
	setThreshold(getFloatAttribute("threshold"));

    std::vector<std::vector<Vector2f>> anchors;
    const auto level = split(getStringAttribute("anchors"), ";");
    for(auto&& part : level) {
        const auto parts = split(part, ",");
        std::vector<Vector2f> levelAnchors;
        for(int i = 0; i < parts.size(); i += 2) {
            levelAnchors.push_back(Vector2f(std::stof(parts[i]), std::stof(parts[i+1])));
        }
        anchors.push_back(levelAnchors);
    }
    setAnchors(anchors);

	NeuralNetwork::loadAttributes();
}

void BoundingBoxNetwork::setAnchors(std::vector<std::vector<Vector2f>> anchors) {
    m_tensorToBoundingBoxSet->setAnchors(anchors);
    setModified(true);
}

BoundingBoxNetwork::BoundingBoxNetwork() {
    createInputPort<Image>(0);
    createOutputPort<BoundingBoxSet>(0);

    m_tensorToBoundingBoxSet = TensorToBoundingBoxSet::New();

    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", 0.5f);
    createStringAttribute("anchors", "Anchors", "Should be formatted like: x1,y1,x2,y2;x1,y1,x2,y2", "");
}

BoundingBoxNetwork::BoundingBoxNetwork(std::string modelFilename, float scaleFactor, float threshold,
                                       std::vector<std::vector<Vector2f>> anchors, BoundingBoxNetworkType type,
                                       float meanIntensity, float stanardDeviationIntensity,
                                       std::vector<NeuralNetworkNode> inputNodes,
                                       std::vector<NeuralNetworkNode> outputNodes, std::string inferenceEngine,
                                       std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, scaleFactor, meanIntensity, stanardDeviationIntensity, inputNodes, outputNodes, inferenceEngine, customPlugins) {
    createInputPort<Image>(0);
    createOutputPort<BoundingBoxSet>(0);
    m_tensorToBoundingBoxSet = TensorToBoundingBoxSet::New();

    setThreshold(threshold);
    setAnchors(anchors);
}

void BoundingBoxNetwork::execute() {
    runNeuralNetwork();

    mRuntimeManager->startRegularTimer("output_processing");
    m_tensorToBoundingBoxSet->setNrOfInputNodes(m_processedOutputData.size());
    for(auto node : m_processedOutputData) {
        auto tensor = std::dynamic_pointer_cast<Tensor>(node.second);
        m_tensorToBoundingBoxSet->setInputData(node.first, tensor);
    }
    addOutputData(m_tensorToBoundingBoxSet->updateAndGetOutputData<BoundingBoxSet>());
    mRuntimeManager->stopRegularTimer("output_processing");
}


void BoundingBoxNetwork::setThreshold(float threshold) {
    m_tensorToBoundingBoxSet->setThreshold(threshold);
    setModified(true);
}

void BoundingBoxNetwork::setType(BoundingBoxNetworkType type) {
    m_tensorToBoundingBoxSet->setType(type);
    setModified(true);
}

}