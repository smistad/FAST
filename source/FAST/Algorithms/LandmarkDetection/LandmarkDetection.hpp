#ifndef LANDMARK_DETECTION_HPP_
#define LANDMARK_DETECTION_HPP_

#include "FAST/ProcessObject.hpp"
#include <caffe/caffe.hpp>

namespace fast {

class LandmarkDetection : public ProcessObject {
	FAST_OBJECT(LandmarkDetection)
	public:
		void loadModel(
				std::string modelFile,
				std::string weightsFile
		);
		void setMirrorImage(bool mirrorImage);
	private:
		LandmarkDetection();
		void execute();

		SharedPointer<caffe::Net<float> > mNet;
		caffe::Blob<float> mMeanBlob;
		bool mModelLoaded;
		bool mMirrorImage;

};

}

#endif
