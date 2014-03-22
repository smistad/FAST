#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace fast {

#define FAST_OBJECT(className)                                  \
    public:                                                     \
        typedef boost::shared_ptr<className> Ptr;               \
        static className::Ptr New() {                           \
            className * ptr = new className();                  \
            className::Ptr smartPtr(ptr);                       \
            ptr->setPtr(smartPtr);                              \
                                                                \
            return smartPtr;                                    \
        }                                                       \
    private:                                                    \
        boost::weak_ptr<className> mPtr;                        \
        void setPtr(className::Ptr ptr) {                       \
            mPtr = ptr;                                         \
        }                                                       \

class PipelineObject {
    public:
        PipelineObject() : mIsModified(false) {};
        void update();
        typedef boost::shared_ptr<PipelineObject> Ptr;
        void addParent(boost::weak_ptr<PipelineObject> parent);
        virtual ~PipelineObject() {};
    protected:
        // Pointer to the parent pipeline object
        std::vector<boost::weak_ptr<PipelineObject> > mParentPipelineObjects;

        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

};

}; // end namespace fast

#endif
