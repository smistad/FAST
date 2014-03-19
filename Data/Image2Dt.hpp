#ifndef IMAGE2DT_HPP
#define IMAGE2DT_HPP

#include "PipelineObject.hpp"
#include "Image2D.hpp"
namespace fast {

class Image2Dt;
typedef boost::shared_ptr<Image2Dt> Image2DtPtr;

class Image2Dt : public PipelineObject {
    public:
        Image2DPtr getNextFrame();
    private:
        void execute();
};
}; // end namespace fast

#endif
