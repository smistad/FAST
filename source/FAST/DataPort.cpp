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
                object->setTimestep(mCurrentTimestep);
                mFrames[mCurrentTimestep] = object;
            } else {
                //std::cout << "Adding frame with nr " << mCurrentTimestep + 1 << std::endl;
                object->setTimestep(mCurrentTimestep+1);
                mFrames[mCurrentTimestep + 1] = object;
            }
        }
        mFrameConditionVariable.notify_all();

    } else if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        std::unique_lock<std::mutex> lock(mMutex);
        if(!mIsStaticData) {
            // If data is not static, use semaphore to check if available space for a new frame
            //std::cout << mProcessObject->getNameOfClass() + " waiting to add " << mCurrentTimestep << " (" << mFrameCounter << ") PROCESS_ALL_FRAMES" << std::endl;
            if(!mGetCalled && mFillCount->getCount() == mMaximumNumberOfFrames)
                Reporter::error() << "EXECUTION BLOCKED by DataPort from " << mProcessObject->getNameOfClass() << ". Do you have a DataPort object that is not used?" << Reporter::end();
            lock.unlock();
            mEmptyCount->wait();
            lock.lock();

            // If stop signal has been set, return
            if(mStop) {
                throw ThreadStopped();
            }
        }

        {
            // Add data
            if(mCurrentTimestep > mFrameCounter)
                mFrameCounter = mCurrentTimestep;
            //std::cout << mProcessObject->getNameOfClass() + " adding frame with nr " << mFrameCounter << std::endl;
            object->setTimestep(mFrameCounter);
            mFrames[mFrameCounter] = object;
            mFrameCounter++;
        }

        if(!mIsStaticData) {
            // If data is not static, use semaphore to signal that a new data is available
            lock.unlock();
            mFillCount->signal();
        } else {
            // If data is static, use condition variable to signal that a new data is available
            lock.unlock();
            mFrameConditionVariable.notify_all();
        }

    } else if(mStreamingMode == STREAMING_MODE_STORE_ALL_FRAMES) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(mCurrentTimestep > mFrameCounter)
                mFrameCounter = mCurrentTimestep;
            //std::cout << mProcessObject->getNameOfClass() + " STORE_ALL_FRAMES adding frame with nr " << mFrameCounter << std::endl;
            object->setTimestep(mFrameCounter);
            mFrames[mFrameCounter] = object;
            mFrameCounter++;
        }
        mFrameConditionVariable.notify_all();
    } else {
        throw Exception("Error in DataPort::addFrame");
    }
}

DataObject::pointer DataPort::getNextDataFrame() {
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
            while(mFrames.count(mCurrentTimestep) == 0 && !mStop) {
                //std::cout << "Waiting for " << mCurrentTimestep << std::endl;
                mFrameConditionVariable.wait(lock);
            }
        }

        if(mStop) {
            throw ThreadStopped();
        }

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

uint64_t DataPort::getFrameCounter() const {
    return mFrameCounter;
}

void DataPort::setMaximumNumberOfFrames(uint frames) {
    if(mFrameCounter > 0)
        throw Exception("Have to call setMaximumNumberOfFrames before executing pipeline");
    mMaximumNumberOfFrames = frames;
    mFillCount = std::make_unique<LightweightSemaphore>(0);
    mEmptyCount = std::make_unique<LightweightSemaphore>(mMaximumNumberOfFrames);
}

void DataPort::stop() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStop = true;
    }
    // Pipeline has been ordered to stop, wake up any threads.
    Reporter::info() << "STOPPING in DataPort for PO " << mProcessObject->getNameOfClass() << Reporter::end();
    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData) {
        Reporter::info() << "SIGNALING SEMAPHORES" << Reporter::end();
        mFillCount->signal();
        mEmptyCount->signal();
    } else {
        Reporter::info() << "Notifying condition variables" << Reporter::end();
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

DataObject::pointer DataPort::getFrame(uint64_t timestep) {
    return mFrames.at(timestep);
}

template <>
SharedPointer<DataObject> DataPort::getNextFrame<DataObject>() {
    return getNextDataFrame();
}


} // end namespace fast
