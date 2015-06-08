#include "FAST/ProcessObject.hpp"
#include "FAST/Exception.hpp"
#include <boost/lexical_cast.hpp>

namespace fast {

ProcessObject::ProcessObject() : mIsModified(false), mRuntimeManager(new oul::RuntimeMeasurementsManager) {
     mDevices[0] = DeviceManager::getInstance().getDefaultComputationDevice();
}

void ProcessObject::update() {
    bool aParentHasBeenModified = false;
    // TODO check mInputConnections here instead
    boost::unordered_map<uint, ProcessObjectPort>::iterator it;
    for(it = mInputConnections.begin(); it != mInputConnections.end(); it++) {
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

    // If this process object itself has been modified or a parent object (input)
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

oul::RuntimeMeasurementPtr ProcessObject::getRuntime() {
    return mRuntimeManager->getTiming("execute");
}

oul::RuntimeMeasurementPtr ProcessObject::getRuntime(std::string name) {
    return mRuntimeManager->getTiming(name);
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
    boost::unordered_map<uint, bool>::iterator it;
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
    boost::unordered_map<uint, bool>::iterator it;
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
    boost::unordered_map<uint, ProcessObjectPort>::iterator it;
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
        if(!DeviceManager::getInstance().deviceSatisfiesCriteria(device, mDeviceCriteria[deviceNumber]))
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
    mOutputData[outputNumber] = data;
}


void ProcessObject::setOutputDataDynamicDependsOnInputData(uint outputNumber, uint inputNumber) {
    mOutputDynamicDependsOnInput[outputNumber] = inputNumber;
}

void ProcessObject::setMainDeviceCriteria(const DeviceCriteria& criteria) {
    mDeviceCriteria[0] = criteria;
    mDevices[0] = DeviceManager::getInstance().getDevice(criteria);
}

void ProcessObject::setDeviceCriteria(uint deviceNumber,
        const DeviceCriteria& criteria) {
    mDeviceCriteria[deviceNumber] = criteria;
    mDevices[deviceNumber] = DeviceManager::getInstance().getDevice(criteria);
}

// New pipeline
void ProcessObject::setInputConnection(ProcessObjectPort port) {
    setInputConnection(0, port);
}

void ProcessObject::setInputConnection(uint connectionID, ProcessObjectPort port) {
    mInputConnections[connectionID] = port;
}

ProcessObjectPort ProcessObject::getOutputPort() {
    return getOutputPort(0);
}

ProcessObjectPort ProcessObject::getOutputPort(uint portID) {
    ProcessObjectPort port(portID, mPtr.lock());
    return port;
}

DataObject::pointer ProcessObject::getOutputDataX(uint portID) const {
    DataObject::pointer data;

    // If output data is not created
    if(mOutputData.count(portID) == 0) {
        throw Exception("Could not find output data for port " + boost::lexical_cast<std::string>(portID));
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

void ProcessObject::setInputData(DataObject::pointer data) {
    setInputData(0, data);
}

ProcessObjectPort ProcessObject::getInputPort(uint portID) const {
    return mInputConnections.at(portID);
}


void ProcessObject::updateTimestamp(DataObject::pointer data) {
    boost::unordered_map<uint, ProcessObjectPort>::iterator it;
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
}

ProcessObject::pointer ProcessObjectPort::getProcessObject() const {
    return mProcessObject;
}

DataObject::pointer ProcessObjectPort::getData() const {
    return mProcessObject->getOutputDataX(mPortID);
}

uint ProcessObjectPort::getPortID() const {
    return mPortID;
}

bool ProcessObjectPort::isDataModified() const {
    return mTimestamp != getData()->getTimestamp();
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

} // namespace fast
