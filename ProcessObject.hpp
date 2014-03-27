#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "SmartPointers.hpp"
#include <boost/unordered_map.hpp>
#include <vector>
#include "Object.hpp"
#include "DataObject.hpp"

namespace fast {

class ProcessObject : public Object {
    public:
        ProcessObject() : mIsModified(false) {};
        void update();
        typedef SharedPointer<ProcessObject> pointer;
        void addParent(DataObject::pointer parent);
        virtual ~ProcessObject() {};
    protected:
        // Pointer to the parent pipeline object
        std::vector<DataObject::pointer> mParentDataObjects;
        std::vector<unsigned long> mTimestamps;

        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

    private:
        void setTimestamp(DataObject::pointer object, unsigned long timestamp);

};

}; // end namespace fast

#endif
