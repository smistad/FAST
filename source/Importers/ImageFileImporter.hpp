#ifndef IMAGE_FILE_IMPORTER_HPP_
#define IMAGE_FILE_IMPORTER_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"

namespace fast {

class ImageFileImporter : public ProcessObject {
    FAST_OBJECT(ImageFileImporter)
    public:
        void setFilename(std::string filename);
        Image::pointer getOutput();
    private:
        ImageFileImporter();
        void execute();

        std::string mFilename;
};

}

#endif
