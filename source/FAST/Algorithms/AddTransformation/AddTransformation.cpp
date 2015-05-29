#include "FAST/Algorithms/AddTransformation/AddTransformation.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


void AddTransformation::setTransformationInputConnection(ProcessObjectPort port) {
    setInputConnection(1, port);
}

AddTransformation::AddTransformation() {
    createInputPort<SpatialDataObject>(0);
    createInputPort<AffineTransformation>(1);
    createOutputPort<SpatialDataObject>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void AddTransformation::execute() {
    SpatialDataObject::pointer data = getStaticInputData<SpatialDataObject>(0);
    AffineTransformation::pointer transform = getStaticInputData<AffineTransformation>(1);
    AffineTransformation* T = transform.getPtr().get();
    SceneGraphNode::pointer dataNode = data->getSceneGraphNode();

    if(data == mPrevious) {
        // This has already been processed, just change the transformation
        dataNode->getParent()->setTransformation(*T);
    } else {
        // Add new node
        SceneGraphNode::pointer newNode = SceneGraphNode::New();
        newNode->setParent(dataNode->getParent());
        newNode->setTransformation(*T);
        dataNode->setParent(newNode);
    }
    mPrevious = data;

    setStaticOutputData<SpatialDataObject>(0, data);
}

}
