#include "FAST/Data/DataObject.hpp"
#include "FAST/ProcessObject.hpp"

namespace fast {

DataObject::DataObject() : mTimestampModified(0), mIsDynamicData(false) {
}


bool DataObject::isDynamicData() {
    return mIsDynamicData;
}

unsigned long DataObject::getTimestamp() {
    return mTimestampModified;
}

void DataObject::updateModifiedTimestamp() {
    mTimestampModified++;
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

void DataObject::setStreamer(Streamer::pointer streamer) {
    mStreamer = streamer;
}

Streamer::pointer DataObject::getStreamer() {
    return mStreamer.lock();
}

} // end namespace fast
