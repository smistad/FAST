%include "FAST/ProcessObject.i"
%shared_ptr(fast::KalmanFilter)
%shared_ptr(fast::AppearanceModel)
%shared_ptr(fast::ShapeModel)

namespace fast {

%nodefaultctor AppearanceModel;
class AppearanceModel : public Object {
};
%template(AppearanceModelPtr) std::shared_ptr<AppearanceModel>;

%nodefaultctor ShapeModel;
class ShapeModel : public Object {
};
%template(ShapeModelPtr) std::shared_ptr<ShapeModel>;

class KalmanFilter : public ProcessObject {
	public:
		static std::shared_ptr<KalmanFilter> New();
		void setShapeModel(std::shared_ptr<ShapeModel> shapeModel);
		void setAppearanceModel(std::shared_ptr<AppearanceModel> appearanceModel);
		void setIterations(int iterations);
		void setStartIterations(int iterations);
		ProcessObjectPort getDisplacementsOutputPort();
};

%template(KalmanFilterPtr) std::shared_ptr<KalmanFilter>;

}
