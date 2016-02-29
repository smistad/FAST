namespace fast {

class ImageRenderer : public Renderer {
    public:
    	ImageRenderer();
        void addInputConnection(ProcessObjectPort port);
	private:
		void execute(); // Must have this, or class will be defined as abstract 
};

} // end namespace fast