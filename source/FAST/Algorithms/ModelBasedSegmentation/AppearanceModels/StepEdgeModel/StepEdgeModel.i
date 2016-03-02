%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%shared_ptr(fast::StepEdgeModel)


namespace fast {

class StepEdgeModel : public AppearanceModel {
	public:
		static SharedPointer<StepEdgeModel> New();
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
	private:
		StepEdgeModel();
};


%template(StepEdgeModelPtr) SharedPointer<StepEdgeModel>;

}