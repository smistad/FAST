#include "LineSet.hpp"

namespace fast {

void LineSet::create(std::vector<Vector3f> vertices,
        std::vector<Vector2ui> lines) {
    mVertices = vertices;
    mLines = lines;
    updateModifiedTimestamp();
}

LineSetAccess::pointer LineSet::getAccess(accessType access) {

	blockIfBeingWrittenTo();

    if(access == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
    	{
            std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
            mDataIsBeingWrittenTo = true;
    	}
        updateModifiedTimestamp();
    }

    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }
    // TODO should send a parameter whether writing is allowed or not
    LineSetAccess::pointer accessObject(new LineSetAccess(&mVertices, &mLines, mPtr.lock()));
	return accessObject;
}

LineSet::LineSet() {
}

LineSet::~LineSet() {
    freeAll();
}

void LineSet::freeAll() {
    mVertices.clear();
    mLines.clear();
}

void LineSet::free(ExecutionDevice::pointer device) {
    freeAll();
}

BoundingBox LineSet::getBoundingBox() const {
    if(mVertices.size() == 0)
        return BoundingBox();
    std::vector<Vector3f> list;
    for(uint i = 0; i < mVertices.size(); i++) {
        list.push_back(mVertices[i]);
    }
    return BoundingBox(list);
}

} // end namespace fast


