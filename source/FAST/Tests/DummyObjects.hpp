#ifndef DUMMYOBJECTS_HPP_
#define DUMMYOBJECTS_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include <unordered_map>
#include <thread>

namespace fast {

// Create a dummy class that extends the DataObject which is an abstract class
class DummyDataObject : public SpatialDataObject {
    FAST_OBJECT(DummyDataObject)
    public:
        uint getID() const {
            return mID;
        };
        void create(uint ID) {
            mID = ID;
        }
        bool hasBeenFreed(ExecutionDevice::pointer device) {
            if(mFreed.count(device) > 0) {
                return mFreed[device];
            }
            return false;
        };
    private:
        DummyDataObject() {};
        void free(ExecutionDevice::pointer device) {
            mFreed[device] = true;
        };
        void freeAll() {};
        std::unordered_map<ExecutionDevice::pointer, bool> mFreed;
        uint mID;
};


// Create a dummy class that extends the ProcessObject which is an abstract class
class DummyProcessObject : public ProcessObject {
    FAST_OBJECT(DummyProcessObject)
    public:
        void setIsModified() { mIsModified = true; };
        void setIsNotModified() { mIsModified = false; };
        bool hasExecuted() { return mHasExecuted; };
        void setHasExecuted(bool value) { mHasExecuted = value; };
        void updateDataTimestamp() { getOutputData<DummyDataObject>(0)->updateModifiedTimestamp(); };
    private:
        DummyProcessObject() : mHasExecuted(false) {
            createInputPort<DummyDataObject>(0);
            createOutputPort<DummyDataObject>(0);
        };
        void execute() {
            mHasExecuted = true;
            auto input = getInputData<DummyDataObject>(0);
            auto output = DummyDataObject::New();
            output->create(input->getID());
            addOutputData(0, output);
        };
        bool mHasExecuted;
};


class DummyProcessObject2 : public ProcessObject {
    FAST_OBJECT(DummyProcessObject2)
    public:
        int getStaticDataID() const;
    private:
        DummyProcessObject2();
        void execute();

        int mStaticID = 0;

};

class DummyProcessObject3 : public ProcessObject {
    FAST_PROCESS_OBJECT(DummyProcessObject3)
    public:
    private:
        DummyProcessObject3();
        void execute();
};

class DummyStreamer : public Streamer {
    FAST_OBJECT(DummyStreamer)
    public:
        void setSleepTime(uint milliseconds);
        void setTotalFrames(uint frames);
        bool hasReachedEnd();
        void generateStream() override;
        uint getFramesToGenerate();
        ~DummyStreamer();
    private:
        DummyStreamer();
        void execute();

        uint mSleepTime = 1;
        uint mFramesToGenerate = 20;
        uint mFramesGenerated = 0;

        std::mutex mFramesGeneratedMutex;

};

class DummyImporter : public ProcessObject {
    FAST_OBJECT(DummyImporter)
    public:
        void setModified();
    protected:
        DummyImporter();
        void execute();
    private:
        int mExecuted = 0; // count number of times executed

};

};




#endif /* DUMMYOBJECTS_HPP_ */
