#ifndef DynamicImage_HPP
#define DynamicImage_HPP

#include "FAST/Streamers/Streamer.hpp"
#include "FAST/Data/DataObject.hpp"
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/lexical_cast.hpp>

namespace fast {

class DynamicData : public DataObject {
    FAST_OBJECT(DynamicData)
    public:
        void setMaximumNumberOfFrames(uint nrOfFrames);
        DataObject::pointer getNextFrame(WeakPointer<Object> processObject);
        DataObject::pointer getNextFrame(Object::pointer processObject);
        void addFrame(DataObject::pointer frame);
        unsigned int getSize() const;
        ~DynamicData();

        bool hasReachedEnd();
        DataObject::pointer getCurrentFrame();
        void registerConsumer(WeakPointer<Object> processObject);
        void registerConsumer(Object::pointer processObject);
    private:


        // If the flag mKeepAllFrames is set to false, this vector will have
        // a max size of 1
        //std::vector<typename T::pointer> mFrames;

        // Keep track of which frame is next, only used when mKeepAllFrames is
        // set to true
        //unsigned long mCurrentFrame;

        boost::unordered_map<WeakPointer<Object>, uint> mConsumerFrameCounters;
        uint getLowestFrameCount() const;
        void removeOldFrames(uint frameCounter);
        void setAllConsumersUpToDate();
        // Maps frame counter to a data
        boost::unordered_map<uint, DataObject::pointer> mFrames2;
        // This is the frame number of HEAD
        unsigned long mCurrentFrameCounter;
        // Only used with newest frame only:
        DataObject::pointer mCurrentFrame2;

        uint mMaximumNrOfFrames;

        // Use named semaphore if Mac
#if defined(__APPLE__) || defined(__MACOSX)
        uint mSemaphoreNumber;
        boost::interprocess::named_semaphore* fillCount;
        boost::interprocess::named_semaphore* emptyCount;
#else
        boost::interprocess::interprocess_semaphore* fillCount;
        boost::interprocess::interprocess_semaphore* emptyCount;
#endif

        boost::mutex mStreamMutex;

        bool mHasReachedEnd;
    protected:
        DynamicData();
        // TODO not implemented yet
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
};

} // end namespace fast

#endif
