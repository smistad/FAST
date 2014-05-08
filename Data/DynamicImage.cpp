#include "DynamicImage.hpp"
using namespace fast;

Image::pointer DynamicImage::getNextFrame() {
    mStreamMutex.lock();
    // TODO: If no frame is available the method should maybe wait?
    if(mFrames.size() == 0)
        throw Exception("No frames available");
    Image::pointer ret;
    if(mStreamingMode == STREAMING_MODE_STORE_ALL_FRAMES) {
        // Get the next one
        ret = mFrames[mCurrentFrame];
        mCurrentFrame++;
    } else {
        // Always get the one in front
        ret = mFrames[0];
    }
    mStreamMutex.unlock();

    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        // Remove the frame that was read, the one in the front
        mFrames.erase(mFrames.begin());
    }

    if(mStreamingMode == STREAMING_MODE_STORE_ALL_FRAMES || mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        if(mFrames.size() > 0)
            updateModifiedTimestamp();
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
