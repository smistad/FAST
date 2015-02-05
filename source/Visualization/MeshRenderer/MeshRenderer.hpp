#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "Mesh.hpp"
#include "Color.hpp"
#include "Renderer.hpp"

namespace fast {

class MeshRenderer : public Renderer {
    FAST_OBJECT(MeshRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        void addInputConnection(ProcessObjectPort port, Color color, float opacity);
        BoundingBox getBoundingBox();
        void setDefaultOpacity(float opacity);
        void setDefaultColor(Color color);
        void setColor(ProcessObjectPort port, Color color);
        void setOpacity(ProcessObjectPort port, float opacity);
    private:
        MeshRenderer();
        void execute();
        void draw();

        boost::unordered_map<ProcessObjectPort, Color> mInputColors;
        boost::unordered_map<ProcessObjectPort, float> mInputOpacities;
        boost::unordered_map<uint, Mesh::pointer> mMeshToRender;
        Color mDefaultColor;
        float mDefaultOpacity;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
