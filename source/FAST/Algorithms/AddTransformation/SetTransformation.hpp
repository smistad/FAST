#ifndef SET_TRANSFORMATION_HPP_
#define SET_TRANSFORMATION_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

class SetTransformation : public ProcessObject {
    FAST_OBJECT(SetTransformation)
    public:
        void setTransformationInputConnection(ProcessObjectPort port);
    private:
        SetTransformation();
        void execute();
};

}

#endif
