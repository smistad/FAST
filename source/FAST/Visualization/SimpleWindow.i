%include "FAST/ProcessObject.i"

%shared_ptr(fast::SimpleWindow)

namespace fast {

class SimpleWindow : public Window {
    public:
    	static std::shared_ptr<SimpleWindow> New();
    	SimpleWindow();
        void addRenderer(Renderer::pointer renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        void setSize(unsigned int w, unsigned int h);
        void set2DMode();
        void set3DMode();
        void start();
};

%template(SimpleWindowPtr) std::shared_ptr<SimpleWindow>;


}