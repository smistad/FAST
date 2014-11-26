#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "Mesh.hpp"
#include "Color.hpp"
#include "Renderer.hpp"

namespace fast {

class MeshRenderer : public Renderer {
    FAST_OBJECT(MeshRenderer)
    public:
        void setInput(MeshData::pointer mesh);
        void addInput(MeshData::pointer mesh);
        void addInput(MeshData::pointer mesh, Color color, float opacity);
        BoundingBox getBoundingBox();
        void setDefaultOpacity(float opacity);
        void setDefaultColor(Color color);
        void setColor(MeshData::pointer mesh, Color color);
        void setOpacity(MeshData::pointer mesh, float opacity);
    private:
        MeshRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        boost::unordered_map<MeshData::pointer, Color> mInputColors;
        boost::unordered_map<MeshData::pointer, float> mInputOpacities;
        Color mDefaultColor;
        float mDefaultOpacity;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
