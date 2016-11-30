#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Visualization/Renderer.hpp"

namespace fast {

class MeshRenderer : public Renderer {
    FAST_OBJECT(MeshRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        void addInputConnection(ProcessObjectPort port, Color color, float opacity);
        BoundingBox getBoundingBox();
        void setDefaultOpacity(float opacity);
        void setDefaultColor(Color color);
        void setDefaultSpecularReflection(float specularReflection);
        void setColor(ProcessObjectPort port, Color color);
        void setColor(int label, Color color);
        void setOpacity(ProcessObjectPort port, float opacity);
        void setLineSize(int size);
    private:
        void draw();
        void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );
        MeshRenderer();
        void execute();

        boost::unordered_map<ProcessObjectPort, Color> mInputColors;
        boost::unordered_map<int, Color> mLabelColors;
        boost::unordered_map<ProcessObjectPort, float> mInputOpacities;
        boost::unordered_map<uint, Mesh::pointer> mMeshToRender;
        Color mDefaultColor;
        float mDefaultSpecularReflection;
        float mDefaultOpacity;
        boost::mutex mMutex;
        int mLineSize;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
