#include "DynamicImage.hpp"
using namespace fast;

Image::pointer DynamicImage::getNextFrame() {
    mStreamMutex.lock();

    // Check if no frames are available
    if(mFrames.size() == 0 || mFrames.size() <= mCurrentFrame) {
        if(mStreamer.lock()->hasReachedEnd()) {
            mStreamMutex.unlock();
            throw Exception("Streamer has reached the end.");
        } else {
            mStreamMutex.unlock();
            throw Exception("This exception should not have occured. There is a bug somewhere in which getNextFrame is called more than once for the same timestamp.");
        }
    }

    // Get the frame
    Image::pointer ret;
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
        // Get the next one
        ret = mFrames[mCurrentFrame];
        mCurrentFrame++;
    } else {
        // Always get the one in front
        ret = mFrames[0];
    }

    // Remove the frame from collection if necessary
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        // Remove the frame that was read, the one in the front
        mFrames.erase(mFrames.begin());
    }

    // Check if dynamic image has reached the end
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        if(mStreamer.lock()->hasReachedEnd())
            mHasReachedEnd = true;
    } else if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        if(mStreamer.lock()->hasReachedEnd() && mFrames.size() == 0)
            mHasReachedEnd = true;
    } else { // store all
        if(mStreamer.lock()->hasReachedEnd() && mCurrentFrame == mFrames.size())
            mHasReachedEnd = true;
    }

    // Update modified timestamp so that a process object will read the next frame
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
        if(mCurrentFrame < mFrames.size())
            updateModifiedTimestamp();
    } else if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        if(mFrames.size() > 0)
            updateModifiedTimestamp();
    }

    mStreamMutex.unlock();

    return ret;
}

void DynamicImage::addFrame(Image::pointer frame) {
    if(!mStreamer.lock().isValid())
        throw Exception("A DynamicImage must have a streamer set before it can be used.");
    mStreamMutex.lock();
    updateModifiedTimestamp();
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        mFrames.clear();
    }
    mFrames.push_back(frame);
    mStreamMutex.unlock();
}

DynamicImage::DynamicImage() {
    mCurrentFrame = 0;
    mIsDynamicData = true;
    mHasReachedEnd = false;
    updateModifiedTimestamp();
}


unsigned int DynamicImage::getSize() const {
    return mFrames.size();
}

void DynamicImage::setStreamer(Streamer::pointer streamer) {
    mStreamer = streamer;
}

Streamer::pointer DynamicImage::getStreamer() const {
    return mStreamer;
}


bool DynamicImage::hasReachedEnd() const {
    return mHasReachedEnd;
}
