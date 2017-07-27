#ifndef POINT_RENDERER_HPP_
#define POINT_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Color.hpp"
#include <mutex>

namespace fast {

class FAST_EXPORT  VertexRenderer : public Renderer {
    FAST_OBJECT(VertexRenderer)
    public:
        void addInputConnection(DataPort::pointer port);
        void addInputConnection(DataPort::pointer port, Color color, float size);
        void addInputData(Mesh::pointer data);
        void addInputData(Mesh::pointer data, Color color, float size);
        void setDefaultColor(Color color);
        void setDefaultSize(float size);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(DataPort::pointer input, bool drawOnTop);
        void setColor(DataPort::pointer input, Color color);
        void setSize(DataPort::pointer input, float size);
        void draw();
		void draw2D(
                cl::BufferGL PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );
    private:
        VertexRenderer();

        float mDefaultPointSize;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        bool mDefaultColorSet;
        std::unordered_map<DataPort::pointer, float> mInputSizes;
        std::unordered_map<DataPort::pointer, Color> mInputColors;
        std::unordered_map<DataPort::pointer, bool> mInputDrawOnTop;
        std::mutex mMutex;
};

}

#endif
