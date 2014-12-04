#include "PointSet.hpp"

namespace fast {

void PointSet::create(std::vector<Vector3f> points) {
    mPointSet = points;
}

uint PointSet::getNrOfPoints() const {
    return mPointSet.size();
}

bool PointSet::isAnyDataBeingAccessed() const {
    return mHostDataIsBeingAccessed;
}

PointSetAccess PointSet::getAccess(accessType access) {
    update();

    if(mIsBeingWrittenTo)
        throw Exception("Requesting access to a surface that is already being written to.");
    if(access == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mIsBeingWrittenTo = true;
        updateModifiedTimestamp();
    }

    // TODO should send a parameter whether writing is allowed or not to PointSetAccess

    mHostDataIsBeingAccessed = true;
    return PointSetAccess(&mPointSet, &mHostDataIsBeingAccessed, &mIsBeingWrittenTo);
}

PointSet::PointSet() {
    mIsBeingWrittenTo = false;
    mHostDataIsBeingAccessed = false;
}

void PointSet::freeAll() {
    mPointSet.clear();
}

void PointSet::free(ExecutionDevice::pointer device) {
    // TODO
    if(device->isHost()) {
        mPointSet.clear();
    }
}

BoundingBox PointSet::getBoundingBox() const {
    std::vector<Vector3f> list;
    for(uint i = 0; i < mPointSet.size(); i++) {
        list.push_back(mPointSet[i]);
    }
    return BoundingBox(list);
}

PointSet::~PointSet() {
    freeAll();
}


} // end namespace fast
