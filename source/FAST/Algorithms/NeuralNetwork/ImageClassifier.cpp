#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageClassifier::ImageClassifier() {
	createOutputPort<ImageClassification>(0, OUTPUT_DEPENDS_ON_INPUT, 0, true);
	createStringAttribute("labels", "Labels", "Name of each class", "");
	mOutputName = "";
}

void ImageClassifier::setLabels(std::vector<std::string> labels) {
	mLabels = labels;
}
void ImageClassifier::setOutputName(std::string outputName)	 {
	mOutputName = outputName;
}

void ImageClassifier::execute() {
	NeuralNetwork::execute();

	// Create output data
    tensorflow::Tensor result;
	if(mOutputName == "") {
		result = getNetworkOutput();
	} else {
		result = getNetworkOutput(mOutputName);
	}
	Eigen::Tensor<float, 2, 1> tensor = result.tensor<float, 2>();
	ImageClassification::pointer output = getStaticOutputData<ImageClassification>(0);
	for(int i = 0; i < tensor.dimension(0); ++i) { // for each input image
		std::map<std::string, float> mapResult;
		for(int j = 0; j < tensor.dimension(1); ++j) { // for each class
			mapResult[mLabels[j]] = tensor(i, j);
			reportInfo() << mLabels[j] << ": " << tensor(i, j) << reportEnd();
		}

		output->create(mapResult);
	}

}

void ImageClassifier::loadAttributes() {
	NeuralNetwork::loadAttributes();
	setLabels(getStringListAttribute("labels"));
}

}
