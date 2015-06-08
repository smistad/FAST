#ifndef DUMMYOBJECTS_HPP_
#define DUMMYOBJECTS_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"
#include "FAST/Data/DynamicData.hpp"
#include <boost/unordered_map.hpp>

namespace fast {

// Create a dummy class that extends the DataObject which is an abstract class
class DummyDataObject : public SpatialDataObject {
    FAST_OBJECT(DummyDataObject)
    public:
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
        boost::unordered_map<ExecutionDevice::pointer, bool> mFreed;
};


// Create a dummy class that extends the ProcessObject which is an abstract class
class DummyProcessObject : public ProcessObject {
    FAST_OBJECT(DummyProcessObject)
    public:
        void setIsModified() { mIsModified = true; };
        void setIsNotModified() { mIsModified = false; };
        void setInputRequired(uint number) { ProcessObject::setInputRequired(number, true); };
        bool hasExecuted() { return mHasExecuted; };
        void setHasExecuted(bool value) { mHasExecuted = value; };
        void updateDataTimestamp() { getOutputData<DummyDataObject>(0)->updateModifiedTimestamp(); };
    private:
        DummyProcessObject() : mHasExecuted(false) {
            createInputPort<DummyDataObject>(0, false);
            createOutputPort<DummyDataObject>(0, OUTPUT_STATIC, 0);
        };
        void execute() {
            mHasExecuted = true;
            getOutputData<DummyDataObject>(0);
        };
        bool mHasExecuted;
};

};




#endif /* DUMMYOBJECTS_HPP_ */
