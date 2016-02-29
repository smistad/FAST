namespace fast {

class SimpleWindow : public Window {
    public:
    	SimpleWindow();
        void addRenderer(Renderer::pointer renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        void setWindowSize(unsigned int w, unsigned int h);
        void set2DMode();
        void set3DMode();
};

}