%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%shared_ptr(fast::StepEdgeModel)


namespace fast {

class StepEdgeModel : public AppearanceModel {
	public:
		static SharedPointer<StepEdgeModel> New();
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
		enum EdgeType {
			EDGE_TYPE_ANY,
			EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE,
			EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE
		};
		void setEdgeType(EdgeType type);
	private:
		StepEdgeModel();
};


%template(StepEdgeModelPtr) SharedPointer<StepEdgeModel>;

}