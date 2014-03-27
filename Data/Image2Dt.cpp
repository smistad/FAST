#include "Image2Dt.hpp"
using namespace fast;

Image2D::pointer Image2Dt::getNextFrame() {
    mStreamer->getStreamMutex().lock();
    Image2D::pointer ret = mFrames[mCurrentFrame];
    mStreamer->getStreamMutex().unlock();
    if(mKeepAllFrames)
        mCurrentFrame++;
    return ret;
}

void Image2Dt::addFrame(Image2D::pointer frame) {
    mStreamer->getStreamMutex().lock();
    updateModifiedTimestamp();
    if(!mKeepAllFrames) {
        mFrames.clear();
    }
    mFrames.push_back(frame);
    mStreamer->getStreamMutex().unlock();
}

Image2Dt::Image2Dt() {
    mCurrentFrame = 0;
    mKeepAllFrames = false;
    updateModifiedTimestamp();
}

void Image2Dt::setStreamer(Streamer *streamer) {
    mStreamer = streamer;
}
