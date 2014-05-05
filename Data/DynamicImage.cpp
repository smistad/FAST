#include "DynamicImage.hpp"
using namespace fast;

Image::pointer DynamicImage::getNextFrame() {
    mStreamMutex.lock();
    // TODO: If no frame is available the method should maybe wait?
    if(mFrames.size() == 0)
        throw Exception("No frames available");
    Image::pointer ret = mFrames[mCurrentFrame];
    mStreamMutex.unlock();
    if(mKeepAllFrames)
        mCurrentFrame++;
    return ret;
}

void DynamicImage::addFrame(Image::pointer frame) {
    mStreamMutex.lock();
    updateModifiedTimestamp();
    if(!mKeepAllFrames) {
        mFrames.clear();
    }
    mFrames.push_back(frame);
    mStreamMutex.unlock();
}

DynamicImage::DynamicImage() {
    mCurrentFrame = 0;
    mKeepAllFrames = false;
    mIsDynamicData = true;
    updateModifiedTimestamp();
}


unsigned int DynamicImage::getSize() const {
    return mFrames.size();
}
