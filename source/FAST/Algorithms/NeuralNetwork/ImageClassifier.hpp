#ifndef IMAGE_CLASSIFIER_HPP
#define IMAGE_CLASSIFIER_HPP

#include "FAST/ProcessObject.hpp"
#include "NeuralNetwork.hpp"
#include <caffe/caffe.hpp>

namespace fast {

class ImageClassifier : public NeuralNetwork {
	FAST_OBJECT(ImageClassifier)
	public:
		void setLabels(std::vector<std::string> labels);
		std::vector<std::map<std::string, float> > getClassification(std::string outputLayerName = "");
	private:
		ImageClassifier();
		void execute();

		// A map of label -> score
		std::vector<std::string> mLabels;

};

}

#endif
