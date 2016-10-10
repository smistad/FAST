#ifndef IMAGE_CLASSIFIER_HPP
#define IMAGE_CLASSIFIER_HPP

#include "FAST/ProcessObject.hpp"
#include "NeuralNetwork.hpp"
#include "FAST/Data/SimpleDataObject.hpp"

namespace fast {

// Create the data object used as output from the ImageClassifier
FAST_SIMPLE_DATA_OBJECT(ImageClassification, (std::map<std::string, float>))


class ImageClassifier : public NeuralNetwork {
	FAST_OBJECT(ImageClassifier)
	public:
		void setLabels(std::vector<std::string> labels);
        void setOutputName(std::string outputName);
	private:
		ImageClassifier();
		void execute();

		// A map of label -> score
		std::vector<std::string> mLabels;
        std::string mOutputName;

};

}

#endif
