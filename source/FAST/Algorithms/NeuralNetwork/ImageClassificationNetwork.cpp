#include "ImageClassificationNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include <FAST/Data/Text.hpp>

namespace fast {

ImageClassificationNetwork::ImageClassificationNetwork() {
	createOutputPort<ImageClassification>(0);
	createStringAttribute("labels", "Labels", "Name of each class", "");
}

ImageClassificationNetwork::ImageClassificationNetwork(std::string modelFilename, std::vector<std::string> labels,
                                                       float scaleFactor, float meanIntensity,
                                                       float stanardDeviationIntensity,
                                                       int temporalWindow,
                                                       std::vector<NeuralNetworkNode> inputNodes,
                                                       std::vector<NeuralNetworkNode> outputNodes,
                                                       std::string inferenceEngine,
                                                       std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename,scaleFactor,meanIntensity,stanardDeviationIntensity,inputNodes,outputNodes,inferenceEngine,customPlugins) {
    createOutputPort<ImageClassification>(0);
    setLabels(labels);
    setTemporalWindow(temporalWindow);
}

ImageClassificationNetwork::ImageClassificationNetwork(std::string modelFilename,
                                                       std::vector<NeuralNetworkNode> inputNodes,
                                                       std::vector<NeuralNetworkNode> outputNodes,
                                                       std::string inferenceEngine,
                                                       std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, inputNodes, outputNodes, inferenceEngine, customPlugins) {
    createOutputPort<ImageClassification>(0);
}

void ImageClassificationNetwork::setLabels(std::vector<std::string> labels) {
	mLabels = labels;
    setModified(true);
}

void ImageClassificationNetwork::execute() {

    runNeuralNetwork();

    // TODO batch support
    auto tensor = std::dynamic_pointer_cast<Tensor>(m_processedOutputData[0]);
    if(!tensor)
        throw Exception("ImageClassificationNetwork batch support not implemented");
    auto access = tensor->getAccess(ACCESS_READ);

    auto data = access->getData<1>();
    if(mLabels.size() != data.dimension(0)) {
        throw Exception("Incorrect number of labels given to ImageClassificationNetwork");
    }
    std::map<std::string, float> mapResult;
    for(int j = 0; j < data.dimension(0); ++j) { // for each class
        mapResult[mLabels[j]] = data(j);
        reportInfo() << mLabels[j] << ": " << data(j) << reportEnd();
    }

    if(m_temporalWindow > 1) {
        // Average over temporal window
        m_results.push_back(mapResult);
        if(m_results.size() > m_temporalWindow) m_results.pop_front();
        std::map<std::string, float> outputMap;
        for(auto& label : mLabels)
            outputMap[label] = 0.0f;
        for(auto& map : m_results) {
            for(auto& item : map) {
                outputMap[item.first] += item.second;
            }
        }
        for(auto& label : mLabels)
            outputMap[label] /= m_results.size();
        auto output = ImageClassification::create(outputMap);
        addOutputData(0, output);
    } else {
        auto output = ImageClassification::create(mapResult);
        addOutputData(0, output);
    }
}

void ImageClassificationNetwork::loadAttributes() {
	NeuralNetwork::loadAttributes();
	setLabels(getStringListAttribute("labels"));
}

void ImageClassificationNetwork::setTemporalWindow(int window) {
    if(window < 1)
        throw Exception("Temporal window in ImageClassificationNetwork must be > 0");
    m_temporalWindow = window;
    setModified(true);
}

ClassificationToText::ClassificationToText(int bufferSize) {
    createInputPort<ImageClassification>(0);
    createOutputPort<Text>(0);
    createIntegerAttribute("average_size", "Average size", "nr of frames to average", 100);
    setBufferSize(bufferSize);
}

void ClassificationToText::loadAttributes() {
    mBufferSize = getIntegerAttribute("average_size");
}

void ClassificationToText::execute() {
    auto classification = getInputData<ImageClassification>();

    std::map<std::string, float> values = classification->get();

    // Add to buffer
    mBuffer.push_back(values);

    // Remove item from buffer
    if(mBuffer.size() > mBufferSize)
        mBuffer.pop_front();

    // Calculate average
    for(auto&& val : values)
       val.second = 0;

    // Sum
    for(auto sample : mBuffer) {
        for(auto val : sample) {
            values[val.first] += val.second;
        }
    }
    // Divide
    for(auto&& val : values) {
        val.second /= mBuffer.size();
    }

    // Find classification with max
    float max = 0;
    std::string label;
    for (auto &&item : values) {
        if(item.second > max) {
            max = item.second;
            label = item.first;
        }
    }

    char buffer[8];
    std::sprintf(buffer, "%.2f", max);
    std::string result = label + ": " + buffer;
    auto text = Text::create(result);
    addOutputData(0, text);
}

void ClassificationToText::setBufferSize(int bufferSize) {
    mBufferSize = bufferSize;
    setModified(true);
}

}
