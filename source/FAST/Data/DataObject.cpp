#include "FAST/Data/DataObject.hpp"
#include "FAST/ProcessObject.hpp"

namespace fast {

DataObject::DataObject() :
        mTimestampModified(0),
        mTimestampCreated(0) {

    mDataIsBeingAccessed = false;
    mDataIsBeingWrittenTo = false;
}

void DataObject::blockIfBeingWrittenTo() {
    std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
    while(mDataIsBeingWrittenTo) {
        mDataIsBeingWrittenToCondition.wait(lock);
    }
}

void DataObject::blockIfBeingAccessed() {
    std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
    while(mDataIsBeingAccessed) {
        mDataIsBeingWrittenToCondition.wait(lock);
    }
}

void DataObject::accessFinished() {
	{
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = false;
	}
	mDataIsBeingWrittenToCondition.notify_one();

	{
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = false;
	}
	mDataIsBeingAccessedCondition.notify_one();
}

uint64_t DataObject::getTimestamp() const {
    return mTimestampModified;
}

uint64_t DataObject::getCreationTimestamp() const {
    return mTimestampCreated;
}

void DataObject::setCreationTimestamp(uint64_t timestamp) {
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

uint64_t DataObject::getTimestep() const {
    return mTimestep;
}

void DataObject::setTimestep(uint64_t timestep) {
    mTimestep = timestep;
}

void DataObject::setMetadata(std::string name, std::string value) {
    mMetadata[name] = value;
}

std::string DataObject::getMetadata(std::string name) const {
    if(mMetadata.count(name) == 0)
        throw Exception("Metadata " + name + " was not found.");

    return mMetadata.at(name);
}

std::unordered_map<std::string, std::string> DataObject::getMetadata() const {
    return mMetadata;
}

void DataObject::setMetadata(std::unordered_map<std::string, std::string> metadata) {
    mMetadata = metadata;
}

} // end namespace fast
