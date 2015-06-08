#include "DynamicData.hpp"
#include "FAST/ProcessObject.hpp"

namespace fast {


DynamicData::~DynamicData() {
    delete fillCount;
    delete emptyCount;

#if defined(__APPLE__) || defined(__MACOSX)
    std::string name = "FAST_fill_count_" + boost::lexical_cast<std::string>(mSemaphoreNumber);
    boost::interprocess::named_semaphore::remove(name.c_str());
    name = "FAST_empty_count_" + boost::lexical_cast<std::string>(mSemaphoreNumber);
    boost::interprocess::named_semaphore::remove(name.c_str());
#endif
}

void DynamicData::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
    if(mFrames2.size() > 0)
        throw Exception("Must call setMaximumNumberOfFrames before streaming is started");
    if(mMaximumNrOfFrames > 0) {
        delete fillCount;
        delete emptyCount;

        // Use named semaphore if Mac
#if defined(__APPLE__) || defined(__MACOSX)
        std::string name = "FAST_fill_count_" + boost::lexical_cast<std::string>(mSemaphoreNumber);
        boost::interprocess::named_semaphore::remove(name.c_str());
        std::string name2 = "FAST_empty_count_" + boost::lexical_cast<std::string>(mSemaphoreNumber);
        boost::interprocess::named_semaphore::remove(name2.c_str());
        fillCount = new boost::interprocess::named_semaphore(boost::interprocess::create_only, name.c_str(), 0);
        emptyCount = new boost::interprocess::named_semaphore(boost::interprocess::create_only, name2.c_str(), nrOfFrames);
#else
        fillCount = new boost::interprocess::interprocess_semaphore(0);
        emptyCount = new boost::interprocess::interprocess_semaphore(nrOfFrames);
#endif
    }
}

uint DynamicData::getLowestFrameCount() const {
    if(mConsumerFrameCounters.size() == 0)
        return 0;

    boost::unordered_map<WeakPointer<Object>, uint>::const_iterator it;
    uint lowestFrameCount = std::numeric_limits<uint>::max();
    for(it = mConsumerFrameCounters.begin(); it != mConsumerFrameCounters.end(); it++) {
        if(it->second < lowestFrameCount) {
            lowestFrameCount = it->second;
        }
    }

    return lowestFrameCount;
}

void DynamicData::removeOldFrames(uint frameCounter) {
    // TODO this could be implemented in a faster way
    boost::unordered_map<uint, DataObject::pointer>::iterator it = mFrames2.begin();
    while(it != mFrames2.end()) {
        if(it->first < frameCounter) {
            it = mFrames2.erase(it);
		} else {
			it++;
		}
    }
}

void DynamicData::registerConsumer(Object::pointer processObject) {
    registerConsumer(WeakPointer<Object>(processObject));
}

void DynamicData::registerConsumer(WeakPointer<Object> processObject) {
    Streamer::pointer streamer = getStreamer();
    mStreamMutex.lock();
     if(mConsumerFrameCounters.count(processObject) == 0) {
        if(streamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
            mConsumerFrameCounters[processObject] = mCurrentFrameCounter;
        } else if(streamer->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
            mConsumerFrameCounters[processObject] = 0;
        } else {
            mConsumerFrameCounters[processObject] = getLowestFrameCount();
        }
     }
    mStreamMutex.unlock();
}

DataObject::pointer DynamicData::getNextFrame(Object::pointer processObject) {
    return getNextFrame(WeakPointer<Object>(processObject));
}


void DynamicData::setAllConsumersUpToDate() {
    unsigned long timestamp = getTimestamp();
    boost::unordered_map<WeakPointer<Object>, uint>::iterator it;
    for(it = mConsumerFrameCounters.begin(); it != mConsumerFrameCounters.end(); it++) {
        ProcessObject::pointer consumer = ProcessObject::pointer(it->first.lock());
        consumer->updateTimestamp(mPtr.lock());
    }
}

DataObject::pointer DynamicData::getNextFrame(WeakPointer<Object> processObject) {
    Streamer::pointer streamer = getStreamer();
    if(!streamer.isValid()) {
        // If streamer has been deleted, return the current frame instead
        throw Exception("Streamer has been deleted, but someone has called getNextFrame");
        //return getCurrentFrame();
    }

    // Producer consumer model
    if(mMaximumNrOfFrames > 0 && streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        bool frameShouldBeRemoved = true;

        if(mConsumerFrameCounters.count(processObject) > 0) {
            uint thisFrameCounter = mConsumerFrameCounters[processObject]; // Current consumer frame counter

            // If any other frame counters are less or equal, we do not want to remove frame
            boost::unordered_map<WeakPointer<Object>, uint>::const_iterator it;
            for(it = mConsumerFrameCounters.begin(); it != mConsumerFrameCounters.end(); it++) {
                if(it->second <= thisFrameCounter) {
                    frameShouldBeRemoved = false;
                    break;
                }
            }
        }

        if(frameShouldBeRemoved)
            fillCount->wait(); // decrement
    }

    mStreamMutex.lock();
    if(streamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        // Always return last frame
        if(mCurrentFrameCounter == 0) {
            mStreamMutex.unlock();
            throw Exception("Trying to get next frame, when no frame has ever been given to dynamic data.");
        }
        DataObject::pointer returnData = mCurrentFrame2;
        mStreamMutex.unlock();
        return mCurrentFrame2;
    }
    // If process object is not registered, register it
    if(mConsumerFrameCounters.count(processObject) == 0) {
        if(streamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
            mConsumerFrameCounters[processObject] = mCurrentFrameCounter;
        } else if(streamer->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
            mConsumerFrameCounters[processObject] = 0;
        } else {
            uint lowestFrameCount = getLowestFrameCount(); // This must be assigned to a variable first to ensure correct result!
            mConsumerFrameCounters[processObject] = lowestFrameCount;
        }
    }
    // Return frame
    DataObject::pointer returnData;
    if(mFrames2.count(mConsumerFrameCounters[processObject]) > 0) {
        returnData = mFrames2[mConsumerFrameCounters[processObject]];
    } else {
        mStreamMutex.unlock();
        throw Exception("Frame in dynamic data was not found");
    }


    // Increment
    mConsumerFrameCounters[processObject] = mConsumerFrameCounters[processObject]+1;

    // If PROCESS_ALL and this has smallest frame counter, remove old frame
    if(streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        removeOldFrames(getLowestFrameCount());
        if(mFrames2.size() > 0) { // Update timestamp if there are more frames available
            updateModifiedTimestamp();
            // For each consumer
            boost::unordered_map<WeakPointer<Object>, uint>::iterator it;
            for(it = mConsumerFrameCounters.begin(); it != mConsumerFrameCounters.end(); it++) {
                // Check if next frame is available
                if(mFrames2.count(it->second) == 0) {
                    // Set consumer up to date, so that it will not request data yet
                    ProcessObject::pointer consumer = it->first.lock();
                    consumer->updateTimestamp(mPtr.lock());
                }
            }
        } else {
            // All frames are gone, make sure timestamps that all POs have are up to date
            setAllConsumersUpToDate();
        }
    } else if(streamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        // With newest frame only, always remove?
        //mFrames2.erase(removeFrame);
    } else {
        // Update timestamp if there are more frames available
        if(mConsumerFrameCounters[processObject] < mFrames2.size()) {
            updateModifiedTimestamp();
        }
    }

    mStreamMutex.unlock();
    // Producer consumer
    if(mMaximumNrOfFrames > 0 && streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        emptyCount->post(); // increment
    }

    return returnData;
}

DataObject::pointer DynamicData::getCurrentFrame() {
    Streamer::pointer streamer = getStreamer();
    mStreamMutex.lock();
    /*
    if(mFrames.size() == 0 || mFrames.size() <= mCurrentFrame) {
        if(!streamer.isValid()) {
            mStreamMutex.unlock();
            throw Exception("Streamer does not exist (anymore), stop.");
        } else {
            if(streamer->hasReachedEnd()) {
                mStreamMutex.unlock();
                throw Exception("Streamer has reached the end.");
            } else {
                mStreamMutex.unlock();
                throw Exception("This exception should not have occured. ");
            }
        }
    }
    */
    DataObject::pointer ret = mCurrentFrame2;
    mStreamMutex.unlock();
    return ret;
}

void DynamicData::addFrame(DataObject::pointer frame) {
    Streamer::pointer streamer = getStreamer();
    if(!streamer.isValid()) {
        //throw Exception("A DynamicImage must have a streamer set before it can be used.");
        return;
    }
    if(mMaximumNrOfFrames > 0) {
        if(streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
            // Producer consumer model using semaphores
            emptyCount->wait(); // decrement
            //down(mutex);
        } else if(streamer->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
            if(mFrames2.size() >= mMaximumNrOfFrames)
                throw NoMoreFramesException("Maximum number of frames reached. You can change the this number using the setMaximumNumberOfFrames method on the streamer/dynamic data objects.");
        }
    }
    mStreamMutex.lock();

    updateModifiedTimestamp();
    if(getStreamer()->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        mFrames2.clear();
    }
    mFrames2[mCurrentFrameCounter] = frame;
    mCurrentFrame2 = frame;
    mCurrentFrameCounter++;
    mStreamMutex.unlock();
    if(mMaximumNrOfFrames > 0 && streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        fillCount->post(); // increment
    }
}

DynamicData::DynamicData() {
    mCurrentFrameCounter = 0;
    mMaximumNrOfFrames = 0;
    mIsDynamicData = true;
    mHasReachedEnd = false;
    fillCount = NULL;
    emptyCount = NULL;
    updateModifiedTimestamp();

#if defined(__APPLE__) || defined(__MACOSX)
    static uint namedSemaphoresCount = 0;
    mSemaphoreNumber = namedSemaphoresCount;
    namedSemaphoresCount++;
#endif
}


unsigned int DynamicData::getSize() const {
    return mFrames2.size();
}


bool DynamicData::hasReachedEnd() {
    Streamer::pointer streamer = getStreamer();
    if(!streamer.isValid()) {
        throw Exception("A DynamicData must have a streamer set before it can be used.");
    }
    mStreamMutex.lock();
    // Check if has reached end can be changed to true
    if(!mHasReachedEnd) {
        // TODO the checks for NEWEST_FRAME AND PROCESS_ALL are not necessarily correct, for a pipeline with several steps
        switch(streamer->getStreamingMode()) {
        case STREAMING_MODE_NEWEST_FRAME_ONLY:
            // Should get frame nr of last to see if we have read the last
            if(streamer->hasReachedEnd())
                mHasReachedEnd = true;
            break;
        case STREAMING_MODE_PROCESS_ALL_FRAMES:
            if(streamer->hasReachedEnd() && mFrames2.size() == 0)
                mHasReachedEnd = true;
            break;
        case STREAMING_MODE_STORE_ALL_FRAMES:
            if(streamer->hasReachedEnd() && streamer->getNrOfFrames() == getLowestFrameCount())
                mHasReachedEnd = true;
            break;
        }
    }
    mStreamMutex.unlock();
    return mHasReachedEnd;
}



}
