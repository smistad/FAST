#include "PipelineObject.hpp"
using namespace fast;

void PipelineObject::update() {
    if(parentPipelineObject != NULL)
        parentPipelineObject->update();

    if(this->isModified) {
        this->execute();
    }
}
