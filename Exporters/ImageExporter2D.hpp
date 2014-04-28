#ifndef IMAGEEXPORTER2D_HPP_
#define IMAGEEXPORTER2D_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"
#include "Image2Dt.hpp"
#include <string>

namespace fast {

class ImageExporter2D : public ProcessObject {
    FAST_OBJECT(ImageExporter2D)
    public:
        void setInput(Image::pointer image);
        void setInput(Image2Dt::pointer image);
        void setFilename(std::string filename);
    private:
        ImageExporter2D();
        void execute();

        std::string mFilename;
        Image::pointer mStaticInput;
        Image2Dt::pointer mDynamicInput;

};


} // end namespace fast




#endif /* IMAGEEXPORTER2D_HPP_ */
