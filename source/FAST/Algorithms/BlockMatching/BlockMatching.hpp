#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;

/**
 * 2D block matching on the GPU. Input is a stream of input images, output is a stream of images
 * with 2 channels giving the x,y motion of each pixel.
 */
class BlockMatching : public ProcessObject {
    FAST_OBJECT(BlockMatching)
    public:
    private:
        BlockMatching();
        void execute() override;

        SharedPointer<Image> m_previousFrame;

};

}