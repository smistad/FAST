#include <thread>
#include <vector>
#include <FAST/Streamers/Streamer.hpp>
#include "DataPort.hpp"
#include "ProcessObject.hpp"

namespace fast {

void DataPort::addFrame(DataObject::pointer object) {

    if(mStreamingMode == STREAMING_MODE_NEWEST_FRAME_ONLY) {

        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(!mFrames.empty()) // If not empty, remove
                mFrames.pop();
            mFrames.push(object);
        }
        mFrameConditionVariable.notify_all();

    } else if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        //std::cout << "ADDING DATA at " << mProcessObject->getNameOfClass() << std::endl;
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
        } else {
            if(!mFrames.empty()) // When static, if not empty, remove element first in queue
                mFrames.pop();
        }

        {
            // Add data
            mFrames.push(object);
            //std::cout << "Data added at " << mProcessObject->getNameOfClass() << ", queue size: " << mFrames.size() << std::endl;
            mFrameCounter++;
        }

        if(!mIsStaticData) {
            // If data is not static, use semaphore to signal that a new data is available
            lock.unlock();
            //std::cout << "signaling fill count..." << std::endl;
            mFillCount->signal();
            //std::cout << "signaling fill count done " << mFillCount->getCount() << std::endl;
        } else {
            // If data is static, use condition variable to signal that a new data is available
            lock.unlock();
            mFrameConditionVariable.notify_all();
        }

    } else if(mStreamingMode == STREAMING_MODE_STORE_ALL_FRAMES) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            //std::cout << mProcessObject->getNameOfClass() + " STORE_ALL_FRAMES adding frame with nr " << mFrameCounter << std::endl;
            mFrames.push(object);
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
            //std::cout << "Waiting to get data from " << mProcessObject->getNameOfClass() << " " << mFillCount->getCount() << std::endl;
            mFillCount->wait();
            lock.lock();
        } else {
            lock.lock();
            // Do this using condition variable
            while(mFrames.empty() && !mStop) {
                mFrameConditionVariable.wait(lock);
            }
        }

        if(mStop) {
            throw ThreadStopped();
        }

        data = mFrames.front();
        m_previousFrame = data;
        //std::cout << "Got data from " << mProcessObject->getNameOfClass() << std::endl;

        if(!mIsStaticData) {
            std::cout << "Removing data from port with data from " << mProcessObject->getNameOfClass() << std::endl;
            mFrames.pop();
        }

        lock.unlock();
    }

    if(mStreamingMode == STREAMING_MODE_PROCESS_ALL_FRAMES && !mIsStaticData) {
        //std::cout << "Signaling empty count " << mProcessObject->getNameOfClass() << std::endl;
        mEmptyCount->signal();
    }

    mGetCalled = true;

    return data;
}

void DataPort::moveDataToNextTimestep() {
    std::lock_guard<std::mutex> lock(mMutex);
    /*
    if(!mFrames.empty() && mFrames.front() != m_previousFrame && m_previousFrame != nullptr) {
        std::cout << "adding previous frame: " << m_previousFrame << std::endl;
        mFrames.push(m_previousFrame);
    }*/
}

void DataPort::setStreamingMode(StreamingMode mode) {
    mStreamingMode = mode;
}

void DataPort::setTimestep(uint64_t timestep) {
}

DataPort::DataPort(SharedPointer<ProcessObject> processObject) {
    mProcessObject = processObject;
    setMaximumNumberOfFrames(50);
    auto isStreamer = std::dynamic_pointer_cast<Streamer>(processObject);
    if(isStreamer) {
        mIsStaticData = false;
    } else {
        mIsStaticData = true;
    }
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
    return !mFrames.empty();
}

uint DataPort::getSize() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mFrames.size();
}

DataObject::pointer DataPort::getFrame(uint64_t timestep) {
    std::lock_guard<std::mutex> lock(mMutex);
    if(mFrames.empty())
        throw Exception("No frames available in getFrame");
    return mFrames.front();
}

template <>
SharedPointer<DataObject> DataPort::getNextFrame<DataObject>() {
    return getNextDataFrame();
}


} // end namespace fast
