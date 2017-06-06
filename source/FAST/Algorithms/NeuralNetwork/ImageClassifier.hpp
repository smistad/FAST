#ifndef IMAGE_CLASSIFIER_HPP
#define IMAGE_CLASSIFIER_HPP

#include "FAST/ProcessObject.hpp"
#include "NeuralNetwork.hpp"
#include "FAST/Data/SimpleDataObject.hpp"
#include "FAST/Visualization/TextRenderer/TextRenderer.hpp"

namespace fast {

// Create the data object used as output from the ImageClassifier

typedef std::map<std::string, float> classifications;
FAST_SIMPLE_DATA_OBJECT(ImageClassification, classifications)


class FAST_EXPORT  ImageClassifier : public NeuralNetwork {
	FAST_OBJECT(ImageClassifier)
	public:
		void setLabels(std::vector<std::string> labels);
        void setOutputName(std::string outputName);
        void loadAttributes();
	private:
		ImageClassifier();
		void execute();

		// A map of label -> score
		std::vector<std::string> mLabels;
        std::string mOutputName;

};


/*
 * ProcessObject to convert a classification into text
 */
class FAST_EXPORT  ClassificationToText : public ProcessObject {
    FAST_OBJECT(ClassificationToText)
private:
    ClassificationToText() {
        createInputPort<ImageClassification>(0);
        createOutputPort<Text>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    }
    void execute() {
        ImageClassification::pointer classification = getStaticInputData<ImageClassification>();
        Text::pointer text = getStaticOutputData<Text>();

        // Find classification with max
        ImageClassification::access access = classification->getAccess(ACCESS_READ);
        std::map<std::string, float> values = access->getData();
        float max = 0;
        std::string label;
        for (auto &&item : values) {
            if(item.second > max) {
                max = item.second;
                label = item.first;
            }
        }

        Text::access access2 = text->getAccess(ACCESS_READ_WRITE);
        char buffer[8];
        std::sprintf(buffer, "%.2f", max);
        std::string result = label + ": " + buffer;
        access2->setData(result);
    }
};

}

#endif
