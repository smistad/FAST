#pragma once

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Color.hpp"
#include <mutex>

namespace fast {

/**
 * @brief Renders vertices as a circular points
 *
 * @ingroup renderers
 * @sa Mesh
 */
class FAST_EXPORT  VertexRenderer : public Renderer {
    FAST_PROCESS_OBJECT(VertexRenderer)
    public:
        /**
         * @brief Create instance
         * @param size Vertex point size (can be in pixels or millimeters, see sizeIsInPixels parameter)
         * @param sizeIsInPixels Whether size is given in pixels or millimeters
         * @param minSize Minimum size in pixels, used when sizeInPixels = false
         * @param color Override color stored for each vertex
         * @param drawOnTop Disable depth testing and always draw vertices on top
         * @return instance
         */
        FAST_CONSTRUCTOR(VertexRenderer,
                         float, size, = 10.0f,
                         bool, sizeIsInPixels, = true,
                         int, minSize, = 1,
                         Color, color, = Color::Null(),
                         bool, drawOnTop, = false
        );
        uint addInputConnection(DataChannel::pointer port) override;
        uint addInputConnection(DataChannel::pointer port, Color color, float size);
        uint addInputData(DataObject::pointer data) override;
        uint addInputData(Mesh::pointer data, Color color, float size);
        void setDefaultColor(Color color);
        void setDefaultSize(float size);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(uint inputNr, bool drawOnTop);
        void setColor(uint inputNr, Color color);
        void setSize(uint inputNr, float size);
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight);
    private:
        float mDefaultPointSize;
        bool m_sizeIsInPixels = true;
        int m_minSize;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        std::unordered_map<uint, float> mInputSizes;
        std::unordered_map<uint, Color> mInputColors;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;
};

}
