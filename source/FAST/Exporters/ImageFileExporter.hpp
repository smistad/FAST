#ifndef IMAGE_FILE_IMPORTER_HPP_
#define IMAGE_FILE_IMPORTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT ImageFileExporter : public ProcessObject {
    FAST_OBJECT(ImageFileExporter)
    public:
        void setFilename(std::string filename);
        ImageFileExporter();
    private:
        void execute();

        std::string mFilename;
};

}

#endif
