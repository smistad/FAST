#ifndef INVERSE_GRADIENT_SEGMENTATION_HPP
#define INVERSE_GRADIENT_SEGMENTATION_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  InverseGradientSegmentation : public ProcessObject {
    FAST_OBJECT(InverseGradientSegmentation)
    public:
        void setCenterlineInputConnection(ProcessObjectPort port);
        void setVectorFieldInputConnection(ProcessObjectPort port);
    private:
        InverseGradientSegmentation();
        void execute();

};

}

#endif
