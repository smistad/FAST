#ifndef DynamicImage_HPP
#define DynamicImage_HPP

#include "Streamer.hpp"
#include "DataObject.hpp"
#include <vector>
#include <boost/unordered_map.hpp>

namespace fast {

template <class T>
class DynamicData : public virtual DataObject {
    public:                                                     
        typedef SharedPointer<DynamicData<T> > pointer;
        static typename DynamicData<T>::pointer New() {                       
            DynamicData<T> * ptr = new DynamicData<T>();                  
            typename DynamicData<T>::pointer smartPtr(ptr);                   
            ptr->setPtr(smartPtr);                              
                                                                
            return smartPtr;                                    
        }                                                       
    private:                                                    
        void setPtr(typename DynamicData<T>::pointer ptr) {                   
            mPtr = ptr;                                         
        }                                                       
    public:
        //typename T::pointer getNextFrame();
        typename T::pointer getNextFrame(WeakPointer<Object> processObject);
        typename T::pointer getNextFrame(Object::pointer processObject);
        void addFrame(typename T::pointer frame);
        unsigned int getSize() const;
        ~DynamicData() {};
        void setStreamer(Streamer::pointer streamer);
        Streamer::pointer getStreamer();
        bool hasReachedEnd();
        typename T::pointer getCurrentFrame();
        void registerConsumer(WeakPointer<Object> processObject);
        void registerConsumer(Object::pointer processObject);
    private:

        WeakPointer<Streamer> mStreamer;

        // If the flag mKeepAllFrames is set to false, this vector will have
        // a max size of 1
        //std::vector<typename T::pointer> mFrames;

        // Keep track of which frame is next, only used when mKeepAllFrames is
        // set to true
        //unsigned long mCurrentFrame;

        boost::unordered_map<WeakPointer<Object>, uint> mConsumerFrameCounters;
        uint getLowestFrameCount() const;
        void removeOldFrames(uint frameCounter);
        // Maps frame counter to a data
        boost::unordered_map<uint, typename T::pointer> mFrames2;
        // This is the frame number of HEAD
        unsigned long mCurrentFrameCounter;
        // Only used with newest frame only:
        typename T::pointer mCurrentFrame2;


        boost::mutex mStreamMutex;

        bool mHasReachedEnd;
    protected:
        DynamicData();
        // TODO not implemented yet
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
};

/*
template <class T>
typename T::pointer DynamicData<T>::getNextFrame() {
    Streamer::pointer streamer = mStreamer.lock();
    if(!streamer.isValid()) {
        // If streamer has been deleted, return the current frame instead
        return getCurrentFrame();
    }
    mStreamMutex.lock();

    // Check if no frames are available
    if(mFrames.size() == 0 || mFrames.size() <= mCurrentFrame) {
        if(streamer->hasReachedEnd()) {
            mStreamMutex.unlock();
            throw Exception("Streamer has reached the end.");
        } else {
            mStreamMutex.unlock();
            throw Exception("This exception should not have occured. There is a bug somewhere in which getNextFrame is called more than once for the same timestamp.");
        }
    }

    // Get the frame
    typename T::pointer ret;
    if(streamer->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
        // Get the next one
        ret = mFrames[mCurrentFrame];
        mCurrentFrame++;
    } else {
        // Always get the one in front
        ret = mFrames[0];
    }

    // Remove the frame from collection if necessary
    if(streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        // Remove the frame that was read, the one in the front
        mFrames.erase(mFrames.begin());
    }

    // Update modified timestamp so that a process object will read the next frame
    if(streamer->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
        if(mCurrentFrame < mFrames.size())
            updateModifiedTimestamp();
    } else if(streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        if(mFrames.size() > 0)
            updateModifiedTimestamp();
    }

    mStreamMutex.unlock();

    mBoundingBox = ret->getBoundingBox();

    return ret;
}
*/

template <class T>
uint DynamicData<T>::getLowestFrameCount() const {
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

template <class T>
void DynamicData<T>::removeOldFrames(uint frameCounter) {
    // TODO this could be implemented in a faster way
    typename boost::unordered_map<uint, typename T::pointer>::iterator it;
    for(it = mFrames2.begin(); it != mFrames2.end(); it++) {
        if(it->first < frameCounter) {
            mFrames2.erase(it);
        }
    }
}

template <class T>
void DynamicData<T>::registerConsumer(Object::pointer processObject) {
    registerConsumer(WeakPointer<Object>(processObject));
}

template <class T>
void DynamicData<T>::registerConsumer(WeakPointer<Object> processObject) {
    Streamer::pointer streamer = mStreamer.lock();
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

template <class T>
typename T::pointer DynamicData<T>::getNextFrame(Object::pointer processObject) {
    return getNextFrame(WeakPointer<Object>(processObject));
}

template <class T>
typename T::pointer DynamicData<T>::getNextFrame(WeakPointer<Object> processObject) {
    Streamer::pointer streamer = mStreamer.lock();
    if(!streamer.isValid()) {
        // If streamer has been deleted, return the current frame instead
        throw Exception("Streamer has been deleted, but someone has called getNextFrame");
        //return getCurrentFrame();
    }
    mStreamMutex.lock();
    if(streamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        // Always return last frame
        if(mCurrentFrameCounter == 0)
            throw Exception("Trying to get next frame, when no frame has ever been given to dynamic data.");
        typename T::pointer returnData = mCurrentFrame2;
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
            mConsumerFrameCounters[processObject] = getLowestFrameCount();
        }
    }
    // Return frame
    typename T::pointer returnData;
    if(mFrames2.count(mConsumerFrameCounters[processObject]) > 0) {
        returnData = mFrames2[mConsumerFrameCounters[processObject]];
    } else {
        throw Exception("Frame in dynamic data was not found");
    }


    // Increment
    mConsumerFrameCounters[processObject] = mConsumerFrameCounters[processObject]+1;

    // If PROCESS_ALL and this has smallest frame counter, remove old frame
    if(streamer->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        removeOldFrames(getLowestFrameCount());
        if(mFrames2.size() > 0) // Update timestamp if there are more frames available
            updateModifiedTimestamp();
    } else if(streamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        // With newest frame only, always remove?
        //mFrames2.erase(removeFrame);
    } else {
        // Update timestamp if there are more frames available
        if(mConsumerFrameCounters[processObject] < mFrames2.size())
            updateModifiedTimestamp();
    }


    mStreamMutex.unlock();
    return returnData;
}

template <class T>
typename T::pointer DynamicData<T>::getCurrentFrame() {
    Streamer::pointer streamer = mStreamer.lock();
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
    typename T::pointer ret = mCurrentFrame2;
    mStreamMutex.unlock();
    return ret;
}

template <class T>
void DynamicData<T>::addFrame(typename T::pointer frame) {
    Streamer::pointer streamer = mStreamer.lock();
    if(!streamer.isValid()) {
        //throw Exception("A DynamicImage must have a streamer set before it can be used.");
        return;
    }
    mStreamMutex.lock();

    updateModifiedTimestamp();
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        mFrames2.clear();
    }
    mFrames2[mCurrentFrameCounter] = frame;
    mCurrentFrame2 = frame;
    mCurrentFrameCounter++;
    mStreamMutex.unlock();
}

template <class T>
DynamicData<T>::DynamicData() {
    mCurrentFrameCounter = 0;
    mIsDynamicData = true;
    mHasReachedEnd = false;
    updateModifiedTimestamp();
}


template <class T>
unsigned int DynamicData<T>::getSize() const {
    return mFrames2.size();
}

template <class T>
void DynamicData<T>::setStreamer(Streamer::pointer streamer) {
    mStreamer = streamer;
}

template <class T>
Streamer::pointer DynamicData<T>::getStreamer() {
    return mStreamer.lock();
}

template <class T>
bool DynamicData<T>::hasReachedEnd() {
    Streamer::pointer streamer = mStreamer.lock();
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

} // end namespace fast

#endif
