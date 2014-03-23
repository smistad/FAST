#ifndef IMAGEEXPORTER2D_HPP_
#define IMAGEEXPORTER2D_HPP_

#include "PipelineObject.hpp"
#include "Image2D.hpp"
#include "Image2Dt.hpp"
#include <string>

namespace fast {

class ImageExporter2D : public PipelineObject {
    FAST_OBJECT(ImageExporter2D)
    public:
        void setInput(Image2D::Ptr image);
        void setInput(Image2Dt::Ptr image);
        void setFilename(std::string filename);
    private:
        ImageExporter2D();
        void execute();

        std::string mFilename;
        Image2D::Ptr mStaticInput;
        Image2Dt::Ptr mDynamicInput;

};


} // end namespace fast




#endif /* IMAGEEXPORTER2D_HPP_ */
