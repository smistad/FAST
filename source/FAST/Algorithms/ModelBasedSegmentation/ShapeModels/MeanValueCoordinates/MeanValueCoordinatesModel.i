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
};

%template(MeanValueCoordinatesModelPtr) SharedPointer<MeanValueCoordinatesModel>;

}