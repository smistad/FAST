#ifndef LANDMARK_DETECTION_HPP_
#define LANDMARK_DETECTION_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp"

namespace fast {

class Mesh;

class FAST_EXPORT  DNNAppearanceModel : public NeuralNetwork {
	FAST_OBJECT(DNNAppearanceModel)
	public:
		void loadObjects(
				std::string objectsFile
		);
		void setMirrorImage(bool mirrorImage);
	private:
		DNNAppearanceModel();
		void execute();

		bool mMirrorImage;
		SharedPointer<Mesh> mOutputMesh;
        bool mObjectsLoaded;
};

}

#endif
