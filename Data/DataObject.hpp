#ifndef DATAOBJECT_HPP_
#define DATAOBJECT_HPP_

#include "SmartPointers.hpp"
#include "Object.hpp"

namespace fast {

class DataObject : public Object {
    public:
        DataObject() : mTimestampModified(0) {};
        typedef SharedPointer<DataObject> pointer;
        void update();
        void setParent(Object::pointer parent);
        unsigned long getTimestamp();
        void updateModifiedTimestamp();
    private:
        Object::pointer mParentObject;
        unsigned long mTimestampModified;
};

}




#endif /* DATAOBJECT_HPP_ */
