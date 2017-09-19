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

/**
 * Neural network image classification
 */
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


/**
 * ProcessObject to convert a classification into text
 */
class FAST_EXPORT  ClassificationToText : public ProcessObject {
    FAST_OBJECT(ClassificationToText)
    private:
        std::deque<std::map<std::string, float>> mBuffer; // used for calculating temporal average
        int mBufferSize = 100; // How large the buffer can be
        ClassificationToText() {
            createInputPort<ImageClassification>(0);
            createOutputPort<Text>(0);
            createIntegerAttribute("average_size", "Average size", "nr of frames to average", 100);
        }
        void loadAttributes(std::vector<Attribute> attributes) {
            mBufferSize = getIntegerAttribute("avarage_size");
        }
        void execute() {
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

            Text::access access2 = text->getAccess(ACCESS_READ_WRITE);
            char buffer[8];
            std::sprintf(buffer, "%.2f", max);
            std::string result = label + ": " + buffer;
            access2->setData(result);
        }
};

}

#endif
