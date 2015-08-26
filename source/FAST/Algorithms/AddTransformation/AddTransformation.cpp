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

        // Find root node of dataNode
        SceneGraphNode::pointer currentNode = dataNode->getParent();
        SceneGraphNode::pointer currentChildNode = dataNode;
        while(!currentNode->isRootNode()) {
            currentChildNode = currentNode;
            currentNode = currentNode->getParent();
        }
        // CurrentNode is now root node
        // change transformation
        currentChildNode->setTransformation(*T);
    } else {
        // Find root node of dataNode
        SceneGraphNode::pointer currentNode = dataNode->getParent();
        while(!currentNode->isRootNode()) {
            currentNode = currentNode->getParent();
        }
        // CurrentNode is now root node
        // Add new root node
        SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
        currentNode->setParent(newRootNode);
        currentNode->setTransformation(*T);
    }
    mPrevious = data;

    setStaticOutputData<SpatialDataObject>(0, data);
}

}
