#include "AddTransformation.hpp"
#include "SceneGraph.hpp"
#include "Image.hpp"

namespace fast {


void AddTransformation::setTransformationInputConnection(ProcessObjectPort port) {
    setInputConnection(1, port);
}

AddTransformation::AddTransformation() {
    createInputPort<SpatialDataObject>(0);
    createInputPort<LinearTransformation>(1);
    createOutputPort<SpatialDataObject>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void AddTransformation::execute() {
    SpatialDataObject::pointer data = getStaticInputData<SpatialDataObject>(0);
    LinearTransformation::pointer transform = getStaticInputData<LinearTransformation>(1);
    LinearTransformation* T = transform.getPtr().get();
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

    setStaticOutputData(0, data);
}

}
