#include "PipelineObject.hpp"
using namespace fast;

void PipelineObject::update() {
    for(int i = 0; i < mParentPipelineObjects.size(); i++)
        mParentPipelineObjects[i]->update();

    if(this->mIsModified) {
        this->execute();
    }
}
