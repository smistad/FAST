#ifndef IMAGE2DT_HPP
#define IMAGE2DT_HPP

#include "PipelineObject.hpp"
#include "Image2D.hpp"
#include <vector>

namespace fast {

class Image2Dt : public PipelineObject {
    public:
        Image2DPtr getNextFrame();
        void addFrame(Image2DPtr frame);
    private:
        // Flag whether to keep and store all frames or only use the current
        bool mKeepAllFrames;

        // If the flag mKeepAllFrames is set to false, this vector will have
        // a max size of 1
        std::vector<Image2DPtr> mFrames;

        // Keep track of which frame is next, only used when mKeepAllFrames is
        // set to true
        unsigned long mCurrentFrame;

        void execute();
};

typedef boost::shared_ptr<Image2Dt> Image2DtPtr;

} // end namespace fast

#endif
