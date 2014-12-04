#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "SmartPointers.hpp"
#include <boost/unordered_map.hpp>
#include <vector>
#include "Object.hpp"
#include "DataObject.hpp"
#include "RuntimeMeasurement.hpp"
#include "RuntimeMeasurementManager.hpp"
#include "ExecutionDevice.hpp"
#include "DeviceManager.hpp"

namespace fast {

template <class T>
class DynamicData;

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
        void setOutputData(uint outputNumber, DataObject::pointer object);
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
        /**
         * This method returns static data always. So if the input is dynamic data it will get the next frame.
         */
        template <class DataType>
        DataObject::pointer getStaticInputData(uint inputNumber) const;
        /**
         * This method returns static data always. So if the output is dynamic data it will create a new frame and return that.
         */
        template <class DataType>
        DataObject::pointer getStaticOutputData(uint outputNumber) const;
        template <class StaticDataType, class DynamicDataType>
        DataObject::pointer getOutputData(uint outputNumber);
        template <class DataType>
        DataObject::pointer getOutputData(uint outputNumber);
        void setOutputDataDynamicDependsOnInputData(uint outputNumber, uint inputNumber);
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
        boost::unordered_map<uint, uint> mOutputDynamicDependsOnInput;

        template <class T>
        friend class DynamicData;
};

template <class DataType>
DataObject::pointer ProcessObject::getOutputData(uint outputNumber) {
    if(mOutputDynamicDependsOnInput.count(outputNumber) > 0) {
        throw Exception("Your output data depends on input data. Use the method getOutputData<StaticDataType, DynamicDatatype>() instead of getOutputData<DataType>().");
    }
    DataObject::pointer data;

    // If output data is not created
    if(mOutputs.count(outputNumber) == 0) {
        // Create data
        data = DataType::New();
        data->setSource(mPtr.lock());
        mOutputs[outputNumber] = data;
    } else {
        data = mOutputs[outputNumber];
    }

    return data;
}

template <class StaticDataType, class DynamicDataType>
DataObject::pointer ProcessObject::getOutputData(uint outputNumber) {
    DataObject::pointer data;

    // If output data is not created
    if(mOutputs.count(outputNumber) == 0) {
        // Is output dependent on any input?
        if(mOutputDynamicDependsOnInput.count(outputNumber) > 0) {
            uint inputNumber = mOutputDynamicDependsOnInput[outputNumber];
            if(mInputs.count(inputNumber) == 0)
                throw Exception("Must call input before output.");
            DataObject::pointer objectDependsOn = mInputs[inputNumber];
            if(objectDependsOn->isDynamicData()) {
                data = DynamicDataType::New();
                typename DynamicDataType::pointer(data)->setStreamer(
                    typename DynamicDataType::pointer(objectDependsOn)->getStreamer());
            } else {
                data = StaticDataType::New();
            }
            data->setSource(mPtr.lock());
            mOutputs[outputNumber] = data;
        } else {
            // Create data
            data = StaticDataType::New();
            data->setSource(mPtr.lock());
            mOutputs[outputNumber] = data;
        }
    } else {
        data = mOutputs[outputNumber];
    }

    return data;
}

template <class DataType>
DataObject::pointer ProcessObject::getStaticInputData(uint inputNumber) const {
    // at throws exception if element not found, while [] does not
    DataObject::pointer data = mInputs.at(inputNumber);
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        returnData = typename DynamicData<DataType>::pointer(data)->getNextFrame(mPtr);
    } else {
        returnData = data;
    }

    return returnData;
}

template <class DataType>
DataObject::pointer ProcessObject::getStaticOutputData(uint outputNumber) const {
    // at throws exception if element not found, while [] does not
    DataObject::pointer data = mOutputs.at(outputNumber);
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        // Create new frame
        returnData = DataType::New();
        typename DynamicData<DataType>::pointer(data)->addFrame(returnData);
    } else {
        returnData = data;
    }

    return returnData;
}


}; // end namespace fast

#endif
