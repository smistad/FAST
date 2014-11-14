#ifndef DATAOBJECT_HPP_
#define DATAOBJECT_HPP_

#include "SmartPointers.hpp"
#include "Object.hpp"
#include "ExecutionDevice.hpp"
#include <boost/unordered_map.hpp>
#include "BoundingBox.hpp"

namespace fast {

class DataObject : public Object {
    public:
        DataObject() : mTimestampModified(0),mIsDynamicData(false) {};
        typedef SharedPointer<DataObject> pointer;
        void update();
        void setSource(Object::pointer source);
        unsigned long getTimestamp();
        void updateModifiedTimestamp();
        void retain(ExecutionDevice::pointer device);
        void release(ExecutionDevice::pointer device);
        virtual BoundingBox getBoundingBox() const;
        virtual ~DataObject() { };
        bool isDynamicData();
    protected:
        virtual void free(ExecutionDevice::pointer device) = 0;
        virtual void freeAll() = 0;
        BoundingBox mBoundingBox;
        bool mIsDynamicData;
    private:
        boost::unordered_map<ExecutionDevice::pointer, unsigned int> mReferenceCount;
        // The souce object is the process object that created this data object
        // It is defined as a weak pointer to break a cyclic dependency on the process objects
        WeakPointer<Object> mSourceObject;
        unsigned long mTimestampModified;
};

}




#endif /* DATAOBJECT_HPP_ */
