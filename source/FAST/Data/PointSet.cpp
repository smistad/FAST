#include "PointSet.hpp"

namespace fast {

void PointSet::create(std::vector<Vector3f> points) {
    mPointSet = points;
    updateModifiedTimestamp();
}

uint PointSet::getNrOfPoints() const {
    return mPointSet.size();
}

PointSetAccess::pointer PointSet::getAccess(accessType access) {

	blockIfBeingWrittenTo();

    if(access == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
    	{
            boost::unique_lock<boost::mutex> lock(mDataIsBeingWrittenToMutex);
            mDataIsBeingWrittenTo = true;
    	}
        updateModifiedTimestamp();
    }

    {
        boost::unique_lock<boost::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }
    // TODO should send a parameter whether writing is allowed or not to PointSetAccess
    PointSetAccess::pointer accessObject(new PointSetAccess(&mPointSet, mPtr.lock()));
	return accessObject;
}

PointSet::PointSet() {
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
