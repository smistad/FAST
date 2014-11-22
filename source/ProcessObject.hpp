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
#include "ExecutionDevice.hpp"
#include "DeviceManager.hpp"

namespace fast {

class ProcessObject : public virtual Object {
    public:
        ProcessObject() :
            mIsModified(false),
            mRuntimeManager(new oul::RuntimeMeasurementsManager)
            { mDevices[0] = DeviceManager::getInstance().getDefaultComputationDevice(); };
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
        void removeParent(const DataObject::pointer data);
        virtual ~ProcessObject() {};
        void enableRuntimeMeasurements();
        void disableRuntimeMeasurements();

        void setMainDevice(ExecutionDevice::pointer device);
        ExecutionDevice::pointer getMainDevice() const;
        void setDevice(uint deviceNumber, ExecutionDevice::pointer device);
        ExecutionDevice::pointer getDevice(uint deviceNumber) const;
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
        void setInputData(uint inputNumber, DataObject::pointer data);
        void setInputData(uint inputNumber, DataObject::pointer data, const ExecutionDevice::pointer device);
        DataObject::pointer getInputData(uint inputNumber) const;
        template <class T>
        DataObject::pointer getOutputData(uint outputNumber);
        template <class StaticType, class DynamicType>
        DataObject::pointer getOutputData(uint outputNumber, DataObject::pointer objectDependsOn);
        uint getNrOfInputData() const;



    private:
        void changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device);
        void setTimestamp(DataObject::pointer object, unsigned long timestamp);
        void preExecute();
        void postExecute();

        boost::unordered_map<uint, DataObject::pointer> mInputs;
        boost::unordered_map<uint, bool> mRequiredInputs;
        boost::unordered_map<uint, bool> mReleaseAfterExecute;
        boost::unordered_map<uint, DataObject::pointer> mOutputs;
        boost::unordered_map<uint, std::vector<uint> > mInputDevices;
        boost::unordered_map<uint, ExecutionDevice::pointer> mDevices;

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
