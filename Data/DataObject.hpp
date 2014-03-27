#ifndef DATAOBJECT_HPP_
#define DATAOBJECT_HPP_

#include "SmartPointers.hpp"
#include "Object.hpp"
#include "ExecutionDevice.hpp"
#include <boost/unordered_map.hpp>

namespace fast {

class DataObject : public Object {
    public:
        DataObject() : mTimestampModified(0) {};
        typedef SharedPointer<DataObject> pointer;
        void update();
        void setParent(Object::pointer parent);
        unsigned long getTimestamp();
        void updateModifiedTimestamp();
        void retain(ExecutionDevice::pointer device);
        void release(ExecutionDevice::pointer device);
        virtual ~DataObject() { };
    protected:
        virtual void free(ExecutionDevice::pointer device) = 0;
        virtual void freeAll() = 0;
    private:
        boost::unordered_map<ExecutionDevice::pointer, unsigned int> mReferenceCount;
        Object::pointer mParentObject;
        unsigned long mTimestampModified;
};

}




#endif /* DATAOBJECT_HPP_ */
