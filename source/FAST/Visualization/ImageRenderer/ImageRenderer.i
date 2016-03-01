%import "FAST/ProcessObject.i"

namespace fast {


class ImageRenderer : public Renderer {
    public:
    	static SharedPointer<ImageRenderer> New();
    	ImageRenderer();
        void addInputConnection(ProcessObjectPort port);
	private:
		void execute(); // Must have this, or class will be defined as abstract 
};

// This is needed for some strange reason
typedef SharedPointer<ImageRenderer> ImageRendererPtr;
%template(ImageRendererPtr) SharedPointer<ImageRenderer>;



} // end namespace fast