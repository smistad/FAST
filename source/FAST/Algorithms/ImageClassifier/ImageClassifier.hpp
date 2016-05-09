#ifndef IMAGE_CLASSIFIER_HPP
#define IMAGE_CLASSIFIER_HPP

#include "FAST/ProcessObject.hpp"
#include <caffe/caffe.hpp>

namespace fast {

class ImageClassifier : public ProcessObject {
	FAST_OBJECT(ImageClassifier)
	public:
		void loadModel(
				std::string modelFile,
				std::string trainingFile,
				std::string meanFile
		);
		void setLabels(std::vector<std::string> labels);
		std::vector<std::map<std::string, float> > getResult() const;
	private:
		ImageClassifier();
		void execute();

		SharedPointer<caffe::Net<float> > mNet;
		caffe::Blob<float> mMeanBlob;
		bool mModelLoaded;
		// A map of label -> score
		std::vector<std::map<std::string, float> > mResult;
		std::vector<std::string> mLabels;

};

}

#endif
