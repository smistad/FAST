#pragma once

#include "FAST/ProcessObject.hpp"
#include "NeuralNetwork.hpp"
#include "FAST/Data/SimpleDataObject.hpp"
#include <queue>

namespace fast {

// Create the data object used as output from the ImageClassificationNetwork

typedef std::map<std::string, float> classifications;

/**
 * @brief Image classification data object
 *
 * @ingroup neural-network
 */
FAST_SIMPLE_DATA_OBJECT(ImageClassification, classifications)

/**
 * @brief Image classification neural network
 *
 * This class is a convenience class for a neural network which performs image classification.
 * Use setLabels method to define the class names. The output is then ImageClassification
 * which is a map from class names to confidence values.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT ImageClassificationNetwork : public NeuralNetwork {
	FAST_PROCESS_OBJECT(ImageClassificationNetwork)
	public:
        FAST_CONSTRUCTOR(ImageClassificationNetwork,
                         std::string, modelFilename,,
                         std::vector<std::string>, labels,,
                         float, scaleFactor, = 1.0f,
                         float, meanIntensity, = 0.0f,
                         float, stanardDeviationIntensity, = 1.0f,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
         )
		void setLabels(std::vector<std::string> labels);
        void loadAttributes();
	private:
		ImageClassificationNetwork();
		void execute();

		// A map of label -> score
		std::vector<std::string> mLabels;

};


/**
 * ProcessObject to convert a classification into text
 */
class FAST_EXPORT  ClassificationToText : public ProcessObject {
    FAST_PROCESS_OBJECT(ClassificationToText)
    public:
        FAST_CONSTRUCTOR(ClassificationToText, int, bufferSize, = 1)
        void setBufferSize(int bufferSize);
    private:
        std::deque<std::map<std::string, float>> mBuffer; // used for calculating temporal average
        int mBufferSize = 1; // How large the buffer can be

        void loadAttributes();
        void execute();
};

}
