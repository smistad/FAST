#ifndef DynamicImage_HPP
#define DynamicImage_HPP

#include "Image.hpp"
#include "Streamer.hpp"
#include <vector>

namespace fast {

class DynamicImage : public ImageData {
    FAST_OBJECT(DynamicImage)
    public:
        Image::pointer getNextFrame();
        void addFrame(Image::pointer frame);
        unsigned int getSize() const;
        ~DynamicImage() {};
        void setStreamingMode(StreamingMode mode);
        StreamingMode getStreamingMode() const;
    private:
        DynamicImage();

        StreamingMode mStreamingMode;

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
