#ifndef BOUNDING_BOX_RENDERER_HPP_
#define BOUNDING_BOX_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

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

        boost::unordered_map<uint, BoundingBox> mBoxesToRender;
        boost::mutex mMutex;
};

}

#endif
