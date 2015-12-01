#include "FAST/Data/DataObject.hpp"
#include "FAST/ProcessObject.hpp"

namespace fast {

DataObject::DataObject() :
        mTimestampModified(0),
        mTimestampCreated(0),
        mIsDynamicData(false) {

    mDataIsBeingAccessed = false;
    mDataIsBeingWrittenTo = false;
}

void DataObject::blockIfBeingWrittenTo() {
    boost::unique_lock<boost::mutex> lock(mDataIsBeingWrittenToMutex);
    while(mDataIsBeingWrittenTo) {
        mDataIsBeingWrittenToCondition.wait(lock);
    }
}

void DataObject::blockIfBeingAccessed() {
    boost::unique_lock<boost::mutex> lock(mDataIsBeingAccessedMutex);
    while(mDataIsBeingAccessed) {
        mDataIsBeingWrittenToCondition.wait(lock);
    }
}

void DataObject::accessFinished() {
	{
        boost::unique_lock<boost::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = false;
	}
	mDataIsBeingWrittenToCondition.notify_one();

	{
        boost::unique_lock<boost::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = false;
	}
	mDataIsBeingAccessedCondition.notify_one();
}

bool DataObject::isDynamicData() const {
    return mIsDynamicData;
}

unsigned long DataObject::getTimestamp() const {
    return mTimestampModified;
}

unsigned long DataObject::getCreationTimestamp() const {
    return mTimestampCreated;
}

void DataObject::setCreationTimestamp(unsigned long timestamp) {
    mTimestampCreated = timestamp;
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
