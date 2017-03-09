%include "FAST/ProcessObject.i"
%shared_ptr(fast::ImageResampler)

namespace fast {

class ImageResampler : public ProcessObject {
    public:
    	static SharedPointer<ImageResampler> New();
        void setOutputSpacing(float spacingX, float spacingY);
        void setOutputSpacing(float spacingX, float spacingY, float spacingZ);
	private:
		ImageResampler();
};

// This must come after class declaration
%template(ImageResamplerPtr) SharedPointer<ImageResampler>;
}
