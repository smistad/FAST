#include "Image2Dt.hpp"
using namespace fast;

Image2D::Ptr Image2Dt::getNextFrame() {
    mStreamer->getStreamMutex().lock();
    Image2D::Ptr ret = mFrames[mCurrentFrame];
    mStreamer->getStreamMutex().unlock();
    if(mKeepAllFrames)
        mCurrentFrame++;
    return ret;
}

void Image2Dt::addFrame(Image2D::Ptr frame) {
    mStreamer->getStreamMutex().lock();
    mIsModified = true;
    if(!mKeepAllFrames) {
        mFrames.clear();
    }
    mFrames.push_back(frame);
    mStreamer->getStreamMutex().unlock();
}

Image2Dt::Image2Dt() {
    mCurrentFrame = 0;
    mKeepAllFrames = false;
    mIsModified = true;
}

void Image2Dt::execute() {
}
