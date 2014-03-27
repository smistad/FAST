#include "DataObject.hpp"
#include "ProcessObject.hpp"
using namespace fast;

void DataObject::update() {
    ProcessObject::pointer object = mParentObject;
    object->update();
}

void DataObject::setParent(Object::pointer parent) {
    if(parent == NULL)
        throw Exception("Trying to add an expired/NULL pointer as a parent object");

    mParentObject = parent;
}

unsigned long DataObject::getTimestamp() {
    return mTimestampModified;
}

void DataObject::updateModifiedTimestamp() {
    static unsigned long globalTimestamp = 0;
    mTimestampModified = ++globalTimestamp;
}
