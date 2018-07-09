%include "FAST/Visualization/Renderer.i"

%shared_ptr(fast::ImageRenderer)

namespace fast {

class ImageRenderer : public Renderer {
    public:
    	static std::shared_ptr<ImageRenderer> New();
        void addInputConnection(ProcessObjectPort port);
        void setInputData(std::shared_ptr<DataObject> data);
	private:
    	ImageRenderer();
};

%template(ImageRendererPtr) std::shared_ptr<ImageRenderer>;

} // end namespace fast