#ifndef DynamicImage_HPP
#define DynamicImage_HPP

#include "Image.hpp"
#include <vector>
#include "Streamer.hpp"

namespace fast {

class DynamicImage : public ImageData {
    FAST_OBJECT(DynamicImage)
    public:
        Image::pointer getNextFrame();
        void addFrame(Image::pointer frame);
        unsigned int getSize() const;
        ~DynamicImage() {};
    private:
        DynamicImage();

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
