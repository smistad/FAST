#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "FAST/SmartPointers.hpp"
#include "FAST/Utility.hpp"
#include <unordered_map>
#include <vector>
#include "FAST/Object.hpp"
#include "FAST/Data/DataObject.hpp"
#include "RuntimeMeasurement.hpp"
#include "RuntimeMeasurementManager.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/DynamicData.hpp"
#include "FAST/Config.hpp"
#include "FAST/Attribute.hpp"

namespace fast{
    class ProcessObjectPort;
}
namespace std {
    template <>
    class hash<fast::ProcessObjectPort>{
    public:
        size_t operator()(const fast::ProcessObjectPort &object) const;
    };
};
namespace fast {


enum InputDataType { INPUT_STATIC, INPUT_DYNAMIC, INPUT_STATIC_OR_DYNAMIC };
enum OutputDataType { OUTPUT_STATIC, OUTPUT_DYNAMIC, OUTPUT_DEPENDS_ON_INPUT };

class OpenCLProgram;
class ProcessObject;

class FAST_EXPORT  ProcessObjectPort {
    public:
        ProcessObjectPort(uint portID, SharedPointer<ProcessObject> processObject);
        ProcessObjectPort() {};
        DataObject::pointer getData();
        std::vector<DataObject::pointer> getMultipleData();
        uint getPortID() const;
        SharedPointer<ProcessObject> getProcessObject() const;
        bool isDataModified() const;
        void updateTimestamp();
        bool operator==(const ProcessObjectPort &other) const;
    private:
        uint mPortID;
        SharedPointer<ProcessObject> mProcessObject;
        unsigned long mTimestamp;
        std::size_t mDataPointer;
};

class FAST_EXPORT  ProcessObject : public virtual Object {
    public:
        virtual ~ProcessObject();
        void update();
        typedef SharedPointer<ProcessObject> pointer;

        // Runtime stuff
        RuntimeMeasurement::pointer getRuntime();
        RuntimeMeasurement::pointer getRuntime(std::string name);
        RuntimeMeasurementsManager::pointer getAllRuntimes();
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
        void createInputPort(uint portID, bool required = true, InputDataType = INPUT_STATIC_OR_DYNAMIC, bool allowMultipleData = false);
        template <class DataType>
        void createOutputPort(uint portID, OutputDataType, int inputPortID = -1, bool enableMultipleData = false);

        void setOutputData(uint outputNumber, DataObject::pointer object);
        void setOutputData(uint outputNumber, std::vector<DataObject::pointer> data);

        // New pipeline methods
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
        ProcessObjectPort getInputPort(uint portID) const;
        void setInputData(uint portID, DataObject::pointer);
        void setInputData(DataObject::pointer);
        void setInputData(uint portID, std::vector<DataObject::pointer>);
        void setInputData(std::vector<DataObject::pointer>);
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
         * This methods returns a vector of all static input data for a given port.
         */
        template <class DataType>
        std::vector<typename DataType::pointer> getMultipleStaticInputData(uint inputNumber) const;
        /**
         * This methods returns a vector of all static input data for port 0.
         */
        template <class DataType>
        std::vector<typename DataType::pointer> getMultipleStaticInputData() const;

        /**
         * This method returns static data always. So if the output is dynamic data it will create a new frame and return that.
         */
        template <class DataType>
        typename DataType::pointer getStaticOutputData(uint portID);
        template <class DataType>
        typename DataType::pointer getStaticOutputData();

        /**
         * This methods creates a new static output data and adds to the list of output data for the given port.
         */
        template <class DataType>
        typename DataType::pointer addStaticOutputData(uint portID);
        /**
         * This methods creates a new static output data and adds to the list of output data for port 0.
         */
        template <class DataType>
        typename DataType::pointer addStaticOutputData();

        template <class DataType>
        DataObject::pointer getOutputData(uint portID);
        template <class DataType>
        DataObject::pointer getOutputData();
        template <class DataType>
        std::vector<typename DataType::pointer> getMultipleOutputData(uint portID);
        template <class DataType>
        std::vector<typename DataType::pointer> getMultipleOutputData();

        DynamicData::pointer getDynamicOutputData(uint portID);
        DynamicData::pointer getDynamicOutputData();

        bool inputPortExists(uint portID) const;
        bool outputPortExists(uint portID) const;
        virtual std::string getNameOfClass() const = 0;
        static std::string getStaticNameOfClass() {
            return "ProcessObject";
        }
        virtual void loadAttributes();
        std::shared_ptr<Attribute> getAttribute(std::string id);
        std::unordered_map<std::string, std::shared_ptr<Attribute>> getAttributes();
        void setAttributes(std::vector<std::shared_ptr<Attribute>> attributes);
    protected:
        ProcessObject();
        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

        virtual void waitToFinish() {};

        RuntimeMeasurementsManager::pointer mRuntimeManager;

        void setInputRequired(uint portID, bool required);
        void releaseInputAfterExecute(uint inputNumber, bool release);

        void setOutputDataDynamicDependsOnInputData(uint outputNumber, uint inputNumber);
        uint getNrOfInputData() const;
        uint getNrOfOutputPorts() const;
        DataObject::pointer getInputData(uint inputNumber) const;

        template <class T>
        void setStaticOutputData(uint portID, DataObject::pointer data);

        void createOpenCLProgram(std::string sourceFilename, std::string name = "");
        cl::Program getOpenCLProgram(
                SharedPointer<OpenCLDevice> device,
                std::string name = "",
                std::string buildOptions = ""
        );

        void createFloatAttribute(std::string id, std::string name, std::string description, float initialValue);
        void createIntegerAttribute(std::string id, std::string name, std::string description, int initialValue);
        void createBooleanAttribute(std::string id, std::string name, std::string description, bool initialValue);
        void createStringAttribute(std::string id, std::string name, std::string description, std::string initialValue);
        float getFloatAttribute(std::string id);
        int getIntegerAttribute(std::string id);
        bool getBooleanAttribute(std::string id);
        std::string getStringAttribute(std::string id);
        std::vector<float> getFloatListAttribute(std::string id);
        std::vector<int> getIntegerListAttribute(std::string id);
        std::vector<bool> getBooleanListAttribute(std::string id);
        std::vector<std::string> getStringListAttribute(std::string id);
    private:
        void updateTimestamp(DataObject::pointer data);
        void changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device);
        void preExecute();
        void postExecute();
        // This fetches output data without creating it
        DataObject::pointer getOutputDataX(uint portID) const;
        std::vector<DataObject::pointer> getMultipleOutputDataX(uint portID) const;

        std::unordered_map<uint, bool> mRequiredInputs;
        std::unordered_map<uint, bool> mReleaseAfterExecute;
        std::unordered_map<uint, std::vector<uint> > mInputDevices;
        std::unordered_map<uint, ExecutionDevice::pointer> mDevices;
        std::unordered_map<uint, uint> mOutputDynamicDependsOnInput;
        std::unordered_map<uint, DeviceCriteria> mDeviceCriteria;

        // New pipeline
        std::unordered_map<uint, ProcessObjectPort> mInputConnections;
        std::unordered_map<uint, std::vector<DataObject::pointer> > mOutputData;
        // Contains a string of required port class
        std::unordered_map<uint, std::string> mInputPortClass;
        std::unordered_map<uint, std::string> mOutputPortClass;
        // Whether the ports accept dynamic, static or any of the two
        std::unordered_map<uint, InputDataType> mInputPortType;
        std::unordered_map<uint, OutputDataType> mOutputPortType;
        // Whether the ports accept multiple data
        std::unordered_map<uint, bool> mInputPortMultipleData;
        std::unordered_map<uint, bool> mOutputPortMultipleData;

        std::unordered_map<std::string, SharedPointer<OpenCLProgram> > mOpenCLPrograms;

        std::unordered_map<std::string, std::shared_ptr<Attribute>> mAttributes;

        friend class DynamicData;
        friend class ProcessObjectPort;
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
            mOutputData[outputNumber].push_back(data);
        } else if(mOutputPortType[outputNumber] == OUTPUT_STATIC) {
            // Create static data
            data = DataType::New();
            mOutputData[outputNumber].push_back(data);
        } else {
            // Create dynamic data
            data = DynamicData::New();
            mOutputData[outputNumber].push_back(data);
        }
    } else {
        data = mOutputData[outputNumber][0];
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
        throw Exception("The input port " + std::to_string(inputNumber) + " does not exist on the ProcessObject " + getNameOfClass());

    // at throws exception if element not found, while [] does not
    ProcessObjectPort port = mInputConnections.at(inputNumber);
    DataObject::pointer data = port.getData();
    DataObject::pointer returnData;
    if(data->isDynamicData()) {
        if(mInputPortType.at(inputNumber) == INPUT_STATIC)
            throw Exception("Input " + std::to_string(inputNumber) + " given to " + getNameOfClass() + " was dynamic while static was required.");
        returnData = typename DynamicData::pointer(data)->getNextFrame(mPtr);
    } else {
        if(mInputPortType.at(inputNumber) == INPUT_DYNAMIC)
            throw Exception("Input " + std::to_string(inputNumber) + " given to " + getNameOfClass() + " was static while dynamic was required.");
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
std::vector<typename DataType::pointer> ProcessObject::getMultipleStaticInputData(uint inputNumber) const {
    if(!inputPortExists(inputNumber))
        throw Exception("The input port " + std::to_string(inputNumber) + " does not exist on the ProcessObject " + getNameOfClass());

    // at throws exception if element not found, while [] does not
    ProcessObjectPort port = mInputConnections.at(inputNumber);
    std::vector<DataObject::pointer> data = port.getMultipleData();
    std::vector<DataObject::pointer> returnData;

	if(mInputPortType.at(inputNumber) == INPUT_DYNAMIC)
		throw Exception("Input " + std::to_string(inputNumber) + " given to " + getNameOfClass() + " was static while dynamic was required.");
	returnData = data;

    // Try to do conversion
    try {
		std::vector<typename DataType::pointer> convertedReturnData;
        // Try to cast the input data to the requested DataType
		for(DataObject::pointer asd : returnData) {
			typename DataType::pointer asd2 = asd;
			convertedReturnData.push_back(asd2);
		}
        return convertedReturnData;
    } catch(Exception &e) {
        // Illegal cast means wrong input data was given to the class
        throw Exception("Wrong input data given to " + getNameOfClass() + " \n" +
                "Required: " + mInputPortClass.at(inputNumber) + " Requested: " + DataType::getStaticNameOfClass());
    }
}

template <class DataType>
std::vector<typename DataType::pointer> ProcessObject::getMultipleStaticInputData() const {
	return getMultipleStaticInputData<DataType>(0);
}

template <class DataType>
typename DataType::pointer ProcessObject::getStaticOutputData(uint outputNumber) {
    if(!outputPortExists(outputNumber))
        throw Exception("The output port " + std::to_string(outputNumber) + " does not exist on the ProcessObject " + getNameOfClass());

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
typename DataType::pointer ProcessObject::addStaticOutputData(uint portID) {
	if(!outputPortExists(portID))
        throw Exception("The output port " + std::to_string(portID) + " does not exist on the ProcessObject " + getNameOfClass());

	DataObject::pointer returnData = DataType::New();
	mOutputData[portID].push_back(returnData);
	return returnData;
}

template <class DataType>
std::vector<typename DataType::pointer> ProcessObject::getMultipleOutputData(uint outputNumber) {
	std::vector<typename DataType::pointer> data;

    // If output data is not created
    if(mOutputData.count(outputNumber) == 0) {
        if(mOutputPortType[outputNumber] != OUTPUT_DYNAMIC) {
            // Create static data
            //mOutputData[outputNumber].push_back(DataType::New());
        } else {
        	throw Exception("Dynamic multiple data not supported yet.");
        }
    } else {
        for(DataObject::pointer unconvertedData : mOutputData[outputNumber])
            data.push_back(typename DataType::pointer(unconvertedData)); // convert from DataObject to DataType
    }

    return data;
}
template <class DataType>
std::vector<typename DataType::pointer> ProcessObject::getMultipleOutputData() {
	return getMultipleOutputData<DataType>(0);

}

template <class DataType>
typename DataType::pointer ProcessObject::addStaticOutputData() {
    return addStaticOutputData<DataType>(0);
}

template <class DataType>
void ProcessObject::setStaticOutputData(uint portID, DataObject::pointer staticData) {

    if(!outputPortExists(portID)) {
        throw Exception("Output port " + std::to_string(portID) + " does not exist on process object " + getNameOfClass());
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
                mOutputData[portID].push_back(data);
                isDynamicData = true;
            }
        } else if(mOutputPortType[portID] == OUTPUT_DYNAMIC) {
            // Create dynamic data
            mOutputData[portID].push_back(DynamicData::New());
            isDynamicData = true;
        }
    } else {
        if(mOutputData[portID][0]->isDynamicData()) {
            isDynamicData = true;
        }
    }

    if(isDynamicData) {
        DynamicData::pointer(mOutputData[portID][0])->addFrame(convertedStaticData);
    } else {
    	if(mOutputData[portID].size() == 0) {
			mOutputData[portID].push_back(convertedStaticData);
    	} else {
			mOutputData[portID][0] = convertedStaticData;
    	}
        mOutputData[portID][0]->updateModifiedTimestamp();
    }
}

template <class DataType>
void ProcessObject::createInputPort(uint portID, bool required, InputDataType inputDataType, bool allowMultipleInputData) {
    if(inputPortExists(portID))
        reportWarning() << "Overriding input port with ID " + std::to_string(portID) + " on " + getNameOfClass() << reportEnd();

    mRequiredInputs[portID] = required;
    mInputPortClass[portID] = DataType::getStaticNameOfClass();
    mInputPortType[portID] = inputDataType;
    mInputPortMultipleData[portID] = allowMultipleInputData;
}

template <class DataType>
void ProcessObject::createOutputPort(uint portID, OutputDataType outputDataType, int inputPortID, bool enableMultipleOutputData) {
    if(outputPortExists(portID))
        reportWarning() << "Overriding output port with ID " + std::to_string(portID) + " on " + getNameOfClass() << reportEnd();

    mOutputPortClass[portID] = DataType::getStaticNameOfClass();
    mOutputPortType[portID] = outputDataType;
    mOutputPortMultipleData[portID] = enableMultipleOutputData;
    if(outputDataType == OUTPUT_DEPENDS_ON_INPUT) {
        if(inputPortID < 0) {
            throw Exception("Output was set to depend on an input port, but no valid inputPortID was given to " + getNameOfClass() );
        }
        setOutputDataDynamicDependsOnInputData(portID, inputPortID);
    }
}



}; // end namespace fast


#endif
