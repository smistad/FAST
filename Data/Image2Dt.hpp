#ifndef IMAGE2DT_HPP
#define IMAGE2DT_HPP

#include "DynamicImage.hpp"
#include "Image.hpp"
#include <vector>
#include "Streamer.hpp"

namespace fast {

class Image2Dt : public DynamicImage {
    FAST_OBJECT(Image2Dt)
    public:
        Image::pointer getNextFrame();
        void addFrame(Image::pointer frame);
        unsigned int getSize() const;
        ~Image2Dt() {};
    private:
        Image2Dt();

        // Flag whether to keep and store all frames or only use the current
        bool mKeepAllFrames;

        // If the flag mKeepAllFrames is set to false, this vector will have
        // a max size of 1
        std::vector<Image::pointer> mFrames;

        // Keep track of which frame is next, only used when mKeepAllFrames is
        // set to true
        unsigned long mCurrentFrame;

        // TODO not implemented yet
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};

        boost::mutex mStreamMutex;
};

} // end namespace fast

#endif
