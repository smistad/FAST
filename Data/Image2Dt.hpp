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
        std::vector<Image2DPtr> mFrames;
        void execute();
};

typedef boost::shared_ptr<Image2Dt> Image2DtPtr;

}; // end namespace fast

#endif
