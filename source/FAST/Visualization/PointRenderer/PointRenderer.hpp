#ifndef POINT_RENDERER_HPP_
#define POINT_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Data/Color.hpp"
#include <mutex>

namespace fast {

class PointRenderer : public Renderer {
    FAST_OBJECT(PointRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        void addInputConnection(ProcessObjectPort port, Color color, float size);
        void addInputData(PointSet::pointer data);
        void addInputData(PointSet::pointer data, Color color, float size);
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
                Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );
        BoundingBox getBoundingBox();
    private:
        PointRenderer();
        void execute();

        float mDefaultPointSize;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        std::unordered_map<ProcessObjectPort, float> mInputSizes;
        std::unordered_map<ProcessObjectPort, Color> mInputColors;
        std::unordered_map<ProcessObjectPort, bool> mInputDrawOnTop;
        std::unordered_map<uint, PointSet::pointer> mPointSetsToRender;
        std::mutex mMutex;
};

}

#endif
