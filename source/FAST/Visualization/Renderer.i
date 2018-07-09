%include "FAST/ProcessObject.i"
%shared_ptr(fast::Renderer)

namespace fast {

class Renderer : public ProcessObject {
	public:
		typedef std::shared_ptr<Renderer> pointer;
	protected:
		virtual void execute() = 0;
};


%template(RendererPtr) std::shared_ptr<Renderer>;
}
