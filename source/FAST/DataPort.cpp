#include <thread>
#include <vector>
#include "DataPort.hpp"
#include "ProcessObject.hpp"

namespace fast {

void DataPort::addFrame(DataObject::pointer object) {

    if(mStreamingMode == STREAMING_MODE_NEWEST_FRAME_ONLY) {

        {
            std::lock_guard<std::mutex> lock(mMutex);
            // If data for current doesn't exist: add it, otherwise add the new data for the next timestep
            if(mFrames.count(mCurrentTimestep) == 0) {
                //std::cout << "Adding frame with nr " << mCurrentTimestep << std::endl;
                mFrames[mCurrentTimestep] = object;
            } else {
                //std::cout << "Adding frame with nr " << mCurrentTimestep + 1 << std::endl;
                mFrames[mCurrentTimestep + 1] = object;
            }
        }
        mFrameConditionVariable.notify_all();

    } else if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        if(!mIsStaticData) {
            // If data is not static, use semaphore to check if available space for a new frame
            //std::cout << mProcessObject->getNameOfClass() + " waiting to add " << mCurrentTimestep << " (" << mFrameCounter << ") PROCESS_ALL_FRAMES" << std::endl;
            if(!mGetCalled && mFillCount->getCount() == mMaximumNumberOfFrames)
                Reporter::error() << "EXECUTION BLOCKED by DataPort from " << mProcessObject->getNameOfClass() << ". Do you have a DataPort object that is not used?" << Reporter::end();
            mEmptyCount->wait();

            // If stop signal has been set, return
            if(mStop) {
                return;
            }
        }

        {
            // Add data
            std::lock_guard<std::mutex> lock(mMutex);
            if(mCurrentTimestep > mFrameCounter)
                mFrameCounter = mCurrentTimestep;
            //std::cout << mProcessObject->getNameOfClass() + " adding frame with nr " << mFrameCounter << std::endl;
            mFrames[mFrameCounter] = object;
            mFrameCounter++;
        }

        if(!mIsStaticData) {
            // If data is not static, use semaphore to signal that a new data is available
            mFillCount->signal();
        } else {
            // If data is static, use condition variable to signal that a new data is available
            mFrameConditionVariable.notify_all();
        }

    } else if(mStreamingMode == STREAMING_MODE_STORE_ALL_FRAMES) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(mCurrentTimestep > mFrameCounter)
                mFrameCounter = mCurrentTimestep;
            //std::cout << mProcessObject->getNameOfClass() + " STORE_ALL_FRAMES adding frame with nr " << mFrameCounter << std::endl;
            mFrames[mFrameCounter] = object;
            mFrameCounter++;
        }
        mFrameConditionVariable.notify_all();
    } else {
        throw Exception("Error in DataPort::addFrame");
    }
    setChanged(true);
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
            //std::cout << "Waiting to get " << mCurrentTimestep << std::endl;
            mFillCount->wait();
            lock.lock();
        } else {
            lock.lock();
            // Do this using condition variable
            while(mFrames.count(mCurrentTimestep) == 0) {
                //std::cout << "Waiting for " << mCurrentTimestep << std::endl;
                mFrameConditionVariable.wait(lock);
            }
        }

        if(mStop) {
            if(mFrames.count(mCurrentTimestep) > 0) {
                return mFrames.at(mCurrentTimestep);
            } else {
                return mFrames.at(mCurrentTimestep - 1);
            }
        }

        //std::cout << "Trying to get frame at " << mCurrentTimestep << std::endl;
        data = mFrames.at(mCurrentTimestep);

        if(mStreamingMode != STREAMING_MODE_STORE_ALL_FRAMES) {
            // Find old frames to delete
            std::vector<uint64_t> framesToDelete;
            for(auto frame : mFrames) {
                if(frame.first < mCurrentTimestep) {
                    framesToDelete.push_back(frame.first);
                }
            }
            // Delete old frames
            for(auto frameNr : framesToDelete) {
                mFrames.erase(frameNr);
            }
        }

        lock.unlock();
    }

    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData)
        mEmptyCount->signal();

    mGetCalled = true;

    return data;
}

void DataPort::moveDataToNextTimestep() {
    std::lock_guard<std::mutex> lock(mMutex);
    //std::cout << "Moving data for " << mProcessObject->getNameOfClass() << " at timestep " << mCurrentTimestep << " size: " << mFrames.size() << " first t " << mFrames.begin()->first << std::endl;
    if(mFrames.count(mCurrentTimestep) == 0) {
        // Only move if frame is not there
        mFrames[mCurrentTimestep] = mFrames.at(mCurrentTimestep - 1);
        mFrames.erase(mCurrentTimestep - 1);
    }
    //std::cout << "Moving data finished" << std::endl;
    mIsStaticData = true;
    //mChanged = true;
}

void DataPort::setStreamingMode(StreamingMode mode) {
    mStreamingMode = mode;
}

void DataPort::setTimestep(uint64_t timestep) {
    mCurrentTimestep = timestep;
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

void DataPort::stop() {
    mStop = true;
    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData) {
        mFillCount->signal();
        mEmptyCount->signal();
    } else {
        mFrameConditionVariable.notify_all();
    }
}

bool DataPort::hasCurrentData() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mFrames.count(mCurrentTimestep) > 0;
}

uint DataPort::getSize() const {
    return mFrames.size();
}

} // end namespace fast
