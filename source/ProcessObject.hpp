#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "SmartPointers.hpp"
#include <boost/unordered_map.hpp>
#include <vector>
#include "Object.hpp"
#include "DataObject.hpp"
#include "RuntimeMeasurement.hpp"
#include "RuntimeMeasurementManager.hpp"
#include "DynamicData.hpp"

namespace fast {

class ProcessObject : public virtual Object {
    public:
        ProcessObject() : mIsModified(false), mRuntimeManager(new oul::RuntimeMeasurementsManager) { };
        void update();
        typedef SharedPointer<ProcessObject> pointer;
        oul::RuntimeMeasurementPtr getRuntime();
        oul::RuntimeMeasurementPtr getRuntime(std::string name);
        void addParent(DataObject::pointer parent);
        /**
         * Remove any old parent objects and set the input as the parent object
         */
        void setParent(DataObject::pointer parent);
        /**
         * Remove all parent objects
         */
        void removeParents();
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


        void setInputRequired(uint inputNumber, bool required);
        void releaseInputAfterExecute(uint inputNumber, bool release);
        void setInputData(uint inputNumber, const DataObject::pointer data);
        DataObject::pointer getInputData(uint inputNumber) const;
        template <class T>
        DataObject::pointer getOutputData(uint outputNumber);
        template <class StaticType, class DynamicType>
        DataObject::pointer getOutputData(uint outputNumber, DataObject::pointer objectDependsOn);

    private:
        void setTimestamp(DataObject::pointer object, unsigned long timestamp);

        boost::unordered_map<uint, DataObject::pointer> mInputs;
        boost::unordered_map<uint, bool> mRequiredInputs;
        boost::unordered_map<uint, bool> mReleaseAfterExecute;
        boost::unordered_map<uint, DataObject::pointer> mOutputs;

};

template <class StaticType, class DynamicType>
DataObject::pointer ProcessObject::getOutputData(uint outputNumber,
        DataObject::pointer objectDependsOn) {
    if(!objectDependsOn.isValid())
        throw Exception("Must call input before output.");
    DataObject::pointer data;
    if(mOutputs.count(outputNumber) == 0) {
        if(objectDependsOn->isDynamicData()) {
            data = DynamicType::New();
            typename DynamicType::pointer(data)->setStreamer(
                typename DynamicType::pointer(objectDependsOn)->getStreamer());
        } else {
            data = StaticType::New();
        }
        data->setSource(mPtr.lock());
        mOutputs[outputNumber] = data;
    } else {
        data = mOutputs[outputNumber];
    }

    return data;
}


template <class T>
DataObject::pointer ProcessObject::getOutputData(uint outputNumber) {
    DataObject::pointer data;
    if(mOutputs.count(outputNumber) == 0) {
        // Create data
        data = T::New();
        data->setSource(mPtr.lock());
        mOutputs[outputNumber] = data;
    } else {
        data = mOutputs[outputNumber];
    }

    return data;
}


}; // end namespace fast

#endif
