#ifndef BOUNDING_BOX_RENDERER_HPP_
#define BOUNDING_BOX_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include <boost/thread/mutex.hpp>
#include <unordered_map>

namespace fast {

class BoundingBoxRenderer : public Renderer {
    FAST_OBJECT(BoundingBoxRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
    private:
        BoundingBoxRenderer();
        void execute();
        void draw();
        BoundingBox getBoundingBox();

        std::unordered_map<uint, BoundingBox> mBoxesToRender;
        boost::mutex mMutex;
};

}

#endif
