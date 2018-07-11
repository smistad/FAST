%include "FAST/ProcessObject.i"
%shared_ptr(fast::KalmanFilter)
%shared_ptr(fast::AppearanceModel)
%shared_ptr(fast::ShapeModel)

namespace fast {

%nodefaultctor AppearanceModel;
class AppearanceModel : public Object {
};
%template(AppearanceModelPtr) SharedPointer<AppearanceModel>;

%nodefaultctor ShapeModel;
class ShapeModel : public Object {
};
%template(ShapeModelPtr) SharedPointer<ShapeModel>;

class KalmanFilter : public ProcessObject {
	public:
		static SharedPointer<KalmanFilter> New();
		void setShapeModel(SharedPointer<ShapeModel> shapeModel);
		void setAppearanceModel(SharedPointer<AppearanceModel> appearanceModel);
		void setIterations(int iterations);
		void setStartIterations(int iterations);
		ProcessObjectPort getDisplacementsOutputPort();
};

%template(KalmanFilterPtr) SharedPointer<KalmanFilter>;

}
