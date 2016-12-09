#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageClassifier::ImageClassifier() {
	createOutputPort<ImageClassification>(0, OUTPUT_DEPENDS_ON_INPUT, 0, true);

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
	reportInfo() << "RESULT: " << reportEnd();
	std::vector<std::vector<float> > result;
	if(mOutputName == "") {
		result = getNetworkOutput();
	} else {
		result = getNetworkOutput(mOutputName);
	}
	for(int i = 0; i < result.size(); ++i) { // for each input image
		std::map<std::string, float> mapResult;
		for(int j = 0; j < result[i].size(); ++j) { // for each class
			mapResult[mLabels[j]] = result[i][j];
			reportInfo() << mLabels[j] << ": " << result[i][j] << reportEnd();
		}

		ImageClassification::pointer result = addStaticOutputData<ImageClassification>(0);
		result->create(mapResult);
	}

}

}
