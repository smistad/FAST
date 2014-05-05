#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "SmartPointers.hpp"
#include <boost/unordered_map.hpp>
#include <vector>
#include "Object.hpp"
#include "DataObject.hpp"
#include "RuntimeMeasurement.hpp"
#include "RuntimeMeasurementManager.hpp"

namespace fast {

class ProcessObject : public Object {
    public:
        ProcessObject() : mIsModified(false), mRuntimeManager(new oul::RuntimeMeasurementsManager) { };
        void update();
        typedef SharedPointer<ProcessObject> pointer;
        oul::RuntimeMeasurementPtr getRuntime();
        void addParent(DataObject::pointer parent);
        virtual ~ProcessObject() {};
        void enableRuntimeMeasurements();
        void disableRuntimeMeasurements();
    protected:
        // Pointer to the parent pipeline object
        std::vector<DataObject::pointer> mParentDataObjects;
        std::vector<unsigned long> mTimestamps;

        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

        virtual void waitToFinish() {};

        oul::RuntimeMeasurementsManagerPtr mRuntimeManager;

    private:
        void setTimestamp(DataObject::pointer object, unsigned long timestamp);

};

}; // end namespace fast

#endif
