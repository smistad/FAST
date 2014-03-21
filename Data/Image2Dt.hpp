#ifndef IMAGE2DT_HPP
#define IMAGE2DT_HPP

#include "PipelineObject.hpp"
#include "Image2D.hpp"
#include <vector>
#include "Streamer.hpp"
#include <boost/shared_ptr.hpp>

namespace fast {

class Image2Dt : public PipelineObject {
    FAST_OBJECT(Image2Dt)
    public:
        Image2D::Ptr getNextFrame();
        void addFrame(Image2D::Ptr frame);
    private:
        Image2Dt();

        // Flag whether to keep and store all frames or only use the current
        bool mKeepAllFrames;

        // If the flag mKeepAllFrames is set to false, this vector will have
        // a max size of 1
        std::vector<Image2D::Ptr> mFrames;

        // Keep track of which frame is next, only used when mKeepAllFrames is
        // set to true
        unsigned long mCurrentFrame;

        void execute();

        // Pointer to the streamer used to drive this object
        Streamer::Ptr mStreamer;
};

} // end namespace fast

#endif
