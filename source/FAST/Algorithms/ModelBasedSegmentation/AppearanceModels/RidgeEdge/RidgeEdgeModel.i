%include "FAST/Algorithms/ModelBasedSegmentation/KalmanFilter.i"
%shared_ptr(fast::RidgeEdgeModel)


namespace fast {

class RidgeEdgeModel : public AppearanceModel {
	public:
		static SharedPointer<RidgeEdgeModel> New();
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
    	void setRidgeSizes(std::vector<float> sizes);
	private:
		RidgeEdgeModel();
};


%template(RidgeEdgeModelPtr) SharedPointer<RidgeEdgeModel>;

}