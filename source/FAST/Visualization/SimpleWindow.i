
%import "FAST/ProcessObject.i"
%import "FAST/Visualization/ImageRenderer/ImageRenderer.i"

namespace fast {

class SimpleWindow : public Window {
    public:
    	static SharedPointer<SimpleWindow> New();
    	SimpleWindow();
        void addRenderer(SharedPointer<Renderer> renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        void setWindowSize(unsigned int w, unsigned int h);
        void set2DMode();
        void set3DMode();
        void start();
};


// This is needed for some strange reason
typedef SharedPointer<SimpleWindow> SimpleWindowPtr;
%template(SimpleWindowPtr) SharedPointer<SimpleWindow>;



}