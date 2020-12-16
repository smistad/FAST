#include "ImageClassificationNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include <FAST/Data/Text.hpp>

namespace fast {

ImageClassificationNetwork::ImageClassificationNetwork() {
	createOutputPort<ImageClassification>(0);
	createStringAttribute("labels", "Labels", "Name of each class", "");
}

void ImageClassificationNetwork::setLabels(std::vector<std::string> labels) {
	mLabels = labels;
}

void ImageClassificationNetwork::execute() {

    run();

    // TODO batch support
    auto tensor = std::dynamic_pointer_cast<Tensor>(m_processedOutputData[0]);
    if(!tensor)
        throw Exception("ImageClassificationNetwork batch support not implemented");
    auto access = tensor->getAccess(ACCESS_READ);
    std::cout << tensor->getShape().toString() << std::endl;

    auto data = access->getData<1>();
    auto output = getOutputData<ImageClassification>(0);
	std::map<std::string, float> mapResult;
	for(int j = 0; j < data.dimension(0); ++j) { // for each class
		mapResult[mLabels[j]] = data(j);
		reportInfo() << mLabels[j] << ": " << data(j) << reportEnd();
	}

	output->create(mapResult);
}

void ImageClassificationNetwork::loadAttributes() {
	NeuralNetwork::loadAttributes();
	setLabels(getStringListAttribute("labels"));
}

ClassificationToText::ClassificationToText() {
    createInputPort<ImageClassification>(0);
    createOutputPort<Text>(0);
    createIntegerAttribute("average_size", "Average size", "nr of frames to average", 100);
}

void ClassificationToText::loadAttributes() {
    mBufferSize = getIntegerAttribute("average_size");
}

void ClassificationToText::execute() {
    ImageClassification::pointer classification = getInputData<ImageClassification>();
    Text::pointer text = getOutputData<Text>();

    ImageClassification::access access = classification->getAccess(ACCESS_READ);
    std::map<std::string, float> values = access->getData();

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
    text->setText(result);
}

}
