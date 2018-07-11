#include "FAST/ProcessObject.hpp"
#include "FAST/Exception.hpp"
#include "FAST/OpenCLProgram.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include <unordered_set>


namespace fast {

ProcessObject::ProcessObject() : mIsModified(false) {
    mDevices[0] = DeviceManager::getInstance()->getDefaultComputationDevice();
    mRuntimeManager = RuntimeMeasurementsManager::New();
}

static bool isStreamer(ProcessObject* po) {
    Streamer* derived_ptr = dynamic_cast<Streamer *>(po);
    bool isStreamer = true;
    if(derived_ptr == nullptr)
        isStreamer = false;

    return isStreamer;
}

void ProcessObject::update(uint64_t timestep, StreamingMode streamingMode) {
    // Call update on all parents
    bool newInputData = false;
    for(auto parent : mInputConnections) {
        DataPort::pointer port = parent.second;
        port->setTimestep(timestep);
        port->setStreamingMode(streamingMode);
        port->getProcessObject()->update(timestep, streamingMode);

        if(mLastProcessed.count(parent.first) > 0) {
            // Compare the last processed data with the new data for this data port
            std::pair<DataObject::pointer, uint64_t> data = mLastProcessed[parent.first];
            if(port->hasCurrentData()) {
                DataObject::pointer previousData = data.first;
                uint64_t previousTimestamp = data.second;
                DataObject::pointer currentData = port->getFrame(timestep);
                uint64_t currentTimestamp = currentData->getTimestamp();
                if(currentData != previousData || previousTimestamp < currentTimestamp) { // There has arrived new data, or data has changed
                    newInputData = true;
                }
            } else {
                // If port currently doesn't have data, check if parent is streamer. If streamer, always execute. PO will block until data arrives
                if(isStreamer(port->getProcessObject().get())) {
                    newInputData = true;
                } else {
                    // TODO should not be possible?
                    reportError() << "Impossible event in ProcessObject::update" << reportEnd();
                }
            }
        } else {
            // First time executing, always execute in this case
            newInputData = true;
        }
    }

    // Set timestep and streaming mode for output connections
    // Also remove dead output ports if any
    for(auto&& outputPorts : mOutputConnections) {
        std::vector<int> deadOutputPorts;
        for(int i = 0; i < outputPorts.second.size(); ++i) {
            auto output = outputPorts.second[i];
            if(!output.expired()) {
                DataPort::pointer port = output.lock();
                port->setTimestep(timestep);
                port->setStreamingMode(streamingMode);
            } else {
                deadOutputPorts.push_back(i);
            }
        }
        for(auto deadLink : deadOutputPorts)
            outputPorts.second.erase(outputPorts.second.begin() + deadLink);
    }

    // If this object is modified, or any parents has new data for this PO: Call execute
    if(mIsModified || (newInputData && mLastTimestepExecuted != timestep)) {
        this->mRuntimeManager->startRegularTimer("execute");
        // set isModified to false before executing to avoid recursive update calls
        reportInfo() << "EXECUTING " << getNameOfClass() << " because " << reportEnd();
        if(mIsModified) {
            reportInfo() << "PO is modified." << reportEnd();
        } else if(newInputData) {
            reportInfo() << "has new input data." << reportEnd();
        }
        mIsModified = false;
        preExecute();
        execute();
        postExecute();
        mLastTimestepExecuted = timestep;
        if(this->mRuntimeManager->isEnabled())
            this->waitToFinish();
        this->mRuntimeManager->stopRegularTimer("execute");
    } else if(!isStreamer(this)) {
        // If this object did not need to execute AND is not a streamer. Move the output data to next timestep.
        for(auto outputPorts : mOutputConnections) {
            for(auto output : outputPorts.second) {
                DataPort::pointer port = output.lock();
                port->moveDataToNextTimestep();
            }
        }
    }
}

DataPort::pointer ProcessObject::getOutputPort(uint portID) {
    validateOutputPortExists(portID);
    // Create DataPort, and it to list and return it
    DataPort::pointer port = std::make_shared<DataPort>(std::static_pointer_cast<ProcessObject>(mPtr.lock()));

    if(mOutputConnections.count(portID) == 0)
        mOutputConnections[portID] = std::vector<std::weak_ptr<DataPort>>();

    mOutputConnections[portID].push_back(std::weak_ptr<DataPort>(port));

    return port;
}

DataPort::pointer ProcessObject::getInputPort(uint portID) {
    return mInputConnections.at(portID);
}

void ProcessObject::setInputConnection(uint portID, DataPort::pointer port) {
    validateInputPortExists(portID);;
    if(port->getProcessObject().get() == this)
        throw Exception("Can't set setInputConnection on self");
    mInputConnections[portID] = port;
    mIsModified = true;
}

void ProcessObject::setInputConnection(DataPort::pointer port) {
    setInputConnection(0, port);
}

void ProcessObject::addOutputData(uint portID, DataObject::pointer data) {
    validateOutputPortExists(portID);

    // Add it to all output connections, if any connections exist
    if(mOutputConnections.count(portID) > 0) {
        for(auto output : mOutputConnections.at(portID)) {
            if(!output.expired()) {
                DataPort::pointer port = output.lock();
                port->addFrame(data);
            }
        }
    }
}

void ProcessObject::setInputData(DataObject::pointer data) {
    setInputData(0, data);
}

class EmptyProcessObject : public ProcessObject {
    FAST_OBJECT(EmptyProcessObject)
    public:
        void setOutputData(DataObject::pointer data) {
            mData = data;
            mIsModified = true;
        };
    private:
        EmptyProcessObject() {
            createOutputPort<DataObject>(0);
        };
        void execute() {
            addOutputData(0, mData);
        };
        DataObject::pointer mData;
};

void ProcessObject::setInputData(uint portID, DataObject::pointer data) {
    validateInputPortExists(portID);
    EmptyProcessObject::pointer PO = EmptyProcessObject::New();
    PO->setOutputData(data);
    setInputConnection(portID, PO->getOutputPort());
    mIsModified = true;
}

void ProcessObject::preExecute() {
    // Validate that all required input connections have been set
    for(auto input : mInputPorts) {
        if(input.second) { // if required
            if(mInputConnections.count(input.first) == 0) {
                throw Exception("Input port " + std::to_string(input.first) + " on process object " + getNameOfClass() + " is missing its required connection.");
            }
        }
    }
}

void ProcessObject::validateInputPortExists(uint portID) {
    if(mInputPorts.count(portID) == 0)
        throw Exception(getNameOfClass() + " has no input port with ID " + std::to_string(portID));
}

void ProcessObject::validateOutputPortExists(uint portID) {
    if(mOutputPorts.count(portID) == 0)
        throw Exception(getNameOfClass() + " has no output port with ID " + std::to_string(portID));
}

void ProcessObject::enableRuntimeMeasurements() {
    mRuntimeManager->enable();
}

void ProcessObject::disableRuntimeMeasurements() {
    mRuntimeManager->disable();
}

RuntimeMeasurement::pointer ProcessObject::getRuntime() {
    return mRuntimeManager->getTiming("execute");
}

RuntimeMeasurement::pointer ProcessObject::getRuntime(std::string name) {
    return mRuntimeManager->getTiming(name);
}

RuntimeMeasurementsManager::pointer ProcessObject::getAllRuntimes() {
    return mRuntimeManager;
}

void ProcessObject::postExecute() {
    /*
    // TODO Release input data if they are marked as "release after execute"
    std::unordered_map<uint, bool>::iterator it;
    for(it = mReleaseAfterExecute.begin(); it != mReleaseAfterExecute.end(); it++) {
        if(it->second) {
            std::vector<uint>::iterator it2;
            for(it2 = mInputDevices[it->first].begin(); it2 != mInputDevices[it->first].end(); it2++) {
                DataObject::pointer data = getInputData(it->first);
                data->release(getDevice(*it2));
            }
        }
    }
     */
}

void ProcessObject::changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device) {
    /*
    // TODO For each input, release old device and retain on new device
    std::unordered_map<uint, ProcessObjectPort>::iterator it;
    for(it = mInputConnections.begin(); it != mInputConnections.end(); it++) {
        std::vector<uint>::iterator it2;
        for(it2 = mInputDevices[it->first].begin(); it2 != mInputDevices[it->first].end(); it2++) {
            if(*it2 == deviceNumber) {
                getInputData(it->first)->release(mDevices[deviceNumber]);
                getInputData(it->first)->retain(device);
            }
        }
    }
     */
}

void ProcessObject::setMainDevice(ExecutionDevice::pointer device) {
    setDevice(0, device);
}

ExecutionDevice::pointer ProcessObject::getMainDevice() const {
    return mDevices.at(0);
}

void ProcessObject::setDevice(uint deviceNumber,
        ExecutionDevice::pointer device) {
    if(mDeviceCriteria.count(deviceNumber) > 0) {
        if(!DeviceManager::getInstance()->deviceSatisfiesCriteria(std::dynamic_pointer_cast<OpenCLDevice>(device), mDeviceCriteria[deviceNumber]))
            throw Exception("Tried to set device which does not satisfy device criteria");
    }
    if(mDevices.count(deviceNumber) > 0) {
        changeDeviceOnInputs(deviceNumber, device);
    }
    mDevices[deviceNumber] = device;
}

ExecutionDevice::pointer ProcessObject::getDevice(uint deviceNumber) const {
    return mDevices.at(deviceNumber);
}

void ProcessObject::setMainDeviceCriteria(const DeviceCriteria& criteria) {
    mDeviceCriteria[0] = criteria;
    mDevices[0] = DeviceManager::getInstance()->getDevice(criteria);
}

void ProcessObject::setDeviceCriteria(uint deviceNumber,
        const DeviceCriteria& criteria) {
    mDeviceCriteria[deviceNumber] = criteria;
    mDevices[deviceNumber] = DeviceManager::getInstance()->getDevice(criteria);
}

void ProcessObject::createOpenCLProgram(std::string sourceFilename, std::string name) {
    OpenCLProgram::pointer program = OpenCLProgram::New();
    program->setName(name);
    program->setSourceFilename(sourceFilename);
    mOpenCLPrograms[name] = program;
}

cl::Program ProcessObject::getOpenCLProgram(
        OpenCLDevice::pointer device,
        std::string name,
        std::string buildOptions
        ) {

    if(mOpenCLPrograms.count(name) == 0) {
        throw Exception("OpenCL program with the name " + name + " not found in " + getNameOfClass());
    }

    OpenCLProgram::pointer program = mOpenCLPrograms[name];
    return program->build(device, buildOptions);
}

ProcessObject::~ProcessObject() {
}

void ProcessObject::setAttributes(std::vector<SharedPointer<Attribute>> attributes) {
    for(SharedPointer<Attribute> attribute : attributes) {
        std::string name = attribute->getName();
        if(mAttributes.count(name) == 0) {
            throw Exception("Attribute " + name + " not found for process object " + getNameOfClass());
        }

        SharedPointer<Attribute> localAttribute = mAttributes.at(name);
        if(localAttribute->getType() != attribute->getType())
            throw Exception("Attribute " + name + " for process object " + getNameOfClass() + " had different type then the one loaded.");

        localAttribute->setValues(attribute->getValues());
    }
}

void ProcessObject::loadAttributes() {
    //throw Exception("The process object " + getNameOfClass() + " has not implemented the loadAttributes method and therefore cannot be loaded from fast pipeline files (.fpl).");
}

void ProcessObject::createFloatAttribute(std::string id, std::string name, std::string description, float initialValue) {
    SharedPointer<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_FLOAT);
    SharedPointer<AttributeValue> value = std::make_shared<AttributeValueFloat>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

void ProcessObject::createIntegerAttribute(std::string id, std::string name, std::string description, int initialValue) {
    SharedPointer<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_INTEGER);
    SharedPointer<AttributeValue> value = std::make_shared<AttributeValueInteger>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

void ProcessObject::createBooleanAttribute(std::string id, std::string name, std::string description, bool initialValue) {
    SharedPointer<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_BOOLEAN);
    SharedPointer<AttributeValue> value = std::make_shared<AttributeValueBoolean>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

void ProcessObject::createStringAttribute(std::string id, std::string name, std::string description, std::string initialValue) {
    SharedPointer<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_STRING);
    SharedPointer<AttributeValue> value = std::make_shared<AttributeValueString>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

SharedPointer<Attribute> ProcessObject::getAttribute(std::string id) {
    if(mAttributes.count(id) == 0)
        throw Exception("Attribute " + id + " not found for process object " + getNameOfClass() +
                                ". Did you forget to define it in the constructor?");

    return mAttributes[id];
}

float ProcessObject::getFloatAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_FLOAT)
        throw Exception("Attribute " + id + " is not of type float in process object " + getNameOfClass());

    SharedPointer<AttributeValueFloat> value = std::dynamic_pointer_cast<AttributeValueFloat>(attribute->getValue());
    return value->get();
}

std::vector<float> ProcessObject::getFloatListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_FLOAT)
        throw Exception("Attribute " + id + " is not of type float in process object " + getNameOfClass());

    std::vector<SharedPointer<AttributeValue>> values = attribute->getValues();
    std::vector<float> list;
    for(auto &&value : values) {
        auto floatValue = std::dynamic_pointer_cast<AttributeValueFloat>(value);
        list.push_back(floatValue->get());
    }
    return list;
}

int ProcessObject::getIntegerAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_INTEGER)
        throw Exception("Attribute " + id + " is not of type integer in process object " + getNameOfClass());

    SharedPointer<AttributeValueInteger> value = std::dynamic_pointer_cast<AttributeValueInteger>(attribute->getValue());
    return value->get();
}

std::vector<int> ProcessObject::getIntegerListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_INTEGER)
        throw Exception("Attribute " + id + " is not of type integer in process object " + getNameOfClass());

    std::vector<SharedPointer<AttributeValue>> values = attribute->getValues();
    std::vector<int> list;
    for(auto &&value : values) {
        auto floatValue = std::dynamic_pointer_cast<AttributeValueInteger>(value);
        list.push_back(floatValue->get());
    }
    return list;
}

bool ProcessObject::getBooleanAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_BOOLEAN)
        throw Exception("Attribute " + id + " is not of type boolean in process object " + getNameOfClass());

    SharedPointer<AttributeValueBoolean> value = std::dynamic_pointer_cast<AttributeValueBoolean>(attribute->getValue());
    return value->get();
}


std::vector<bool> ProcessObject::getBooleanListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_BOOLEAN)
        throw Exception("Attribute " + id + " is not of type boolean in process object " + getNameOfClass());

    std::vector<SharedPointer<AttributeValue>> values = attribute->getValues();
    std::vector<bool> list;
    for(auto &&value : values) {
        auto floatValue = std::dynamic_pointer_cast<AttributeValueBoolean>(value);
        list.push_back(floatValue->get());
    }
    return list;
}

std::string ProcessObject::getStringAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_STRING)
        throw Exception("Attribute " + id + " is not of type string in process object " + getNameOfClass());

    SharedPointer<AttributeValueString> value = std::dynamic_pointer_cast<AttributeValueString>(attribute->getValue());
    return value->get();
}


std::vector<std::string> ProcessObject::getStringListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_STRING)
        throw Exception("Attribute " + id + " is not of type string in process object " + getNameOfClass());

    std::vector<SharedPointer<AttributeValue>> values = attribute->getValues();
    std::vector<std::string> list;
    for(auto &&value : values) {
        auto floatValue = std::dynamic_pointer_cast<AttributeValueString>(value);
        list.push_back(floatValue->get());
    }
    return list;
}

std::unordered_map<std::string, SharedPointer<Attribute>> ProcessObject::getAttributes() {
    return mAttributes;
}

int ProcessObject::getNrOfInputConnections() const {
    return mInputConnections.size();
}

void ProcessObject::stopPipeline() {
    for(auto input : mInputConnections) {
        input.second->stop();
        input.second->getProcessObject()->stopPipeline(); // Stop parent POs
    }

}

bool ProcessObject::hasNewInputData(uint portID) {
    return mInputConnections.at(portID)->hasCurrentData();
}

int ProcessObject::getNrOfOutputPorts() const {
    return mOutputPorts.size();
}

void ProcessObject::setModified(bool modified) {
    mIsModified = modified;

}

} // namespace fast
