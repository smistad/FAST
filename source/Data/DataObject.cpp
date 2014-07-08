#include "DataObject.hpp"
#include "ProcessObject.hpp"
using namespace fast;

void DataObject::update() {
    if(mSourceObject.lock().isValid()) {
        ProcessObject::pointer object = mSourceObject.lock();
        object->update();
    }
}

void DataObject::setSource(Object::pointer source) {
    if(!source.isValid())
        throw Exception("Trying to add an expired/NULL pointer as a parent object");

    mSourceObject = source;
}

unsigned long DataObject::getTimestamp() {
    return mTimestampModified;
}

void DataObject::updateModifiedTimestamp() {
    static unsigned long globalTimestamp = 0;
    mTimestampModified = ++globalTimestamp;
}

void DataObject::retain(ExecutionDevice::pointer device) {
    if(mReferenceCount.count(device) == 0) {
        mReferenceCount[device] = 1;
    } else {
        mReferenceCount[device]++;
    }
}

void DataObject::release(ExecutionDevice::pointer device) {
    if(mReferenceCount.count(device) == 0)
        throw Exception("Trying to release data on a device that has not retained the data.");
    mReferenceCount[device]--;

    // TODO: if there is data left for a data object it must be marked as modified somehow so that it will call update on itself
    // when someone request access on it
    if(mReferenceCount[device] == 0) {
        this->free(device);
    }
}
