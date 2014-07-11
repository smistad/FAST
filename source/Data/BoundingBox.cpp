#include "BoundingBox.hpp"

namespace fast {

BoundingBox::BoundingBox(Float3 pos, Float3 size) {
    mIsInitialized = true;
    // Create corners
    float corners[8][3] = {
        {pos.x(),pos.y(),pos.z()},
        {pos.x()+size.x(),pos.y(),pos.z()},
        {pos.x()+size.x(),pos.y()+size.y(),pos.z()},
        {pos.x()+size.x(),pos.y()+size.y(),pos.z()+size.z()},
        {pos.x(),pos.y()+size.y(),pos.z()+size.z()},
        {pos.x(),pos.y(),pos.z()+size.z()},
        {pos.x()+size.x(),pos.y(),pos.z()+size.z()},
        {pos.x(),pos.y()+size.y(),pos.z()}
    };

    for(int c = 0; c < 8; c++) {
        for(int i = 0; i < 3; i++)
            mCorners[c][i] = corners[c][i];
    }
}

BoundingBox::BoundingBox(Float3 size) {
    mIsInitialized = true;
    Float3 pos;
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    BoundingBox(pos, size);
}

BoundingBox::BoundingBox(Vector<Float3, 8> corners) {
    mIsInitialized = true;
    mCorners = corners; // copy
}

BoundingBox::BoundingBox() {
    mIsInitialized = false;
}

Vector<Float3, 8> BoundingBox::getCorners() {
    if(!mIsInitialized)
        throw Exception("Cannot getCorners because bounding box was not initialized.");
    return mCorners;
}

} // end namespace fast


