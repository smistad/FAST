#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include "SmartPointers.hpp"
#include <vector>

namespace fast {

#define FAST_OBJECT(className)                                  \
    public:                                                     \
        typedef SharedPointer<className> pointer;               \
        static className::pointer New() {                       \
            className * ptr = new className();                  \
            className::pointer smartPtr(ptr);                   \
            ptr->setPtr(smartPtr);                              \
                                                                \
            return smartPtr;                                    \
        }                                                       \
    private:                                                    \
        WeakPointer<className> mPtr;                            \
        void setPtr(className::pointer ptr) {                       \
            mPtr = ptr;                                         \
        }                                                       \

class PipelineObject {
    public:
        PipelineObject() : mIsModified(false) {};
        void update();
        typedef SharedPointer<PipelineObject> pointer;
        void addParent(PipelineObject::pointer parent);
        virtual ~PipelineObject() {};
    protected:
        // Pointer to the parent pipeline object
        std::vector<PipelineObject::pointer> mParentPipelineObjects;

        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

};

}; // end namespace fast

#endif
