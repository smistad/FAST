#ifndef POINT_RENDERER_HPP_
#define POINT_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Data/LineSet.hpp"

namespace fast {

class LineRenderer : public Renderer {
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
        boost::unordered_map<ProcessObjectPort, float> mInputWidths;
        boost::unordered_map<ProcessObjectPort, Color> mInputColors;
        boost::unordered_map<ProcessObjectPort, bool> mInputDrawOnTop;
        boost::unordered_map<uint, LineSet::pointer> mLineSetsToRender;
        boost::mutex mMutex;
};

}

#endif
