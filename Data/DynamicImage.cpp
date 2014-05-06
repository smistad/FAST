#include "DynamicImage.hpp"
using namespace fast;

Image::pointer DynamicImage::getNextFrame() {
    mStreamMutex.lock();
    // TODO: If no frame is available the method should maybe wait?
    if(mFrames.size() == 0)
        throw Exception("No frames available");
    Image::pointer ret = mFrames[mCurrentFrame];
    mStreamMutex.unlock();
    if(mStreamingMode == STREAMING_MODE_KEEP_ALL_FRAMES) {
        mCurrentFrame++;
    }
    return ret;
}

void DynamicImage::addFrame(Image::pointer frame) {
    mStreamMutex.lock();
    updateModifiedTimestamp();
    if(mStreamingMode == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        mFrames.clear();
    }
    mFrames.push_back(frame);
    mStreamMutex.unlock();
}

DynamicImage::DynamicImage() {
    mCurrentFrame = 0;
    mStreamingMode = STREAMING_MODE_NEWEST_FRAME_ONLY;
    mIsDynamicData = true;
    updateModifiedTimestamp();
}


unsigned int DynamicImage::getSize() const {
    return mFrames.size();
}

void DynamicImage::setStreamingMode(StreamingMode mode) {
    mStreamingMode = mode;
}

StreamingMode DynamicImage::getStreamingMode() const {
    return mStreamingMode;
}
