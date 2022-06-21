#pragma once

#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Visualization/Renderer.hpp"

namespace fast {

/**
 * @brief Renders triangle Mesh data
 *
 * @ingroup renderers
 */
class FAST_EXPORT TriangleRenderer : public Renderer {
    FAST_PROCESS_OBJECT(TriangleRenderer)
    public:
        /**
         * @brief Create instance
         * @param color Color of triangles
         * @param opacity Triangle opacity
         * @param wireframe Display wireframe or not
         * @param specularReflection
         * @return instance
         */
        FAST_CONSTRUCTOR(TriangleRenderer,
                         Color, color, = Color::Green(),
                         float, opacity, = 1,
                         bool, wireframe, = false,
                         float, specularReflection, = 0.8f
        );
        uint addInputConnection(DataChannel::pointer port) override;
        uint addInputConnection(DataChannel::pointer port, Color color, float opacity);
        void setDefaultOpacity(float opacity);
        /**
         * Enable/disable renderer of wireframe instead of filled polygons
         * @param wireframe
         */
        void setWireframe(bool wireframe);
        void setDefaultColor(Color color);
        void setDefaultSpecularReflection(float specularReflection);
        void setColor(uint inputNr, Color color);
        void setLabelColor(int label, Color color);
        void setOpacity(uint inputNr, float opacity);
        void setLineSize(int size);
        /**
         * Ignore inverted normals. This gives a better visualization
         * if some normals in a mesh points have opposite direction.
         *
         * Default: true
         * @param ignore
         */
        void setIgnoreInvertedNormals(bool ignore);
    private:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;

        std::unordered_map<uint, Color> mInputColors;
        std::unordered_map<int, Color> mLabelColors;
        std::unordered_map<uint, float> mInputOpacities;
        std::unordered_map<uint, uint> mVAO;
        Color mDefaultColor;
        float mDefaultSpecularReflection;
        float mDefaultOpacity;
        int mLineSize;
        bool mWireframe;
        bool mDefaultColorSet;
        bool mIgnoreInvertedNormals = true;
};

} // namespace fast
