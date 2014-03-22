#include "PipelineObject.hpp"
#include "Exception.hpp"
using namespace fast;

void PipelineObject::update() {
    for(int i = 0; i < mParentPipelineObjects.size(); i++) {
        // Check that object has not been deleted
        if(!mParentPipelineObjects[i].expired())
            mParentPipelineObjects[i].lock()->update();
    }

    if(this->mIsModified) {
        this->execute();
    }
}

void PipelineObject::addParent(boost::weak_ptr<PipelineObject> parent) {
    if(parent.expired())
        throw Exception("Trying to add an expired/NULL pointer as a parent object");

    // Check that it doesn't already exist
    bool exist = false;
    for(int i = 0; i < mParentPipelineObjects.size(); i++) {
        if(parent.lock() == mParentPipelineObjects[i].lock())
            exist = true;
    }
    if(!exist)
        mParentPipelineObjects.push_back(parent);
}
