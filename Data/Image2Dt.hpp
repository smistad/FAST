#ifndef IMAGE2DT_HPP
#define IMAGE2DT_HPP

#include "PipelineObject.hpp"
using namespace fast;

class Image2Dt;
typedef boost::shared_ptr<Image2Dt> Image2DtPtr;

class Image2Dt : public PipelineObject {
    private:
        void execute();
};

#endif
