#ifndef LINE_RENDERER_HPP_
#define LINE_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

class FAST_EXPORT  LineRenderer : public Renderer {
    FAST_OBJECT(LineRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        void addInputConnection(ProcessObjectPort port, Color color, float width);
        void setDefaultColor(Color color);
        void setDefaultLineWidth(float width);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(ProcessObjectPort input, bool drawOnTop);
        void setColor(ProcessObjectPort input, Color color);
        void setWidth(ProcessObjectPort input, float width);
        void draw();
        BoundingBox getBoundingBox();
    private:
        LineRenderer();
        void execute();

        float mDefaultLineWidth;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        std::unordered_map<ProcessObjectPort, float> mInputWidths;
        std::unordered_map<ProcessObjectPort, Color> mInputColors;
        std::unordered_map<ProcessObjectPort, bool> mInputDrawOnTop;
        std::unordered_map<uint, Mesh::pointer> mMeshsToRender;
        std::mutex mMutex;
};

}

#endif
