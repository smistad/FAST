#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "FAST/SmartPointers.hpp"
#include <boost/unordered_map.hpp>
#include <vector>
#include "FAST/Object.hpp"
#include "FAST/Data/DataObject.hpp"
#include "RuntimeMeasurement.hpp"
#include "RuntimeMeasurementManager.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/DynamicData.hpp"

namespace fast {


enum InputDataType { INPUT_STATIC, INPUT_DYNAMIC, INPUT_STATIC_OR_DYNAMIC };
enum OutputDataType { OUTPUT_STATIC, OUTPUT_DYNAMIC, OUTPUT_DEPENDS_ON_INPUT };

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

        template <class DataType>
        void createInputPort(uint portID, bool required = true, InputDataType = INPUT_STATIC_OR_DYNAMIC);
        template <class DataType>
        void createOutputPort(uint portID, OutputDataType, int inputPortID = -1);

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

        bool inputPortExists(uint portID) const;
        bool outputPortExists(uint portID) const;
        virtual std::string getNameOfClass() const = 0;
        static std::string getStaticNameOfClass() {
            return "ProcessObject";
        }
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
        uint getNrOfOutputPorts() const;
        DataObject::pointer getInputData(uint inputNumber) const;

        template <class T>
        void setStaticOutputData(uint portID, DataObject::pointer data);

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
        // Contains a string of required port class
        boost::unordered_map<uint, std::string> mInputPortClass;
        boost::unordered_map<uint, std::string> mOutputPortClass;
        // Whether the ports accept dynamic, static or any of the two
        boost::unordered_map<uint, InputDataType> mInputPortType;
        boost::unordered_map<uint, OutputDataType> mOutputPortType;

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
        } else if(mOutputPortType[outputNumber] == OUTPUT_STATIC) {
            // Create static data
            data = DataType::New();
            mOutputData[outputNumber] = data;
        } else {
            // Create dynamic data
            data = DynamicData::New();
            mOutputData[outputNumber] = data;
        }
    } else {
        data = mOutputData[outputNumber];
    }

    /*
    if(DataType::getStaticNameOfClass() != "") {
        if(mOutputPortClass[outputNumber] != DataType::getStaticNameOfClass())
            throw Exception("Wrong data type given to getOutputData in ProcessObject " + getNameOfClass() + " \n" +
                    "Required: " + mOutputPortClass[outputNumber] + " Given: " + DataType::getStaticNameOfClass());
    }
    */

    return data;
}

template <class DataType>
DataObject::pointer ProcessObject::getOutputData() {
    return getOutputData<DataType>(0);
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticInputData(uint inputNumber) const {
    if(!inputPortExists(inputNumber))
        throw Exception("The input port " + boost::lexical_cast<std::string>(inputNumber) + " does not exist on the ProcessObject " + getNameOfClass());

    // at throws exception if element not found, while [] does not
    ProcessObjectPort port = mInputConnections.at(inputNumber);
    DataObject::pointer data = port.getData();
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        if(mInputPortType.at(inputNumber) == INPUT_STATIC)
            throw Exception("Input " + boost::lexical_cast<std::string>(inputNumber) + " given to " + getNameOfClass() + " was dynamic while static was required.");
        returnData = typename DynamicData::pointer(data)->getNextFrame(mPtr);
    } else {
        if(mInputPortType.at(inputNumber) == INPUT_DYNAMIC)
            throw Exception("Input " + boost::lexical_cast<std::string>(inputNumber) + " given to " + getNameOfClass() + " was static while dynamic was required.");
        returnData = data;
    }

    // Try to do conversion
    try {
        // Try to cast the input data to the requested DataType
        typename DataType::pointer convertedReturnData = returnData;
        return convertedReturnData;
    } catch(Exception &e) {
        // Illegal cast means wrong input data was given to the class
        throw Exception("Wrong input data given to " + getNameOfClass() + " \n" +
                "Required: " + mInputPortClass.at(inputNumber) + " Requested: " + DataType::getStaticNameOfClass() + " Given: " + returnData->getNameOfClass());
    }
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticInputData() const {
    return getStaticInputData<DataType>(0);
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticOutputData(uint outputNumber) {
    if(!outputPortExists(outputNumber))
        throw Exception("The output port " + boost::lexical_cast<std::string>(outputNumber) + " does not exist on the ProcessObject " + getNameOfClass());

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

    // Try to do conversion
    try {
        // Try to cast the input data to the requested DataType
        typename DataType::pointer convertedReturnData = returnData;
        return convertedReturnData;
    } catch(Exception &e) {
        // Illegal cast means wrong input data was given to the class
        throw Exception("Wrong input data given to " + getNameOfClass() + " \n" +
                "Required: " + mOutputPortClass[outputNumber] + " Requested: " + DataType::getStaticNameOfClass() + " Given: " + returnData->getNameOfClass());
    }

    return returnData;
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticOutputData() {
    return getStaticOutputData<DataType>(0);
}

template <class DataType>
void ProcessObject::setStaticOutputData(uint portID, DataObject::pointer staticData) {

    if(!outputPortExists(portID)) {
        throw Exception("Output port " + boost::lexical_cast<std::string>(portID) + " does not exist on process object " + getNameOfClass());
    }

    // Do type checking to see that supplied staticData is of required type
    // Try to do conversion
    typename DataType::pointer convertedStaticData;
    try {
        // Try to cast the input data to the requested DataType
        convertedStaticData = staticData;
    } catch(Exception &e) {
        // Illegal cast means wrong input data was given to the class
        throw Exception("Wrong output data given to " + getNameOfClass() + " \n" +
                "Required: " + mOutputPortClass[portID] + " Requested: " + DataType::getStaticNameOfClass() + " Given: " + staticData->getNameOfClass());
    }

    bool isDynamicData = false;

    // If output data is not created, check if it is dynamic, if it is, create the dynamic data object
    if(mOutputData.count(portID) == 0) {
        // Is output dependent on any input?
        if(mOutputDynamicDependsOnInput.count(portID) > 0) {
            uint inputNumber = mOutputDynamicDependsOnInput[portID];
            if(mInputConnections.count(inputNumber) == 0)
                throw Exception("Must call input before output.");
            ProcessObjectPort port = mInputConnections[inputNumber];
            DataObject::pointer objectDependsOn = port.getData();
            if(objectDependsOn->isDynamicData()) {
                // Create dynamic data
                DataObject::pointer data;
                data = DynamicData::New();
                data->setStreamer(objectDependsOn->getStreamer());
                mOutputData[portID] = data;
                isDynamicData = true;
            }
        } else if(mOutputPortType[portID] == OUTPUT_DYNAMIC) {
            // Create dynamic data
            mOutputData[portID] = DynamicData::New();
            isDynamicData = true;
        }
    } else {
        if(mOutputData[portID]->isDynamicData()) {
            isDynamicData = true;
        }
    }

    if(isDynamicData) {
        DynamicData::pointer(mOutputData[portID])->addFrame(convertedStaticData);
    } else {
        mOutputData[portID] = convertedStaticData;
        mOutputData[portID]->updateModifiedTimestamp();
    }
}

template <class DataType>
void ProcessObject::createInputPort(uint portID, bool required, InputDataType inputDataType) {
    if(inputPortExists(portID))
        throw Exception("Input port with ID " + boost::lexical_cast<std::string>(portID) + " already exist on " + getNameOfClass() );

    mRequiredInputs[portID] = required;
    mInputPortClass[portID] = DataType::getStaticNameOfClass();
    mInputPortType[portID] = inputDataType;
}

template <class DataType>
void ProcessObject::createOutputPort(uint portID, OutputDataType outputDataType, int inputPortID) {
    if(outputPortExists(portID))
        throw Exception("Output port with ID " + boost::lexical_cast<std::string>(portID) + " already exist on " + getNameOfClass() );

    mOutputPortClass[portID] = DataType::getStaticNameOfClass();
    mOutputPortType[portID] = outputDataType;
    if(outputDataType == OUTPUT_DEPENDS_ON_INPUT) {
        if(inputPortID < 0) {
            throw Exception("Output was set to depend on an input port, but no valid inputPortID was given to " + getNameOfClass() );
        }
        setOutputDataDynamicDependsOnInputData(portID, inputPortID);
    }
}

}; // end namespace fast

#endif
