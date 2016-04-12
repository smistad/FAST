%include "FAST/Visualization/Renderer.i"
%shared_ptr(fast::MeshRenderer)

namespace fast {

class MeshRenderer : public Renderer {
    public:
    	static SharedPointer<MeshRenderer> New();
        void addInputConnection(ProcessObjectPort port);
        #void addInputConnection(ProcessObjectPort port, Color color, float opacity);
        void setDefaultOpacity(float opacity);
        #void setDefaultColor(Color color);
        void setDefaultSpecularReflection(float specularReflection);
        #void setColor(ProcessObjectPort port, Color color);
        #void setOpacity(ProcessObjectPort port, float opacity);
};

%template(MeshRendererPtr) SharedPointer<MeshRenderer>;

}