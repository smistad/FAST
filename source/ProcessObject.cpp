#include "ProcessObject.hpp"
#include "Exception.hpp"

using namespace fast;

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
        this->execute();
        if(this->mRuntimeManager->isEnabled())
            this->waitToFinish();
        this->mRuntimeManager->stopRegularTimer("execute");
        this->mIsModified = false;
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

void ProcessObject::setInputRequired(uint inputNumber, bool required) {
    mRequiredInputs[inputNumber] = required;
}

void ProcessObject::releaseInputAfterExecute(uint inputNumber,
        bool release) {
    mReleaseAfterExecute[inputNumber] = release;
}

void ProcessObject::setInputData(uint inputNumber,
        const DataObject::pointer data) {
    // Default values:
    // Input is by default reuiqred and will be relased after execute
    mRequiredInputs[inputNumber] = true;
    mReleaseAfterExecute[inputNumber] = true;

    setParent(data);
    mIsModified = true;
    // TODO Retain on device
    //data->retain(device);
    // TODO if another input with same number existed, release it

    mInputs[inputNumber] = data;
}

DataObject::pointer ProcessObject::getInputData(uint inputNumber) const {
    // at throws exception if element not found, while [] does not
    return mInputs.at(inputNumber);
}



