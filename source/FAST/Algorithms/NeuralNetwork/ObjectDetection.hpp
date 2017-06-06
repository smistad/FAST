#ifndef OBJECT_DETECTION_HPP_
#define OBJECT_DETECTION_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp"

namespace fast {

class FAST_EXPORT  ObjectDetection : public NeuralNetwork {
	FAST_OBJECT(ObjectDetection)
	public:
		void setMirrorImage(bool mirrorImage);
	private:
		ObjectDetection();
		void execute();
};

}

#endif
