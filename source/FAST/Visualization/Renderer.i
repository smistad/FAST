%include "FAST/ProcessObject.i"
%shared_ptr(fast::Renderer)

namespace fast {

class Renderer : public ProcessObject {
	public:
		typedef SharedPointer<Renderer> pointer;
	protected:
		virtual void execute() = 0;
};


%template(RendererPtr) SharedPointer<Renderer>;
}
