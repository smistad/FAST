#include "LineSet.hpp"

namespace fast {

void LineSet::create(std::vector<Vector3f> vertices,
        std::vector<Vector2ui> lines) {
    mVertices = vertices;
    mLines = lines;
    updateModifiedTimestamp();
}

bool LineSet::isAnyDataBeingAccessed() const {
    return mHostDataIsBeingAccessed;
}

LineSetAccess::pointer LineSet::getAccess(accessType access) {

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

    // TODO should send a parameter whether writing is allowed or not

    mHostDataIsBeingAccessed = true;
    LineSetAccess::pointer accessObject(new LineSetAccess(&mVertices, &mLines, &mHostDataIsBeingAccessed, &mIsBeingWrittenTo));
	return accessObject;
}

LineSet::LineSet() {
    mIsBeingWrittenTo = false;
    mHostDataIsBeingAccessed = false;
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
    std::vector<Vector3f> list;
    for(uint i = 0; i < mVertices.size(); i++) {
        list.push_back(mVertices[i]);
    }
    return BoundingBox(list);
}

} // end namespace fast


