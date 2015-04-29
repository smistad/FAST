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
#include "DynamicData.hpp"

namespace fast {


//enum InputDataType { INPUT_STATIC, INPUT_DYNAMIC, INPUT_STATIC_OR_DYNAMIC };
//enum OutputDataType { OUTPUT_STATIC, OUTPUT_DYNAMIC, OUTPUT_DEPENDS_ON_INPUT };

class ProcessObjectPort;

class ProcessObject : public virtual Object {
    public:
        ProcessObject();
        virtual ~ProcessObject() {};
        void update();
        typedef SharedPointer<ProcessObject> pointer;

        // Runtime stuff
        oul::RuntimeMeasurementPtr getRuntime();
        oul::RuntimeMeasurementPtr getRuntime(std::string name);
        void enableRuntimeMeasurements();
        void disableRuntimeMeasurements();

        // Device stuff
        void setMainDevice(ExecutionDevice::pointer device);
        void setMainDeviceCriteria(const DeviceCriteria& citeria);
        ExecutionDevice::pointer getMainDevice() const;
        void setDevice(uint deviceNumber, ExecutionDevice::pointer device);
        void setDeviceCriteria(uint deviceNumber, const DeviceCriteria& criteria);
        ExecutionDevice::pointer getDevice(uint deviceNumber) const;


        void setOutputData(uint outputNumber, DataObject::pointer object);

        // New pipeline methods
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
        ProcessObjectPort getInputPort(uint portID) const;
        void setInputData(uint portID, DataObject::pointer);
        void setInputData(DataObject::pointer);
        ProcessObjectPort getOutputPort();
        ProcessObjectPort getOutputPort(uint portID);
        /**
         * This method returns static data always. So if the input is dynamic data it will get the next frame.
         */
        template <class DataType>
        typename DataType::pointer getStaticInputData(uint inputNumber) const;
        template <class DataType>
        typename DataType::pointer getStaticInputData() const;
        /**
         * This method returns static data always. So if the output is dynamic data it will create a new frame and return that.
         */
        template <class DataType>
        typename DataType::pointer getStaticOutputData(uint portID);
        template <class DataType>
        typename DataType::pointer getStaticOutputData();

        template <class DataType>
        DataObject::pointer getOutputData(uint portID);
        template <class DataType>
        DataObject::pointer getOutputData();

    protected:
        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

        virtual void waitToFinish() {};

        oul::RuntimeMeasurementsManagerPtr mRuntimeManager;

        void setInputRequired(uint portID, bool required);
        void releaseInputAfterExecute(uint inputNumber, bool release);

        void setOutputDataDynamicDependsOnInputData(uint outputNumber, uint inputNumber);
        uint getNrOfInputData() const;
        DataObject::pointer getInputData(uint inputNumber) const;

    private:
        void updateTimestamp(DataObject::pointer data);
        void changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device);
        void preExecute();
        void postExecute();
        // This fetches output data without creating it
        DataObject::pointer getOutputDataX(uint portID) const;

        boost::unordered_map<uint, bool> mRequiredInputs;
        boost::unordered_map<uint, bool> mReleaseAfterExecute;
        boost::unordered_map<uint, std::vector<uint> > mInputDevices;
        boost::unordered_map<uint, ExecutionDevice::pointer> mDevices;
        boost::unordered_map<uint, uint> mOutputDynamicDependsOnInput;
        boost::unordered_map<uint, DeviceCriteria> mDeviceCriteria;

        // New pipeline
        boost::unordered_map<uint, ProcessObjectPort> mInputConnections;
        boost::unordered_map<uint, DataObject::pointer> mOutputData;

        friend class DynamicData;
        friend class ProcessObjectPort;
};


class ProcessObjectPort {
    public:
        ProcessObjectPort(uint portID, ProcessObject::pointer processObject);
        ProcessObjectPort() {};
        DataObject::pointer getData() const;
        uint getPortID() const;
        ProcessObject::pointer getProcessObject() const;
        bool isDataModified() const;
        void updateTimestamp();
        bool operator==(const ProcessObjectPort &other) const;
        friend std::size_t hash_value(fast::ProcessObjectPort const& obj) {
            std::size_t seed = 0;
            boost::hash_combine(seed, obj.getProcessObject().getPtr().get());
            boost::hash_combine(seed, obj.getPortID());
            return seed;
        }
    private:
        uint mPortID;
        ProcessObject::pointer mProcessObject;
        unsigned long mTimestamp;
};



template <class DataType>
DataObject::pointer ProcessObject::getOutputData(uint outputNumber) {
    DataObject::pointer data;

    // If output data is not created
    if(mOutputData.count(outputNumber) == 0) {
        // Is output dependent on any input?
        if(mOutputDynamicDependsOnInput.count(outputNumber) > 0) {
            uint inputNumber = mOutputDynamicDependsOnInput[outputNumber];
            if(mInputConnections.count(inputNumber) == 0)
                throw Exception("Must call input before output.");
            ProcessObjectPort port = mInputConnections[inputNumber];
            DataObject::pointer objectDependsOn = port.getData();
            if(objectDependsOn->isDynamicData()) {
                data = DynamicData::New();
                data->setStreamer(objectDependsOn->getStreamer());
            } else {
                data = DataType::New();
            }
            mOutputData[outputNumber] = data;
        } else {
            // Create data
            data = DataType::New();
            mOutputData[outputNumber] = data;
        }
    } else {
        data = mOutputData[outputNumber];
    }

    return data;
}

template <class DataType>
DataObject::pointer ProcessObject::getOutputData() {
    return getOutputData<DataType>(0);
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticInputData(uint inputNumber) const {
    // at throws exception if element not found, while [] does not
    ProcessObjectPort port = mInputConnections.at(inputNumber);
    DataObject::pointer data = port.getData();
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        returnData = typename DynamicData::pointer(data)->getNextFrame(mPtr);
    } else {
        returnData = data;
    }

    return returnData;
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticInputData() const {
    return getStaticInputData<DataType>(0);
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticOutputData(uint outputNumber) {
    // at throws exception if element not found, while [] does not
    DataObject::pointer data = getOutputData<DataType>(outputNumber);//mOutputs.at(outputNumber);
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        // Create new frame
        returnData = DataType::New();
        typename DynamicData::pointer(data)->addFrame(returnData);
    } else {
        returnData = data;
    }

    return returnData;
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticOutputData() {
    return getStaticOutputData<DataType>(0);
}


}; // end namespace fast

#endif
