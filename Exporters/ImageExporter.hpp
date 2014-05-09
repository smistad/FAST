#ifndef ImageExporter_HPP_
#define ImageExporter_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include <string>

namespace fast {

class ImageExporter : public ProcessObject {
    FAST_OBJECT(ImageExporter)
    public:
        void setInput(Image::pointer image);
        void setInput(DynamicImage::pointer image);
        void setFilename(std::string filename);
    private:
        ImageExporter();
        void execute();

        std::string mFilename;
        Image::pointer mStaticInput;
        DynamicImage::pointer mDynamicInput;

};


} // end namespace fast




#endif /* ImageExporter_HPP_ */
