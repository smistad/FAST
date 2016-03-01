namespace fast {

template <class T>
class SharedPointer {
	public:        
		SharedPointer(Object * object);
        template <class U>
        SharedPointer(SharedPointer<U> object);
        template <class U>
        SharedPointer<T> &operator=(const SharedPointer<U> &other);
        T* operator->();     
};

class ProcessObjectPort {
};

class Object {
};

class ProcessObject : public Object {
    public:
    	ProcessObject();
    	void update();
        ProcessObjectPort getOutputPort();
    protected:
    	virtual void execute() = 0;
};


class Renderer : public ProcessObject {
	public:
		typedef SharedPointer<Renderer> pointer;
	protected:
		virtual void execute() = 0;
};

typedef SharedPointer<Renderer> RendererPtr;
%template(RendererPtr) SharedPointer<Renderer>;





} // end namespace fast