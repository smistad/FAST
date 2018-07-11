%{
#define SWIG_FILE_WITH_INIT
%}
%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%shared_ptr(fast::CardinalSplineModel)

%include "FAST/numpy.i"
%init %{
	import_array();
%}

%apply (float* IN_ARRAY2, int DIM1, int DIM2) {(float* data, int w, int h)};
%inline %{
#include <FAST/Data/DataTypes.hpp>
#include <vector>
std::vector<fast::Vector2f> point_list_to_vector(float* data, int w, int h) {
	std::vector<fast::Vector2f> result;
	for(int y = 0; y < h; ++y) {
		result.push_back(fast::Vector2f(data[y], data[y + h]));
	}
	return result;
}
%}

#include <FAST/Data/DataTypes.hpp>
namespace fast {

class CardinalSplineModel : public ShapeModel {
	FAST_OBJECT(CardinalSplineModel)
	public:
		void setControlPoints(std::vector<fast::Vector2f> controlPoints);
		void initializeShapeToImageCenter();
		void setLocalProcessError(float error);
		void setGlobalProcessError(float error);
		void setResolution(int resolution);
};

%template(CardinalSplineModelPtr) SharedPointer<CardinalSplineModel>;

}
