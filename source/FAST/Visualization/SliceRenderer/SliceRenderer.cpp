#include "SliceRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Algorithms/ImageSlicer/ImageSlicer.hpp"


namespace fast {


void SliceRenderer::execute() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if(m_disabled)
            return;
        if(mStop) {
            return;
        }
    }

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
        if(hasNewInputData(inputNr)) {
            SpatialDataObject::pointer input = getInputData<SpatialDataObject>(inputNr);

            {
                std::lock_guard<std::mutex> lock(mMutex);
                if(mHasRendered) {
                    mHasRendered = false;
                    // Get slicer and execute it on new input data
                    ImageSlicer::pointer slicer;
                    if(mSlicers.count(inputNr) == 0) {
                        slicer = ImageSlicer::New();
                        slicer->setOrthogonalSlicePlane(PLANE_X);
                    } else {
                        slicer = mSlicers.at(inputNr);
                    }
                    slicer->setInputData(input);
                    DataChannel::pointer port = slicer->getOutputPort();
                    slicer->update();

                    mDataToRender[inputNr] = port->getNextFrame<Image>();
                }
            }


        }
    }
}


SliceRenderer::SliceRenderer() {
    createInputPort<Image>(0, false);
    mIsModified = true;
}

SliceRenderer::SliceRenderer(PlaneType orthogonalSlicePlane, int sliceNr) {
    createInputPort(0, "Image");
    mSlicers[0] = ImageSlicer::New();
    setOrthogonalSlicePlane(0, orthogonalSlicePlane, sliceNr);
    setModified(true);
}

SliceRenderer::SliceRenderer(Plane slicePlane) {
    createInputPort(0, "Image");
    mSlicers[0] = ImageSlicer::New();
    setArbitrarySlicePlane(0, slicePlane);
    setModified(true);
}

uint SliceRenderer::addInputConnection(DataChannel::pointer port, PlaneType orthogonalSlicePlane, int sliceNr) {
    uint portID = Renderer::addInputConnection(port);
    mSlicers[portID] = ImageSlicer::New();
    mSlicers[portID]->setOrthogonalSlicePlane(orthogonalSlicePlane, sliceNr);
    return portID;
}

uint SliceRenderer::addInputConnection(DataChannel::pointer port, Plane slicePlane) {
    uint portID = Renderer::addInputConnection(port);
    mSlicers[portID] = ImageSlicer::New();
    mSlicers[portID]->setArbitrarySlicePlane(slicePlane);
    return portID;
}

uint SliceRenderer::addInputConnection(DataChannel::pointer port, std::shared_ptr<ImageSlicer> slicer) {
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

uint SliceRenderer::addInputConnection(DataChannel::pointer port) {
    uint portID =  Renderer::addInputConnection(port);
    mSlicers[portID] = ImageSlicer::New();
    mSlicers[portID]->setOrthogonalSlicePlane(PLANE_X);
    return portID;
}


} // end namespace fast
