#ifndef IMAGE_CLASSIFIER_HPP
#define IMAGE_CLASSIFIER_HPP

#include "FAST/ProcessObject.hpp"
#include "NeuralNetwork.hpp"
#include "FAST/Data/SimpleDataObject.hpp"
#include "FAST/Visualization/TextRenderer/TextRenderer.hpp"
#include <queue>

namespace fast {

// Create the data object used as output from the ImageClassifier

typedef std::map<std::string, float> classifications;
FAST_SIMPLE_DATA_OBJECT(ImageClassification, classifications)

/**
 * Neural network image classification
 */
class FAST_EXPORT  ImageClassifier : public NeuralNetwork {
	FAST_OBJECT(ImageClassifier)
	public:
		void setLabels(std::vector<std::string> labels);
        void loadAttributes();
	private:
		ImageClassifier();
		void execute();

		// A map of label -> score
		std::vector<std::string> mLabels;

};


/**
 * ProcessObject to convert a classification into text
 */
class FAST_EXPORT  ClassificationToText : public ProcessObject {
    FAST_OBJECT(ClassificationToText)
    private:
        std::deque<std::map<std::string, float>> mBuffer; // used for calculating temporal average
        int mBufferSize = 1; // How large the buffer can be

        ClassificationToText();
        void loadAttributes();
        void execute();
};

}

#endif
