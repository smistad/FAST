#include "SliceRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Algorithms/ImageSlicer/ImageSlicer.hpp"


namespace fast {


void SliceRenderer::execute() {
    std::unique_lock<std::mutex> lock(mMutex);
    if(mStop) {
        return;
    }

    // Check if current images has not been rendered, if not wait
    while(!mHasRendered) {
        mRenderedCV.wait(lock);
    }
    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
        if(hasNewInputData(inputNr)) {
            SpatialDataObject::pointer input = getInputData<SpatialDataObject>(inputNr);

            // Get slicer and execute it on new input data
            ImageSlicer::pointer slicer;
            if(mSlicers.count(inputNr) == 0) {
                slicer = ImageSlicer::New();
                slicer->setOrthogonalSlicePlane(PLANE_X);
            } else {
                slicer = mSlicers.at(inputNr);
            }
            slicer->setInputData(input);
            DataPort::pointer port = slicer->getOutputPort();
            slicer->update(0);

            mHasRendered = false;
            mDataToRender[inputNr] = port->getNextFrame<Image>();
        }
    }
}


SliceRenderer::SliceRenderer() {
    createInputPort<Image>(0, false);
    mIsModified = true;
}

uint SliceRenderer::addInputConnection(DataPort::pointer port, PlaneType orthogonalSlicePlane, int sliceNr) {
    uint portID = Renderer::addInputConnection(port);
    mSlicers[portID] = ImageSlicer::New();
    mSlicers[portID]->setOrthogonalSlicePlane(orthogonalSlicePlane, sliceNr);
    return portID;
}

uint SliceRenderer::addInputConnection(DataPort::pointer port, Plane slicePlane) {
    uint portID = Renderer::addInputConnection(port);
    mSlicers[portID] = ImageSlicer::New();
    mSlicers[portID]->setArbitrarySlicePlane(slicePlane);
    return portID;
}

uint SliceRenderer::addInputConnection(DataPort::pointer port, SharedPointer<ImageSlicer> slicer) {
    uint portID = Renderer::addInputConnection(port);
    mSlicers[portID] = slicer;
    return portID;
}

void SliceRenderer::setOrthogonalSlicePlane(uint portID, PlaneType orthogonalSlicePlane, int sliceNr) {
    mSlicers.at(portID)->setOrthogonalSlicePlane(orthogonalSlicePlane, sliceNr);
}

void SliceRenderer::setArbitrarySlicePlane(uint portID, Plane slicePlane) {
    mSlicers.at(portID)->setArbitrarySlicePlane(slicePlane);
}

uint SliceRenderer::addInputConnection(DataPort::pointer port) {
    uint portID =  Renderer::addInputConnection(port);
    mSlicers[portID] = ImageSlicer::New();
    mSlicers[portID]->setOrthogonalSlicePlane(PLANE_X);
    return portID;
}


} // end namespace fast
