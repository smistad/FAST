#include "FAST/Algorithms/AddTransformation/SetTransformation.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

SetTransformation::SetTransformation() {
    createInputPort<SpatialDataObject>(0);
    createInputPort<AffineTransformation>(1);
    createOutputPort<SpatialDataObject>(0);
}

void SetTransformation::execute() {
    SpatialDataObject::pointer data = getInputData<SpatialDataObject>(0);
    AffineTransformation::pointer transform = getInputData<AffineTransformation>(1);
    SceneGraphNode::pointer dataNode = data->getSceneGraphNode();

    dataNode->setTransformation(transform);

    addOutputData(0, data);
}

}
