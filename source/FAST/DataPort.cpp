#include <thread>
#include <vector>
#include "DataPort.hpp"

namespace fast {

void DataPort::addFrame(DataObject::pointer object) {
    {
        if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData) {
            std::cout << "Waiting to add " << mCurrentTiemstep << std::endl;
            mEmptyCount->wait();
            std::cout << "OK to add" << std::endl;
        }
        std::lock_guard<std::mutex> lock(mMutex);
        if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES || mStreamingMode == STREAMING_MODE_STORE_ALL_FRAMES) {
            std::cout << "Adding frame with nr " << mCurrentTiemstep << std::endl;
            mFrames[mCurrentTiemstep] = object;
        } else if(mStreamingMode == STREAMING_MODE_NEWEST_FRAME_ONLY) {
            if(mFrames.count(mCurrentTiemstep) == 0) {
                std::cout << "Adding frame with nr " << mCurrentTiemstep << std::endl;
                mFrames[mCurrentTiemstep] = object;
            } else {
                std::cout << "Adding frame with nr " << mCurrentTiemstep + 1 << std::endl;
                mFrames[mCurrentTiemstep + 1] = object;
            }
        }
        mFrameCounter++;
    }

    setChanged(true);

    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData) {
        mFillCount->signal();
    } else {
        mFrameConditionVariable.notify_all();
    }
}

DataObject::pointer DataPort::getNextFrame() {
    // getNextFrame should **always** return the frame at the current timestep
    DataObject::pointer data;
    {
        std::unique_lock<std::mutex> lock(mMutex);
        lock.unlock();

        // If timestemp frame is not present, block until it is here
        if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData) {
            // Do this using semaphore
            std::cout << "Waiting to get " << mCurrentTiemstep << std::endl;
            mFillCount->wait();
            std::cout << "OK to get" << std::endl;
            lock.lock();
        } else {
            lock.lock();
            // Do this using condition variable
            while(mFrames.count(mCurrentTiemstep) == 0) {
                std::cout << "Waiting for " << mCurrentTiemstep << std::endl;
                mFrameConditionVariable.wait(lock);
            }
        }

        std::cout << "Trying to get frame at " << mCurrentTiemstep << std::endl;
        data = mFrames.at(mCurrentTiemstep);

        if(mStreamingMode != STREAMING_MODE_STORE_ALL_FRAMES) {
            // Find old frames to delete
            std::vector<uint64_t> framesToDelete;
            for(auto frame : mFrames) {
                if(frame.first < mCurrentTiemstep) {
                    framesToDelete.push_back(frame.first);
                }
            }
            // Delete old frames
            for(auto frameNr : framesToDelete)
                mFrames.erase(frameNr);
        }

        lock.unlock();
    }

    setChanged(true);

    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData)
        mEmptyCount->signal();

    return data;
}

void DataPort::moveDataToNextTimestep() {
    std::lock_guard<std::mutex> lock(mMutex);
    std::cout << "Moving data.." << std::endl;
    mFrames[mCurrentTiemstep] = mFrames.at(mCurrentTiemstep - 1);
    mFrames.erase(mCurrentTiemstep - 1);
    mIsStaticData = true;
}

void DataPort::setStreamingMode(StreamingMode mode) {
    mStreamingMode = mode;
}

void DataPort::setTimestep(uint64_t timestep) {
    mCurrentTiemstep = timestep;
}

DataPort::DataPort(SharedPointer<ProcessObject> processObject) {
    mProcessObject = processObject;
    setMaximumNumberOfFrames(50);
}

SharedPointer<ProcessObject> DataPort::getProcessObject() const {
    return mProcessObject;
}

void DataPort::setChanged(bool changed) {
    std::lock_guard<std::mutex> lock(mMutex);
    mChanged = changed;
}

bool DataPort::getChanged() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mChanged;
}

uint64_t DataPort::getFrameCounter() const {
    return mFrameCounter;
}

void DataPort::setMaximumNumberOfFrames(uint frames) {
    if(mFrameCounter > 0)
        throw Exception("Have to call setMaximumNumberOfFrames before executing pipeline");
    mMaximumNumberOfFrames = frames;
    mFillCount = UniquePointer<LightweightSemaphore>(new LightweightSemaphore(0));
    mEmptyCount = UniquePointer<LightweightSemaphore>(new LightweightSemaphore(mMaximumNumberOfFrames));
}

} // end namespace fast
