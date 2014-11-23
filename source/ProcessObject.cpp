#include "ProcessObject.hpp"
#include "Exception.hpp"

namespace fast {

void ProcessObject::update() {
    bool aParentHasBeenModified = false;
    for(unsigned int i = 0; i < mParentDataObjects.size(); i++) {
        // Check that object has not been deleted
        // TODO maybe throw exception here?
        if(mParentDataObjects[i].isValid())
            mParentDataObjects[i]->update();

        // Check if a data object has been updated
        if(mParentDataObjects[i]->getTimestamp() != mTimestamps[i]) {
            aParentHasBeenModified = true;
            // Update timestamp
            setTimestamp(mParentDataObjects[i], mParentDataObjects[i]->getTimestamp());
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

void ProcessObject::addParent(DataObject::pointer parent) {
    if(!parent.isValid())
        throw Exception("Trying to add an expired/NULL pointer as a parent object");

    // Check that it doesn't already exist
    bool exist = false;
    for(unsigned int i = 0; i < mParentDataObjects.size(); i++) {
        if(parent == mParentDataObjects[i])
            exist = true;
    }
    if(!exist) {
        mParentDataObjects.push_back(parent);
        mTimestamps.push_back(parent->getTimestamp());
    }
}

void ProcessObject::enableRuntimeMeasurements() {
    mRuntimeManager->enable();
}

void ProcessObject::disableRuntimeMeasurements() {
    mRuntimeManager->disable();
}

void ProcessObject::setTimestamp(
        DataObject::pointer object,
        unsigned long timestamp) {

    for(unsigned int i = 0; i < mParentDataObjects.size(); i++) {
        if(object == mParentDataObjects[i]) {
            mTimestamps[i] = timestamp;
            break;
        }
    }
}

oul::RuntimeMeasurementPtr ProcessObject::getRuntime() {
    return mRuntimeManager->getTiming("execute");
}

oul::RuntimeMeasurementPtr ProcessObject::getRuntime(std::string name) {
    return mRuntimeManager->getTiming(name);
}

void ProcessObject::setParent(DataObject::pointer parent) {
    removeParents();
    addParent(parent);
}

void ProcessObject::removeParents() {
    mParentDataObjects.clear();
}

void ProcessObject::removeParent(const DataObject::pointer data) {
    std::vector<DataObject::pointer> newParents;
    for(int i = 0; i < mParentDataObjects.size(); i++) {
        if(mParentDataObjects[i] != data) {
            newParents.push_back(mParentDataObjects[i]);
        }
    }
    mParentDataObjects = newParents;
}

void ProcessObject::setInputRequired(uint inputNumber, bool required) {
    mRequiredInputs[inputNumber] = required;
}

void ProcessObject::releaseInputAfterExecute(uint inputNumber,
        bool release) {
    mReleaseAfterExecute[inputNumber] = release;
}

void ProcessObject::setInputData(uint inputNumber,
        DataObject::pointer data) {
    // Default values:
    // Input is by default reuiqred and will be relased after execute
    mRequiredInputs[inputNumber] = true;
    mReleaseAfterExecute[inputNumber] = false;

    // TODO if another input with same number existed, release it and remove it as parent
    if(mInputs.count(inputNumber) > 0) {
        removeParent(data);
        std::vector<uint>::iterator it;
        for(it = mInputDevices[inputNumber].begin(); it != mInputDevices[inputNumber].end(); it++) {
            data->release(getDevice(*it));
        }
        mInputDevices[inputNumber].clear();
    }
    addParent(data);
    mIsModified = true;
    mInputDevices[inputNumber].push_back(0);
    data->retain(getMainDevice());

    mInputs[inputNumber] = data;
}

DataObject::pointer ProcessObject::getInputData(uint inputNumber) const {
    // at throws exception if element not found, while [] does not
    return mInputs.at(inputNumber);
}

void ProcessObject::preExecute() {
    // Check that required inputs are present
    boost::unordered_map<uint, bool>::iterator it;
    for(it = mRequiredInputs.begin(); it != mRequiredInputs.end(); it++) {
        if(it->second) { // if required
            // Check that input exist and is valid
            if(mInputs.count(it->first) == 0 || !mInputs[it->first].isValid()) {
                throw Exception("A process object is missing a required input data.");
            }
        }
    }
}

void ProcessObject::postExecute() {
    // Release input data if they are marked as "release after execute"
    boost::unordered_map<uint, bool>::iterator it;
    for(it = mReleaseAfterExecute.begin(); it != mReleaseAfterExecute.end(); it++) {
        if(it->second) {
            std::vector<uint>::iterator it2;
            for(it2 = mInputDevices[it->first].begin(); it2 != mInputDevices[it->first].end(); it2++) {
                DataObject::pointer data = mInputs[it->first];
                data->release(getDevice(*it2));
            }
        }
    }
}

void ProcessObject::changeDeviceOnInputs(uint deviceNumber, ExecutionDevice::pointer device) {
    // For each input, release old device and retain on new device
    boost::unordered_map<uint, DataObject::pointer>::iterator it;
    for(it = mInputs.begin(); it != mInputs.end(); it++) {
        std::vector<uint>::iterator it2;
        for(it2 = mInputDevices[it->first].begin(); it2 != mInputDevices[it->first].end(); it2++) {
            if(*it2 == deviceNumber) {
                it->second->release(mDevices[deviceNumber]);
                it->second->retain(device);
            }
        }
    }
}

void ProcessObject::setInputData(uint inputNumber,
        const DataObject::pointer data, const ExecutionDevice::pointer device) {
}

void ProcessObject::setMainDevice(ExecutionDevice::pointer device) {
    changeDeviceOnInputs(0, device);
    mDevices[0] = device;
}

ExecutionDevice::pointer ProcessObject::getMainDevice() const {
    return mDevices.at(0);
}

void ProcessObject::setDevice(uint deviceNumber,
        ExecutionDevice::pointer device) {
    if(mDevices.count(deviceNumber) > 0) {
        changeDeviceOnInputs(deviceNumber, device);
    }
    mDevices[deviceNumber] = device;
}

ExecutionDevice::pointer ProcessObject::getDevice(uint deviceNumber) const {
    return mDevices.at(deviceNumber);
}

uint ProcessObject::getNrOfInputData() const {
    return mInputs.size();
}

void ProcessObject::setOutputData(uint outputNumber, DataObject::pointer data) {
    mOutputs[outputNumber] = data;
}


} // namespace fast


