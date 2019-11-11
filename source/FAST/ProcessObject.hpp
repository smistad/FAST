#pragma once

#include "FAST/Utility.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "FAST/Object.hpp"
#include "FAST/Data/DataObject.hpp"
#include "RuntimeMeasurement.hpp"
#include "RuntimeMeasurementManager.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Config.hpp"
#include "FAST/Attribute.hpp"
#include "FAST/DataChannels/DataChannel.hpp"

namespace fast {

class OpenCLProgram;
class ProcessObject;

class FAST_EXPORT  ProcessObject : public Object {
    public:
        virtual ~ProcessObject();
        /**
         * Do update on this PO, which will trigger update on all connected POs.
         * An optional executeToken can be used to synchronize updating to avoid
         * duplicate execution for the same frames when using streaming.
         * Increment the token for every timestep with a positive value.
         *
         * @param executeToken Negative value means that the execute token is disabled.
         */
        void update(int executeToken = -1);
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

        virtual DataChannel::pointer getOutputPort(uint portID = 0);
        virtual DataChannel::pointer getInputPort(uint portID = 0);
        virtual void setInputConnection(DataChannel::pointer port);
        virtual void setInputConnection(uint portID, DataChannel::pointer port);
        virtual void setInputData(DataObject::pointer data);
        virtual void setInputData(uint portID, DataObject::pointer data);
        int getNrOfInputConnections() const;
        int getNrOfOutputPorts() const;

        virtual std::string getNameOfClass() const = 0;
        static std::string getStaticNameOfClass() {
            return "ProcessObject";
        }
        virtual void loadAttributes();
        SharedPointer<Attribute> getAttribute(std::string id);
        std::unordered_map<std::string, SharedPointer<Attribute>> getAttributes();
        void setAttributes(std::vector<SharedPointer<Attribute>> attributes);

        /**
         * Used to stop a pipeline.
         */
        void stopPipeline();

        void setModified(bool modified);

        template <class DataType>
        SharedPointer<DataType> updateAndGetOutputData(uint portID = 0);

    protected:
        ProcessObject();
        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // An integer id which act as a token of when this PO last executed
        int m_lastExecuteToken = -1;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;
        virtual void preExecute();
        virtual void postExecute();

        template <class DataType>
        void createInputPort(uint portID, bool required = true);
        template <class DataType>
        void createOutputPort(uint portID);

        template <class DataType>
        SharedPointer<DataType> getInputData(uint portID = 0);
        template <class DataType>
        SharedPointer<DataType> getOutputData(uint portID = 0);
        void addOutputData(uint portID, DataObject::pointer data);

        bool hasNewInputData(uint portID);

        virtual void waitToFinish() {};


        RuntimeMeasurementsManager::pointer mRuntimeManager;

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

        void changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device);

        std::unordered_map<uint, bool> mRequiredInputs;
        std::unordered_map<uint, std::vector<uint> > mInputDevices;
        std::unordered_map<uint, ExecutionDevice::pointer> mDevices;
        std::unordered_map<uint, DeviceCriteria> mDeviceCriteria;

        // New pipeline
        std::unordered_map<uint, DataChannel::pointer> mInputConnections;
        std::unordered_map<uint, std::vector<std::weak_ptr<DataChannel>>> mOutputConnections;
        std::unordered_map<uint, bool> mInputPorts;
        std::unordered_set<uint> mOutputPorts;
        // <port id, timestep>, register the last timestep of data which this PO executed with
        std::unordered_map<uint, std::pair<DataObject::pointer, uint64_t>> mLastProcessed;

        void validateInputPortExists(uint portID);
        void validateOutputPortExists(uint portID);



        std::unordered_map<std::string, SharedPointer<OpenCLProgram> > mOpenCLPrograms;

        std::unordered_map<std::string, SharedPointer<Attribute>> mAttributes;

        // Frame data
        // Similar to metadata, only this is transferred from input to output
        std::unordered_map<std::string, std::string> m_frameData;
        // Indicates whether this data object is the last frame in a stream, and if so, the name of the stream
        std::unordered_set<std::string> m_lastFrame;


};


template<class DataType>
void ProcessObject::createInputPort(uint portID, bool required) {
    mInputPorts[portID] = required;
}


template<class DataType>
void ProcessObject::createOutputPort(uint portID) {
    mOutputPorts.insert(portID);
}

template<class DataType>
SharedPointer<DataType> ProcessObject::getInputData(uint portID) {
    validateInputPortExists(portID);
    DataChannel::pointer port = mInputConnections.at(portID);
    DataObject::pointer data = port->getNextFrame();
    mLastProcessed[portID] = std::make_pair(data, data->getTimestamp());
    auto convertedData = std::dynamic_pointer_cast<DataType>(data);
    // Check if the conversion went ok
    if(!convertedData)
        throw BadCastException(data->getNameOfClass(), DataType::getStaticNameOfClass());

    // Store frame data for this input data so it can be added to output data later
    for(auto&& lastFrame : data->getLastFrame())
        m_lastFrame.insert(lastFrame);
    for(auto&& frameData : data->getFrameData())
        m_frameData[frameData.first] = frameData.second;

    return convertedData;
}

template<class DataType>
SharedPointer<DataType> ProcessObject::getOutputData(uint portID) {
    validateOutputPortExists(portID);
    // Generate a new output data object
    SharedPointer<DataType> returnData = DataType::New();

    addOutputData(portID, returnData);

    // Return it
    return returnData;
}


template<class DataType>
SharedPointer<DataType> ProcessObject::updateAndGetOutputData(uint portID) {
    auto port = getOutputPort(portID);
    update();
    return port->getNextFrame<DataType>();
}


}; // end namespace fast
