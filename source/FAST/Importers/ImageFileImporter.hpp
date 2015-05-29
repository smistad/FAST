#ifndef IMAGE_FILE_IMPORTER_HPP_
#define IMAGE_FILE_IMPORTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class ImageFileImporter : public ProcessObject {
    FAST_OBJECT(ImageFileImporter)
    public:
        void setFilename(std::string filename);
    private:
        ImageFileImporter();
        void execute();

        std::string mFilename;
};

}

#endif
