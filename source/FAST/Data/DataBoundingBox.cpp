#include "FAST/Data/DataBoundingBox.hpp"

namespace fast {

void DataBoundingBox::createCorners(Vector3f pos, Vector3f size) {
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

    mCorners = MatrixXf::Constant(8,3,0);
    for(int c = 0; c < 8; c++) {
        for(int i = 0; i < 3; i++) {
            mCorners(c,i) = corners[c][i];
        }
    }
}

DataBoundingBox::DataBoundingBox(Vector3f pos, Vector3f size) {
    mIsInitialized = true;
    createCorners(pos, size);
}

DataBoundingBox::DataBoundingBox(Vector3f size) {
    mIsInitialized = true;
    Vector3f pos;
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    createCorners(pos, size);
}

DataBoundingBox::DataBoundingBox(MatrixXf corners) {
    mCorners = corners; // copy
    mIsInitialized = true;
}

DataBoundingBox::DataBoundingBox() {
    mIsInitialized = false;
}

MatrixXf DataBoundingBox::getCorners() const {
    if(!mIsInitialized)
        throw Exception("Cannot getCorners because bounding box was not initialized.");
    return mCorners;
}


void DataBoundingBox::initialize(std::vector<Vector3f> coordinates) {
    if(coordinates.size() == 0) {
        createCorners(Vector3f::Zero(), Vector3f::Ones());
        mIsInitialized = true;
        return;
    }
    // Find min and max of all the coordinates
    Vector3f minimum(coordinates[0].x(), coordinates[0].y(), coordinates[0].z());
    Vector3f maximum(coordinates[0].x(), coordinates[0].y(), coordinates[0].z());
    for(uint i = 1; i < coordinates.size(); i++) {
        Vector3f coordinate = coordinates[i];
        for(uint j = 0; j < 3; j++) {
            if(coordinate[j] < minimum[j]) {
                minimum[j] = coordinate[j];
            }
            if(coordinate[j] > maximum[j]) {
                maximum[j] = coordinate[j];
            }
        }
    }

    // Make new bounding box
    Vector3f size(maximum.x()-minimum.x(), maximum.y()-minimum.y(), maximum.z()-minimum.z());
    createCorners(minimum, size);
    mIsInitialized = true;
}

DataBoundingBox::DataBoundingBox(std::vector<Vector3f> coordinates) {
	initialize(coordinates);
}

DataBoundingBox DataBoundingBox::getTransformedBoundingBox(
        Affine3f transform) const {
    if(!mIsInitialized)
        throw Exception("Cannot getTransformedBoundingBox because bounding box was not initialized.");
    MatrixXf newCorners = MatrixXf::Constant(8,3,0);
    for(uint i = 0; i < 8; i++) {
        Vector3f vertex = mCorners.row(i);
        Vector3f transformedVertex = (transform.matrix()*vertex.homogeneous()).head(3);
        newCorners.row(i) = transformedVertex;
    }
    return DataBoundingBox(newCorners);
}

DataBoundingBox DataBoundingBox::getTransformedBoundingBox(
        Transform::pointer transform) const {
    return getTransformedBoundingBox(transform->get());
}

std::ostream &operator<<(std::ostream &os, DataBoundingBox &object) {
    os << std::endl << "Bounding box" << std::endl;

    MatrixXf corners = object.getCorners();

    for(uint i = 0; i < 8; i++) {
        os << "Corner " << i << ": " << corners(i,0) << ", " << corners(i, 1) << ", " << corners(i, 2) << std::endl;
    }

    return os;

}

bool DataBoundingBox::isInitialized() const {
    return mIsInitialized;
}

} // end namespace fast

