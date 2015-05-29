#ifndef ADD_TRANSFORMATION_HPP_
#define ADD_TRANSFORMATION_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

class AddTransformation : public ProcessObject {
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
