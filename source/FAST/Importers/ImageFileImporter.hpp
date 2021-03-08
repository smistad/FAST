#ifndef IMAGE_FILE_IMPORTER_HPP_
#define IMAGE_FILE_IMPORTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  ImageFileImporter : public ProcessObject {
    FAST_OBJECT(ImageFileImporter)
    public:
        void setFilename(std::string filename);
        void loadAttributes() override;
    private:
        ImageFileImporter();
        void execute() override;

        std::string mFilename;
};

}

#endif
