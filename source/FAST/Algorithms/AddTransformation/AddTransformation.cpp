#include "FAST/Algorithms/AddTransformation/AddTransformation.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


AddTransformation::AddTransformation() {
    createInputPort<SpatialDataObject>(0);
    createInputPort<Transform>(1);
    createOutputPort<SpatialDataObject>(0);
}

void AddTransformation::execute() {
    SpatialDataObject::pointer data = getInputData<SpatialDataObject>(0);
    auto transform = getInputData<Transform>(1);
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
        currentChildNode->setTransform(transform);
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
        currentNode->setTransform(transform);
    }
    mPrevious = data;

    addOutputData(0, data);
}

}
