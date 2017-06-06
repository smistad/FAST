#ifndef ADD_TRANSFORMATION_HPP_
#define ADD_TRANSFORMATION_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

/**
 * This class will add new scene graph node before the root node of the input data object.
 * This node will get the transformation supplied to the transformation input connection
 */
class FAST_EXPORT  AddTransformation : public ProcessObject {
    FAST_OBJECT(AddTransformation)
    public:
        void setTransformationInputConnection(ProcessObjectPort port);
    private:
        AddTransformation();
        void execute();

        SpatialDataObject::pointer mPrevious;
};

}

#endif
