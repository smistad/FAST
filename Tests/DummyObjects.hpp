#ifndef DUMMYOBJECTS_HPP_
#define DUMMYOBJECTS_HPP_

#include "ProcessObject.hpp"
#include "DataObject.hpp"
#include <boost/unordered_map.hpp>

namespace fast {

// Create a dummy class that extends the ProcessObject which is an abstract class
class DummyProcessObject : public ProcessObject {
    FAST_OBJECT(DummyProcessObject)
    public:
        void setIsModified() { mIsModified = true; };
        bool hasExecuted() { return mHasExecuted; };
    private:
        DummyProcessObject() : mHasExecuted(false) {};
        void execute() { mHasExecuted = true; };
        bool mHasExecuted;
};


// Create a dummy class that extends the DataObject which is an abstract class
class DummyDataObject : public DataObject {
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

};




#endif /* DUMMYOBJECTS_HPP_ */
