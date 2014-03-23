#include "PipelineObject.hpp"
#include "Exception.hpp"
using namespace fast;

void PipelineObject::update() {
    for(int i = 0; i < mParentPipelineObjects.size(); i++) {
        // Check that object has not been deleted
        // TODO maybe throw exception here?
        if(mParentPipelineObjects[i] != NULL)
            mParentPipelineObjects[i]->update();
    }

    if(this->mIsModified) {
        this->execute();
    }
}

void PipelineObject::addParent(PipelineObject::Ptr parent) {
    if(parent == NULL)
        throw Exception("Trying to add an expired/NULL pointer as a parent object");

    // Check that it doesn't already exist
    bool exist = false;
    for(int i = 0; i < mParentPipelineObjects.size(); i++) {
        if(parent == mParentPipelineObjects[i])
            exist = true;
    }
    if(!exist)
        mParentPipelineObjects.push_back(parent);
}
