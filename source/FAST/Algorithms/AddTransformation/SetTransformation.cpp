#include "FAST/Algorithms/AddTransformation/SetTransformation.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

SetTransformation::SetTransformation() {
    createInputPort<SpatialDataObject>(0);
    createInputPort<Transform>(1);
    createOutputPort<SpatialDataObject>(0);
}

void SetTransformation::execute() {
    auto data = getInputData<SpatialDataObject>(0);
    auto transform = getInputData<Transform>(1);
    auto dataNode = data->getSceneGraphNode();

    dataNode->setTransform(transform);

    addOutputData(0, data);
}

}
