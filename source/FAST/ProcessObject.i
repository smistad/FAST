%include "FAST/SmartPointers.i"

%shared_ptr(fast::Object)
%shared_ptr(fast::ProcessObject)

typedef unsigned int uint;
namespace fast {

#define FAST_OBJECT(name) public:\
static SharedPointer<name> New();\
private:\
name();\

class ProcessObjectPort {
};

class Object {
};

class ProcessObject : public Object {
    public:
    	void update();
        ProcessObjectPort getOutputPort();
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
    protected:
    	ProcessObject();
    	virtual void execute() = 0;
};

} // end namespace fast