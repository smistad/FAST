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

class ProcessObjectPort;

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
        void setMainDeviceCriteria(const DeviceCriteria& citeria);
        ExecutionDevice::pointer getMainDevice() const;
        void setDevice(uint deviceNumber, ExecutionDevice::pointer device);
        void setDeviceCriteria(uint deviceNumber, const DeviceCriteria& criteria);
        ExecutionDevice::pointer getDevice(uint deviceNumber) const;
        void setOutputData(uint outputNumber, DataObject::pointer object);

        // New pipeline methods
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
        ProcessObjectPort getOutputPort();
        ProcessObjectPort getOutputPort(uint portID);
        DataObject::pointer getOutputDataX(uint portID) const;
        /**
         * This method returns static data always. So if the input is dynamic data it will get the next frame.
         */
        template <class DataType>
        DataObject::pointer getStaticInputData(uint inputNumber) const;
        /**
         * This method returns static data always. So if the output is dynamic data it will create a new frame and return that.
         */
        template <class DataType>
        DataObject::pointer getStaticOutputData(uint outputNumber);
        template <class StaticDataType, class DynamicDataType>
        DataObject::pointer getOutputData(uint outputNumber);
        template <class DataType>
        DataObject::pointer getOutputData(uint outputNumber);
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


        void setInputRequired(uint portID, bool required);
        void releaseInputAfterExecute(uint inputNumber, bool release);
        //void setInputData(uint inputNumber, DataObject::pointer data);
        //void setInputData(uint inputNumber, DataObject::pointer data, const ExecutionDevice::pointer device);

        void setOutputDataDynamicDependsOnInputData(uint outputNumber, uint inputNumber);
        uint getNrOfInputData() const;


        // New pipeline
        void setOutputDataX(uint portID, DataObject::pointer data);
        DataObject::pointer getInputData(uint inputNumber) const;
        ProcessObjectPort getInputPort(uint portID) const;

    private:
        void changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device);
        void setTimestamp(DataObject::pointer object, unsigned long timestamp);
        void preExecute();
        void postExecute();

        boost::unordered_map<uint, bool> mRequiredInputs;
        boost::unordered_map<uint, bool> mReleaseAfterExecute;
        boost::unordered_map<uint, std::vector<uint> > mInputDevices;
        boost::unordered_map<uint, ExecutionDevice::pointer> mDevices;
        boost::unordered_map<uint, uint> mOutputDynamicDependsOnInput;
        boost::unordered_map<uint, DeviceCriteria> mDeviceCriteria;

        // New pipeline
        boost::unordered_map<uint, ProcessObjectPort> mInputConnections;
        boost::unordered_map<uint, DataObject::pointer> mOutputData;


        template <class T>
        friend class DynamicData;
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
    if(mOutputDynamicDependsOnInput.count(outputNumber) > 0) {
        throw Exception("Your output data depends on input data. Use the method getOutputData<StaticDataType, DynamicDatatype>() instead of getOutputData<DataType>().");
    }
    DataObject::pointer data;

    // If output data is not created
    if(mOutputData.count(outputNumber) == 0) {
        // Create data
        data = DataType::New();
        data->setSource(mPtr.lock());
        mOutputData[outputNumber] = data;
        std::cout << "data created at output port " << outputNumber << std::endl;
    } else {
        //data = mOutputs[outputNumber];
        data = mOutputData[outputNumber];
    }

    return data;
}

template <class StaticDataType, class DynamicDataType>
DataObject::pointer ProcessObject::getOutputData(uint outputNumber) {
    DataObject::pointer data;

    // If output data is not created
    if(mOutputData.count(outputNumber) == 0) {
        // Is output dependent on any input?
        // TODO: problem this only works if input and output are of same type
        if(mOutputDynamicDependsOnInput.count(outputNumber) > 0) {
            uint inputNumber = mOutputDynamicDependsOnInput[outputNumber];
            if(mInputConnections.count(inputNumber) == 0)
                throw Exception("Must call input before output.");
            ProcessObjectPort port = mInputConnections[inputNumber];
            DataObject::pointer objectDependsOn = port.getData();//mInputs[inputNumber];
            if(objectDependsOn->isDynamicData()) {
                std::cout << "IS DYNAMIC" << std::endl;
                data = DynamicDataType::New();
                std::cout << "AAAA" << std::endl;
                data->setStreamer(objectDependsOn->getStreamer());
                std::cout << "AAAA" << std::endl;
            } else {
                data = StaticDataType::New();
            }
            data->setSource(mPtr.lock());
                std::cout << "AAAA" << std::endl;
            mOutputData[outputNumber] = data;
        } else {
            // Create data
            data = StaticDataType::New();
            data->setSource(mPtr.lock());
            mOutputData[outputNumber] = data;
        }
    } else {
        data = mOutputData[outputNumber];//mOutputs[outputNumber];
    }

    return data;
}

template <class DataType>
DataObject::pointer ProcessObject::getStaticInputData(uint inputNumber) const {
    // at throws exception if element not found, while [] does not
    ProcessObjectPort port = mInputConnections.at(inputNumber);
    DataObject::pointer data = port.getData();//mInputs.at(inputNumber);
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        returnData = typename DynamicData<DataType>::pointer(data)->getNextFrame(mPtr);
    } else {
        returnData = data;
    }

    return returnData;
}

template <class DataType>
DataObject::pointer ProcessObject::getStaticOutputData(uint outputNumber) {
    // at throws exception if element not found, while [] does not
    DataObject::pointer data = getOutputData<DataType, DynamicData<DataType> >(outputNumber);//mOutputs.at(outputNumber);
    std::cout << "LKAJHD" << std::endl;
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        // Create new frame
        returnData = DataType::New();
        typename DynamicData<DataType>::pointer(data)->addFrame(returnData);
    } else {
        returnData = data;
    }
    std::cout << "KJAHD" << std::endl;

    return returnData;
}


}; // end namespace fast

// A custum boost hashing function for the ProcessObjectPort object so that they can be used
// in unordered data structures. TODO verify that this works
namespace boost {

}
#endif
