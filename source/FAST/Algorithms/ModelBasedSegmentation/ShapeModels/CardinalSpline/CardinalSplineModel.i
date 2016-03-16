%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%shared_ptr(fast::CardinalSplineModel)


namespace fast {

class CardinalSplineModel : public ShapeModel {
	FAST_OBJECT(CardinalSplineModel)
	public:
		void initializeShapeToImageCenter();
};

%template(CardinalSplineModelPtr) SharedPointer<CardinalSplineModel>;

}
