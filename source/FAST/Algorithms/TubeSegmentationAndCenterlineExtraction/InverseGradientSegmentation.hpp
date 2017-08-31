#ifndef INVERSE_GRADIENT_SEGMENTATION_HPP
#define INVERSE_GRADIENT_SEGMENTATION_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  InverseGradientSegmentation : public ProcessObject {
    FAST_OBJECT(InverseGradientSegmentation)
    public:
        void setCenterlineInputConnection(DataPort::pointer port);
        void setVectorFieldInputConnection(DataPort::pointer port);
    private:
        InverseGradientSegmentation();
        void execute();

};

}

#endif
