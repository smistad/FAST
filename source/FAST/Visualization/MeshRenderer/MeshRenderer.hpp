#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Visualization/Renderer.hpp"

namespace fast {

class FAST_EXPORT  MeshRenderer : public Renderer {
    FAST_OBJECT(MeshRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        void addInputConnection(ProcessObjectPort port, Color color, float opacity);
        BoundingBox getBoundingBox();
        void setDefaultOpacity(float opacity);
        /**
         * Enable/disable renderer of wireframe instead of filled polygons
         * @param wireframe
         */
        void setWireframe(bool wireframe);
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
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );
        MeshRenderer();
        void execute();

        std::unordered_map<ProcessObjectPort, Color> mInputColors;
        std::unordered_map<int, Color> mLabelColors;
        std::unordered_map<ProcessObjectPort, float> mInputOpacities;
        std::unordered_map<uint, Mesh::pointer> mMeshToRender;
        Color mDefaultColor;
        float mDefaultSpecularReflection;
        float mDefaultOpacity;
        std::mutex mMutex;
        int mLineSize;
        bool mWireframe;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
