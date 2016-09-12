#ifndef OBJECT_DETECTION_HPP_
#define OBJECT_DETECTION_HPP_

#include "FAST/ProcessObject.hpp"
#include <caffe/caffe.hpp>

namespace fast {

class ObjectDetection : public ProcessObject {
	FAST_OBJECT(ObjectDetection)
	public:
		void loadModel(
				std::string modelFile,
				std::string weightsFile
		);
		void setMirrorImage(bool mirrorImage);
	private:
		ObjectDetection();
		void execute();

		SharedPointer<caffe::Net<float> > mNet;
		bool mModelLoaded;

};

}

#endif
