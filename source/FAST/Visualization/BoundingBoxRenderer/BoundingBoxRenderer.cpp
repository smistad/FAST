#include "BoundingBoxRenderer.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace fast {

BoundingBoxRenderer::BoundingBoxRenderer() {
    createInputPort<SpatialDataObject>(0, false);
}

void BoundingBoxRenderer::execute() {
    std::lock_guard<std::mutex> lock(mMutex);

    for(uint i = 0; i < getNrOfInputConnections(); ++i) {
        SpatialDataObject::pointer data = getInputData<SpatialDataObject>(i);
        mBoxesToRender[i] = data->getTransformedBoundingBox();
    }
}

void
BoundingBoxRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    // Draw each bounding box
    std::unordered_map<uint, BoundingBox>::iterator it;
    glBegin(GL_LINES);
    glColor3f(0.0f, 1.0f, 0.0f);
    for(it = mBoxesToRender.begin(); it != mBoxesToRender.end(); ++it) {
        BoundingBox box = it->second;
        MatrixXf corners = box.getCorners();
        // Should be 12 lines in total
        for(uint i = 0; i < 8; ++i) {
            Vector3f A = corners.row(i);
            for(uint j = 0; j < i; ++j) {
                Vector3f B = corners.row(j);
                // If only 1 coordinate is different, draw the line
                if((A.x() != B.x() ? 1 : 0) +
                    (A.y() != B.y() ? 1 : 0) +
                    (A.z() != B.z() ? 1 : 0) == 1) {
                    glVertex3f(A.x(), A.y(), A.z());
                    glVertex3f(B.x(), B.y(), B.z());
                }

            }
        }
    }
    glEnd();
}

BoundingBox BoundingBoxRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    std::unordered_map<uint, BoundingBox>::iterator it;
    for(it = mBoxesToRender.begin(); it != mBoxesToRender.end(); ++it) {
        BoundingBox transformedBoundingBox = it->second;
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

}
