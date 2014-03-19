#ifndef PIPELINE_OBJECT_HPP
#define PIPELINE_OBJECT_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

namespace fast {

class PipelineObject;
typedef boost::shared_ptr<PipelineObject> PipelineObjectPtr;

class PipelineObject {
    public:
        PipelineObject() : mIsModified(false) {};
        void update();
    protected:
        // Pointer to the parent pipeline object
        std::vector<PipelineObjectPtr> mParentPipelineObjects;

        // Flag to indicate whether the object has been modified
        // and should be executed again
        bool mIsModified;

        // Pure virtual method for executing the pipeline object
        virtual void execute()=0;

        virtual ~PipelineObject();
};

}; // end namespace fast

#endif
