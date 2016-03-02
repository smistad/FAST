%include "FAST/SmartPointers.i"

%shared_ptr(fast::Object)
%shared_ptr(fast::ProcessObject)
%shared_ptr(fast::Renderer)

namespace fast {

class ProcessObjectPort {
};

class Object {
};

class ProcessObject : public Object {
    public:
    	ProcessObject();
    	void update();
        ProcessObjectPort getOutputPort();
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
    protected:
    	virtual void execute() = 0;
};


class Renderer : public ProcessObject {
	public:
		typedef SharedPointer<Renderer> pointer;
	protected:
		virtual void execute() = 0;
};

%template(RendererPtr) SharedPointer<Renderer>;


} // end namespace fast