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

BoundingBox::BoundingBox(std::vector<Float3> coordinates) {
    // Find min and max of all the coordinates
    Float3 minimum(coordinates[0].x(), coordinates[0].y(), coordinates[0].z());
    Float3 maximum(coordinates[0].x(), coordinates[0].y(), coordinates[0].z());
    for(uint i = 1; i < coordinates.size(); i++) {
        Float3 coordinate = coordinates[0];
        for(uint j = 0; j < 4; j++) {
            if(coordinate[j] < minimum[j]) {
                minimum[j] = coordinate[j];
            }
            if(coordinate[j] > maximum[j]) {
                maximum[j] = coordinate[j];
            }
        }
    }

    // Make new bounding box
    Float3 size(maximum.x()-minimum.x(), maximum.y()-minimum.y(), maximum.z()-minimum.z());
    BoundingBox(minimum, size);
}

BoundingBox BoundingBox::getTransformedBoundingBox(
        LinearTransformation transform) {
    Vector<Float3, 8> newCorners;
    for(uint i = 0; i < 8; i++) {
        Float3 vertex = mCorners[i];
        Float3 transformedVertex = transform*vertex;
        newCorners[i] = transformedVertex;
    }
    return BoundingBox(newCorners);
}

} // end namespace fast

