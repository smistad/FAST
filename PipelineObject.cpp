#include "PipelineObject.hpp"
using namespace fast;

void PipelineObject::update() {
    for(int i = 0; i < mParentPipelineObjects.size(); i++)
        mParentPipelineObjects[i]->update();

    if(this->mIsModified) {
        this->execute();
    }
}

void PipelineObject::addParent(PipelineObject::Ptr parent) {
    // Check that it doesn't already exist
    bool exist = false;
    for(int i = 0; i < mParentPipelineObjects.size(); i++) {
        if(parent == mParentPipelineObjects[i])
            exist = true;
    }
    if(!exist)
        mParentPipelineObjects.push_back(parent);
}
