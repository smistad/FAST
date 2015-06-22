#include "FAST/Algorithms/AddTransformation/SetTransformation.hpp"
#include "FAST/SceneGraph.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


void SetTransformation::setTransformationInputConnection(ProcessObjectPort port) {
    setInputConnection(1, port);
}

SetTransformation::SetTransformation() {
    createInputPort<SpatialDataObject>(0);
    createInputPort<AffineTransformation>(1);
    createOutputPort<SpatialDataObject>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void SetTransformation::execute() {
    SpatialDataObject::pointer data = getStaticInputData<SpatialDataObject>(0);
    AffineTransformation::pointer transform = getStaticInputData<AffineTransformation>(1);
    AffineTransformation* T = transform.getPtr().get();
    SceneGraphNode::pointer dataNode = data->getSceneGraphNode();

    dataNode->setTransformation(*T);

    setStaticOutputData<SpatialDataObject>(0, data);
}

}
