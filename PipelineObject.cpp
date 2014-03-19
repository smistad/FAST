#include "PipelineObject.hpp"

void PipelineObject::update() {
    if(parentPipelineObject != NULL)
        parentPipelineObject->update();

    if(this->isModified) {
        this->execute();
    }
}
