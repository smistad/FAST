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
         * @brief Update/Run the pipeline up to this process object.
         *
         * Do update on this PO, which will trigger update on all connected POs
         * thus running the entire pipeline.
         *
         * An optional executeToken can be used to synchronize updating to avoid
         * duplicate execution for the same frames when using streaming.
         * Increment the token for every timestep with a positive value.
         *
         * @param executeToken Negative value means that the execute token is disabled.
         */
        void update(int executeToken = -1);
        typedef std::shared_ptr<ProcessObject> pointer;

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
        /**
         * Get current output data for a given port
         * @param portID
         * @return
         */
        DataObject::pointer getOutputData(uint portID = 0);
        /**
         * Get current output data for a given port
         * @tparam DataType data type to convert to
         * @param portID
         * @return
         */
        template <class DataType>
        std::shared_ptr<DataType> getOutputData(uint portID = 0);
        int getNrOfInputConnections() const;
        int getNrOfOutputPorts() const;

        virtual std::string getNameOfClass() const = 0;
        static std::string getStaticNameOfClass() {
            return "ProcessObject";
        }
        virtual void loadAttributes();
        std::shared_ptr<Attribute> getAttribute(std::string id);
        std::unordered_map<std::string, std::shared_ptr<Attribute>> getAttributes();
        void setAttributes(std::vector<std::shared_ptr<Attribute>> attributes);

        /**
         * @brief Stop a pipeline.
         */
        void stopPipeline();

        /**
         * @brief Mark this process object as modified or not.
         * A modified PO will execute next time it is updated.
         *
         * @param modified
         */
        void setModified(bool modified);

        template <class DataType>
        std::shared_ptr<DataType> updateAndGetOutputData(uint portID = 0);

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

        void createInputPort(uint portID, std::string name = "", std::string description = "", bool required = true);
        void createOutputPort(uint portID, std::string name = "", std::string description = "");
        template <class DataType>
        [[deprecated("Replaced by non templated createInputPort(uint portID, std::string name, std::string description, bool required)")]]
        void createInputPort(uint portID, bool required = true);
        template <class DataType>
        [[deprecated("Replaced by non templated createOutputPort(uint portID, std::string name, std::string description)")]]
        void createOutputPort(uint portID);

        template <class DataType>
        std::shared_ptr<DataType> getInputData(uint portID = 0);
        void addOutputData(DataObject::pointer data, bool propagateLastFrameData = true, bool propagateFrameData = true);
        void addOutputData(uint portID, DataObject::pointer data, bool propagateLastFrameData = true, bool propagateFrameData = true);

        bool hasNewInputData(uint portID);

        virtual void waitToFinish() {};


        RuntimeMeasurementsManager::pointer mRuntimeManager;

        void createOpenCLProgram(std::string sourceFilename, std::string name = "");
        cl::Program getOpenCLProgram(
                std::shared_ptr<OpenCLDevice> device,
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
        struct InputPort {
            std::string name;
            std::string description;
            bool required = true;
        };
        std::unordered_map<uint, InputPort> mInputPorts;
        struct OutputPort {
            std::string name;
            std::string description;
            DataObject::pointer currentData = nullptr;
        };
        std::unordered_map<uint, OutputPort> mOutputPorts;
        // <port id, timestep>, register the last timestep of input data which this PO executed with
        std::unordered_map<uint, std::pair<DataObject::pointer, uint64_t>> mLastProcessed;

        void validateInputPortExists(uint portID);
        void validateOutputPortExists(uint portID);



        std::unordered_map<std::string, std::shared_ptr<OpenCLProgram> > mOpenCLPrograms;

        std::unordered_map<std::string, std::shared_ptr<Attribute>> mAttributes;

        // Frame data
        // Similar to metadata, only this is transferred from input to output
        std::unordered_map<std::string, std::string> m_frameData;
        // Indicates whether this data object is the last frame in a stream, and if so, the name of the stream
        std::unordered_set<std::string> m_lastFrame;

        int m_maximumNrOfFrames = -1;

};

template<class DataType>
void ProcessObject::createInputPort(uint portID, bool required) {
    InputPort inputPort;
    inputPort.required = required;
    mInputPorts[portID] = inputPort;
}

template<class DataType>
void ProcessObject::createOutputPort(uint portID) {
    OutputPort outputPort;
    mOutputPorts[portID] = outputPort;
}

template<class DataType>
std::shared_ptr<DataType> ProcessObject::getInputData(uint portID) {
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
std::shared_ptr<DataType> ProcessObject::getOutputData(uint portID) {
    auto data = getOutputData(portID);
    auto convertedData = std::dynamic_pointer_cast<DataType>(data);
    // Check if the conversion went ok
    if(!convertedData)
        throw BadCastException(data->getNameOfClass(), DataType::getStaticNameOfClass());

    return convertedData;
}

template<class DataType>
std::shared_ptr<DataType> ProcessObject::updateAndGetOutputData(uint portID) {
    auto port = getOutputPort(portID);
    update();
    return port->getNextFrame<DataType>();
}


}; // end namespace fast
