#ifndef DATA_PORT_HPP_
#define DATA_PORT_HPP_
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "FAST/Data/DataObject.hpp"
#include "FAST/Data/DataTypes.hpp"
#include "FAST/Semaphore.hpp"

namespace fast {

enum StreamingMode { STREAMING_MODE_NEWEST_FRAME_ONLY, STREAMING_MODE_STORE_ALL_FRAMES, STREAMING_MODE_PROCESS_ALL_FRAMES };

class ProcessObject;

class FAST_EXPORT DataPort {
    public:
        explicit DataPort(SharedPointer<ProcessObject> processObject);

        void addFrame(DataObject::pointer data);

        template <class T = DataObject>
        SharedPointer<T> getNextFrame();

        void setTimestep(uint64_t timestep);

        void setStreamingMode(StreamingMode mode);

        SharedPointer<ProcessObject> getProcessObject() const;

        uint64_t getFrameCounter() const;

        /**
         *
         * @return the number of frames stored in this DataPort
         */
        uint getSize() const;

        /**
         * If a process object does not execeute one update iteration; it should call this method.
         */
        void moveDataToNextTimestep();

        void setMaximumNumberOfFrames(uint frames);

        /**
         * This will unblock if this DataPort is currently blocking. Used to stop a pipeline.
         */
        void stop();

        bool hasCurrentData();

        typedef SharedPointer<DataPort> pointer;

        DataObject::pointer getFrame(uint64_t timestep);
    private:
        /**
         * The process object which produce data for this port
         */
        SharedPointer<ProcessObject> mProcessObject;
        std::unordered_map<uint64_t, DataObject::pointer> mFrames;
        uint64_t mFrameCounter = 0;
        uint64_t mCurrentTimestep = 0;
        StreamingMode mStreamingMode = STREAMING_MODE_PROCESS_ALL_FRAMES;
        std::mutex mMutex;
        std::condition_variable mFrameConditionVariable;
        /**
         * Used to define buffer size for the producer consumer model used in streaming mode PROCESS_ALL_FRAMES
         */
        uint mMaximumNumberOfFrames;
        std::unique_ptr<LightweightSemaphore> mFillCount;
        std::unique_ptr<LightweightSemaphore> mEmptyCount;

        DataObject::pointer getNextDataFrame();

        bool mIsStaticData = false;
        bool mStop = false;
        bool mGetCalled = false;
};

// Template specialization when T = DataObject
template <>
FAST_EXPORT SharedPointer<DataObject> DataPort::getNextFrame<DataObject>();

template <class T>
SharedPointer<T> DataPort::getNextFrame() {
    auto data = getNextDataFrame();
    auto convertedData = std::dynamic_pointer_cast<T>(data);
    // Check if the conversion went ok
    if(!convertedData)
        throw BadCastException(data->getNameOfClass(), T::getStaticNameOfClass());
    return convertedData;
}

}

#endif
