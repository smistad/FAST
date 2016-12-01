%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%include "std_string.i"
%shared_ptr(fast::MeanValueCoordinatesModel)

namespace fast {

%nodefaultctor MeanValueCoordinatesModel;
class MeanValueCoordinatesModel : public ShapeModel {
	public:
		static SharedPointer<MeanValueCoordinatesModel> New();
		void loadMeshes(std::string surfaceMeshFilename, std::string controlMeshFilename);
		void initializeShapeToImageCenter();
		void setInitialScaling(float x, float y, float z);
		void setInitialTranslation(float x, float y, float z);
		void setLocalProcessError(float error);
		void setGlobalProcessError(float error);
};

%template(MeanValueCoordinatesModelPtr) SharedPointer<MeanValueCoordinatesModel>;

}