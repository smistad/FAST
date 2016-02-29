namespace fast {

class ProcessObjectPort {
};

class ProcessObject : public Object {
    public:
    	ProcessObject();
    	void update();
        ProcessObjectPort getOutputPort();
    protected:
    	virtual void execute() = 0;
};


class Renderer : public ProcessObjectPort {
	protected:
		virtual void execute() = 0;
};

} // end namespace fast