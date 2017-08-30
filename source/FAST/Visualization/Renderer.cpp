#include <FAST/Data/SpatialDataObject.hpp>
#include "Renderer.hpp"

namespace fast {

Renderer::Renderer() {
}



void Renderer::addInputConnection(DataPort::pointer port) {
    uint nr = getNrOfInputConnections();
    if(nr > 0)
        createInputPort<DataObject>(nr);
    setInputConnection(nr, port);
}

void Renderer::lock() {
    mMutex.lock();
}

void Renderer::unlock() {
    mMutex.unlock();
}

void Renderer::stopPipeline() {
    mHasRendered = true;
    mRenderedCV.notify_one();
    ProcessObject::stopPipeline();
}

void Renderer::postDraw() {
    mHasRendered = true;
    mRenderedCV.notify_one();
}

void Renderer::execute() {
    std::unique_lock<std::mutex> lock(mMutex);
    std::cout << "renderer EXECUTE" << std::endl;

    // Check if current images has not been rendered, if not wait
    while(!mHasRendered) {
        mRenderedCV.wait(lock);
    }
    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
        if(hasNewInputData(inputNr)) {
            SpatialDataObject::pointer input = getInputData<SpatialDataObject>(inputNr);

            mHasRendered = false;
            mDataToRender[inputNr] = input;
        }
    }
}

BoundingBox Renderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;

    if(mDataToRender.size() == 0)
        throw Exception("Renderer has no input data. Unable to create bounding box and thereby initialize GL scene.");

    for(auto it : mDataToRender) {
        BoundingBox transformedBoundingBox;
        transformedBoundingBox = it.second->getTransformedBoundingBox();

        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }

    if(coordinates.size() == 0)
        throw Exception("Renderer did not get any data. Unable to create bounding box and thereby initialize GL scene.");

    return BoundingBox(coordinates);
}

}
