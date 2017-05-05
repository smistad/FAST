#include "FAST/ProcessObject.hpp"
#include "FAST/Exception.hpp"
#include "FAST/OpenCLProgram.hpp"
#include <unordered_set>


namespace fast {

ProcessObject::ProcessObject() : mIsModified(false) {
    mDevices[0] = DeviceManager::getInstance()->getDefaultComputationDevice();
    mRuntimeManager = RuntimeMeasurementsManager::New();
}

void ProcessObject::update() {
    bool aParentHasBeenModified = false;
    std::unordered_map<uint, ProcessObjectPort>::iterator it;
    // If this object has multiple connection to the same parent this is used to avoid updating that parent many times
    std::unordered_set<ProcessObject::pointer> parents;
    for(it = mInputConnections.begin(); it != mInputConnections.end(); it++) {
        if(parents.count(it->second.getProcessObject()) == 0) {
            parents.insert(it->second.getProcessObject());
        } else {
            // Skip; already updated
            continue;
        }
        // Update input connection
        ProcessObjectPort& port = it->second; // use reference here to make sure timestamp is updated
        port.getProcessObject()->update();

        // Check if the data object has been updated
        DataObject::pointer data;
        try {
            if(port.isDataModified()) {
                aParentHasBeenModified = true;
                // Update timestamp
                port.updateTimestamp();
            }
        } catch(Exception &e) {
            // Data was not found
        }
    }

    // has been modified, execute is called
    if(this->mIsModified || aParentHasBeenModified) {
        this->mRuntimeManager->startRegularTimer("execute");
        // set isModified to false before executing to avoid recursive update calls
        this->mIsModified = false;
        this->preExecute();
        this->execute();
        this->postExecute();
        if(this->mRuntimeManager->isEnabled())
            this->waitToFinish();
        this->mRuntimeManager->stopRegularTimer("execute");
    }
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

void ProcessObject::setInputRequired(uint portID, bool required) {
    mRequiredInputs[portID] = required;
}

void ProcessObject::releaseInputAfterExecute(uint inputNumber,
        bool release) {
    mReleaseAfterExecute[inputNumber] = release;
}

void ProcessObject::preExecute() {
    // Check that required inputs are present
    std::unordered_map<uint, bool>::iterator it;
    for(it = mRequiredInputs.begin(); it != mRequiredInputs.end(); it++) {
        if(it->second) { // if required
            // Check that input exist and is valid
            if(mInputConnections.count(it->first) == 0/* || !mInputConnections[it->first].isValid()*/) {
                throw Exception("A process object is missing a required input data.");
            }
        }
    }
}

void ProcessObject::postExecute() {
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
}

void ProcessObject::changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device) {
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
        if(!DeviceManager::getInstance()->deviceSatisfiesCriteria(device, mDeviceCriteria[deviceNumber]))
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

uint ProcessObject::getNrOfInputData() const {
    return mInputConnections.size();
}

uint ProcessObject::getNrOfOutputPorts() const {
    return mOutputPortType.size();
}

void ProcessObject::setOutputData(uint outputNumber, DataObject::pointer data) {
	if(mOutputData[outputNumber].size() > 0) {
		mOutputData[outputNumber][0] = data;
	} else {
		mOutputData[outputNumber].push_back(data);
	}
}

void ProcessObject::setOutputData(uint outputNumber, std::vector<DataObject::pointer> data) {
    mOutputData[outputNumber] = data;
}

void ProcessObject::setOutputDataDynamicDependsOnInputData(uint outputNumber, uint inputNumber) {
    mOutputDynamicDependsOnInput[outputNumber] = inputNumber;
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

// New pipeline
void ProcessObject::setInputConnection(ProcessObjectPort port) {
    setInputConnection(0, port);
}

void ProcessObject::setInputConnection(uint connectionID, ProcessObjectPort port) {
    mInputConnections[connectionID] = port;

    // Clear output data
    mOutputData.clear();
}

ProcessObjectPort ProcessObject::getOutputPort() {
    return getOutputPort(0);
}

ProcessObjectPort ProcessObject::getOutputPort(uint portID) {
    ProcessObjectPort port(portID, mPtr.lock());
    return port;
}

DataObject::pointer ProcessObject::getOutputDataX(uint portID) const {

    return getMultipleOutputDataX(portID)[0];
}

std::vector<DataObject::pointer> ProcessObject::getMultipleOutputDataX(uint portID) const {
    std::vector<DataObject::pointer> data;

    // If output data is not created
    if(mOutputData.count(portID) == 0) {
        throw Exception("Could not find output data for port " + std::to_string(portID) + " in " + getNameOfClass());
    } else {
        data = mOutputData.at(portID);
    }

    return data;
}

DataObject::pointer ProcessObject::getInputData(uint inputNumber) const {
    // at throws exception if element not found, while [] does not
    ProcessObjectPort port = mInputConnections.at(inputNumber);
    return port.getData();
}

void ProcessObject::setInputData(uint portID, DataObject::pointer data) {
    class EmptyProcessObject : public ProcessObject {
        FAST_OBJECT(EmptyProcessObject)
        public:
        private:
            void execute() {};
    };
    EmptyProcessObject::pointer PO = EmptyProcessObject::New();
    PO->setOutputData(0, data);
    setInputConnection(portID, PO->getOutputPort());
    mIsModified = true;
}

void ProcessObject::setInputData(std::vector<DataObject::pointer>data) {
    setInputData(0, data);
}

void ProcessObject::setInputData(uint portID, std::vector<DataObject::pointer> data) {
    class EmptyProcessObject : public ProcessObject {
        FAST_OBJECT(EmptyProcessObject)
        public:
        private:
            void execute() {};
    };
    EmptyProcessObject::pointer PO = EmptyProcessObject::New();
    PO->setOutputData(0, data);
    setInputConnection(portID, PO->getOutputPort());
    mIsModified = true;
}

void ProcessObject::setInputData(DataObject::pointer data) {
    setInputData(0, data);
}

ProcessObjectPort ProcessObject::getInputPort(uint portID) const {
    return mInputConnections.at(portID);
}


void ProcessObject::updateTimestamp(DataObject::pointer data) {
    std::unordered_map<uint, ProcessObjectPort>::iterator it;
    for(it = mInputConnections.begin(); it != mInputConnections.end(); it++) {
        ProcessObjectPort& port = it->second;
        if(port.getData() == data) {
            port.updateTimestamp();
        }
    }
}

ProcessObjectPort::ProcessObjectPort(uint portID,
        ProcessObject::pointer processObject) {
    mPortID = portID;
    mProcessObject = processObject;
    mTimestamp = 0;
    mDataPointer = 0;
}

ProcessObject::pointer ProcessObjectPort::getProcessObject() const {
    return mProcessObject;
}

DataObject::pointer ProcessObjectPort::getData() {
	mDataPointer = (std::size_t)mProcessObject->getOutputDataX(mPortID).getPtr().get();
    return mProcessObject->getOutputDataX(mPortID);
}

std::vector<DataObject::pointer> ProcessObjectPort::getMultipleData() {
    return mProcessObject->getMultipleOutputDataX(mPortID);
}

uint ProcessObjectPort::getPortID() const {
    return mPortID;
}

bool ProcessObjectPort::isDataModified() const {
    return mTimestamp != mProcessObject->getOutputDataX(mPortID)->getTimestamp() ||
            (mDataPointer != 0 && mDataPointer != (std::size_t)mProcessObject->getOutputDataX(mPortID).getPtr().get());
}

void ProcessObjectPort::updateTimestamp() {
    mTimestamp = getData()->getTimestamp();
}

bool ProcessObjectPort::operator==(const ProcessObjectPort &other) const {
    return mPortID == other.getPortID() && mProcessObject == other.getProcessObject();
}

bool ProcessObject::inputPortExists(uint portID) const {
    return mInputPortType.count(portID) > 0;
}

bool ProcessObject::outputPortExists(uint portID) const {
    return mOutputPortType.count(portID) > 0;
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

void ProcessObject::setAttributes(std::vector<std::shared_ptr<Attribute>> attributes) {
    for(std::shared_ptr<Attribute> attribute : attributes) {
        std::string name = attribute->getName();
        if(mAttributes.count(name) == 0) {
            throw Exception("Attribute " + name + " not found for process object " + getNameOfClass());
        }

        std::shared_ptr<Attribute> localAttribute = mAttributes.at(name);
        if(localAttribute->getType() != attribute->getType())
            throw Exception("Attribute " + name + " for process object " + getNameOfClass() + " had different type then the one loaded.");

        localAttribute->setValues(attribute->getValues());
    }
}

void ProcessObject::loadAttributes() {
    //throw Exception("The process object " + getNameOfClass() + " has not implemented the loadAttributes method and therefore cannot be loaded from fast pipeline files (.fpl).");
}

void ProcessObject::createFloatAttribute(std::string id, std::string name, std::string description, float initialValue) {
    std::shared_ptr<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_FLOAT);
    std::shared_ptr<AttributeValue> value = std::make_shared<AttributeValueFloat>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

void ProcessObject::createIntegerAttribute(std::string id, std::string name, std::string description, int initialValue) {
    std::shared_ptr<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_INTEGER);
    std::shared_ptr<AttributeValue> value = std::make_shared<AttributeValueInteger>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

void ProcessObject::createBooleanAttribute(std::string id, std::string name, std::string description, bool initialValue) {
    std::shared_ptr<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_BOOLEAN);
    std::shared_ptr<AttributeValue> value = std::make_shared<AttributeValueBoolean>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

void ProcessObject::createStringAttribute(std::string id, std::string name, std::string description, std::string initialValue) {
    std::shared_ptr<Attribute> attribute = std::make_shared<Attribute>(id, name, description, ATTRIBUTE_TYPE_STRING);
    std::shared_ptr<AttributeValue> value = std::make_shared<AttributeValueString>(initialValue);
    attribute->setValue(value);
    mAttributes[id] = attribute;
}

std::shared_ptr<Attribute> ProcessObject::getAttribute(std::string id) {
    if(mAttributes.count(id) == 0)
        throw Exception("Attribute " + id + " not found for process object " + getNameOfClass() +
                                ". Did you forget to define it in the constructor?");

    return mAttributes[id];
}

float ProcessObject::getFloatAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_FLOAT)
        throw Exception("Attribute " + id + " is not of type float in process object " + getNameOfClass());

    std::shared_ptr<AttributeValueFloat> value = std::dynamic_pointer_cast<AttributeValueFloat>(attribute->getValue());
    return value->get();
}

std::vector<float> ProcessObject::getFloatListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_FLOAT)
        throw Exception("Attribute " + id + " is not of type float in process object " + getNameOfClass());

    std::vector<std::shared_ptr<AttributeValue>> values = attribute->getValues();
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

    std::shared_ptr<AttributeValueInteger> value = std::dynamic_pointer_cast<AttributeValueInteger>(attribute->getValue());
    return value->get();
}

std::vector<int> ProcessObject::getIntegerListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_INTEGER)
        throw Exception("Attribute " + id + " is not of type integer in process object " + getNameOfClass());

    std::vector<std::shared_ptr<AttributeValue>> values = attribute->getValues();
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

    std::shared_ptr<AttributeValueBoolean> value = std::dynamic_pointer_cast<AttributeValueBoolean>(attribute->getValue());
    return value->get();
}


std::vector<bool> ProcessObject::getBooleanListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_BOOLEAN)
        throw Exception("Attribute " + id + " is not of type boolean in process object " + getNameOfClass());

    std::vector<std::shared_ptr<AttributeValue>> values = attribute->getValues();
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

    std::shared_ptr<AttributeValueString> value = std::dynamic_pointer_cast<AttributeValueString>(attribute->getValue());
    return value->get();
}


std::vector<std::string> ProcessObject::getStringListAttribute(std::string id) {
    auto attribute = getAttribute(id);
    if(attribute->getType() != ATTRIBUTE_TYPE_STRING)
        throw Exception("Attribute " + id + " is not of type string in process object " + getNameOfClass());

    std::vector<std::shared_ptr<AttributeValue>> values = attribute->getValues();
    std::vector<std::string> list;
    for(auto &&value : values) {
        auto floatValue = std::dynamic_pointer_cast<AttributeValueString>(value);
        list.push_back(floatValue->get());
    }
    return list;
}

std::unordered_map<std::string, std::shared_ptr<Attribute>> ProcessObject::getAttributes() {
    return mAttributes;
}

DynamicData::pointer ProcessObject::getDynamicOutputData(uint outputNumber) {
    DynamicData::pointer data;

    // If output data is not created
    if(mOutputData.count(outputNumber) == 0) {
        // Is output dependent on any input?
        if(mOutputDynamicDependsOnInput.count(outputNumber) > 0) {
            uint inputNumber = mOutputDynamicDependsOnInput[outputNumber];
            if(mInputConnections.count(inputNumber) == 0)
                throw Exception("Must call input before output.");
            ProcessObjectPort port = mInputConnections[inputNumber];
            DataObject::pointer objectDependsOn = port.getData();
            data = DynamicData::New();
            data->setStreamer(objectDependsOn->getStreamer());
            mOutputData[outputNumber].push_back(data);
        } else {
            // Create dynamic data
            data = DynamicData::New();
            mOutputData[outputNumber].push_back(data);
        }
    } else {
        data = mOutputData[outputNumber][0];
    }

    return data;
}

DynamicData::pointer ProcessObject::getDynamicOutputData() {
    return getDynamicOutputData(0);
}



} // namespace fast

namespace std {
size_t hash<fast::ProcessObjectPort>::operator()(const fast::ProcessObjectPort &object) const {
    std::size_t seed = 0;
    fast::hash_combine(seed, object.getProcessObject().getPtr().get());
    fast::hash_combine(seed, object.getPortID());
    return seed;
}
} // end namespace std