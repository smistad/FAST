%include "FAST/ProcessObject.i"
%shared_ptr(fast::KalmanFilter)
%shared_ptr(fast::AppearanceModel)
%shared_ptr(fast::ShapeModel)

namespace fast {

%ignore AppearanceModel;
class AppearanceModel : public Object {
};
%template(AppearanceModelPtr) SharedPointer<AppearanceModel>;

%ignore ShapeModel;
class ShapeModel : public Object {
};
%template(ShapeModelPtr) SharedPointer<ShapeModel>;

class KalmanFilter : public ProcessObject {
	public:
		static SharedPointer<KalmanFilter> New();
		void setShapeModel(SharedPointer<ShapeModel> shapeModel);
		void setAppearanceModel(SharedPointer<AppearanceModel> appearanceModel);
};

%template(KalmanFilterPtr) SharedPointer<KalmanFilter>;

}