#ifndef RIDGE_TRAVERSAL_CENTERLINE_EXTRACTION_HPP
#define RIDGE_TRAVERSAL_CENTERLINE_EXTRACTION_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  RidgeTraversalCenterlineExtraction : public ProcessObject {
    FAST_OBJECT(RidgeTraversalCenterlineExtraction)
    public:
    private:
        RidgeTraversalCenterlineExtraction();
        void execute();
};

}

#endif
