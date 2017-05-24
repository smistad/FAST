%include "FAST/ProcessObject.i"
%shared_ptr(fast::GaussianSmoothingFilter);

namespace fast {

class GaussianSmoothingFilter : public ProcessObject {
    public:
    	static SharedPointer<GaussianSmoothingFilter> New();
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
	private:
		GaussianSmoothingFilter();
//        void setOutputType(DataType type);
};

// This must come after class declaration
%template(GaussianSmoothingFilterPtr) SharedPointer<GaussianSmoothingFilter>;
}