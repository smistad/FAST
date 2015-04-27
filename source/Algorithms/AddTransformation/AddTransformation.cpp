#include "AddTransformation.hpp"
#include "SceneGraph.hpp"
#include "Image.hpp"

namespace fast {


void AddTransformation::setTransformationInputConnection(ProcessObjectPort port) {
    setInputConnection(1, port);
}

AddTransformation::AddTransformation() {
    setOutputDataDynamicDependsOnInputData(0,0);
}

void AddTransformation::execute() {
    Image::pointer image = getStaticInputData<Image>(0);
    LinearTransformation::pointer transform = getStaticInputData<LinearTransformation>(1);
    LinearTransformation* T = transform.getPtr().get();

    SceneGraph::insertParentNodeToData(image, *T);

    DataObject::pointer output = getOutputData<Image>();
    if(output->isDynamicData()) {
        DynamicImage::pointer(output)->addFrame(image);
    } else {
        throw Exception("Not implemented yet.");
    }
}

}
