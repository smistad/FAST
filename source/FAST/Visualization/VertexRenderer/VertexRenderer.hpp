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
        void addInputConnection(ProcessObjectPort port);
        void addInputConnection(ProcessObjectPort port, Color color, float size);
        void addInputData(Mesh::pointer data);
        void addInputData(Mesh::pointer data, Color color, float size);
        void setDefaultColor(Color color);
        void setDefaultSize(float size);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(ProcessObjectPort input, bool drawOnTop);
        void setColor(ProcessObjectPort input, Color color);
        void setSize(ProcessObjectPort input, float size);
        void draw();
		void draw2D(
                cl::BufferGL PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );
        BoundingBox getBoundingBox();
    private:
        VertexRenderer();
        void execute();

        float mDefaultPointSize;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        bool mDefaultColorSet;
        std::unordered_map<ProcessObjectPort, float> mInputSizes;
        std::unordered_map<ProcessObjectPort, Color> mInputColors;
        std::unordered_map<ProcessObjectPort, bool> mInputDrawOnTop;
        std::unordered_map<uint, Mesh::pointer> mPointSetsToRender;
        std::mutex mMutex;
};

}

#endif
