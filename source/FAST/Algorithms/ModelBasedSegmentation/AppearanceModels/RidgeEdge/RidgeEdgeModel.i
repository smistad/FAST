%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%shared_ptr(fast::RidgeEdgeModel)

namespace fast {

class RidgeEdgeModel : public AppearanceModel {
	public:
		static std::shared_ptr<RidgeEdgeModel> New();
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
        void setMinimumRidgeSize(float sizeInMM);
        enum EdgeType {
			EDGE_TYPE_ANY,
			EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE,
			EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE
		};
		void setEdgeType(EdgeType type);
	private:
		RidgeEdgeModel();
};


%template(RidgeEdgeModelPtr) std::shared_ptr<RidgeEdgeModel>;

}