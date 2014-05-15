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
        void setStreamer(Streamer::pointer streamer);
        Streamer::pointer getStreamer() const;
        bool hasReachedEnd();
    private:
        DynamicImage();

        WeakPointer<Streamer> mStreamer;

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

        bool mHasReachedEnd;
};

} // end namespace fast

#endif
